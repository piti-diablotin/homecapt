#include "homecapt.h"
#include "ui_homecapt.h"
#include <QMetaObject>
#include <QMetaMethod>
#include <QDebug>
#include "locationmaker.h"
#include "sensormaker.h"
#include <QDateTime>

HomeCapt::HomeCapt(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::HomeCapt),
  _api(this),
  _locSensModel(new QStandardItemModel(this)),
  _currentType(0),
  _currentSensor(0),
  _currentDuration(-1),
  _currentDurationButton(nullptr)
{
  ui->setupUi(this);
  connect(&_api,SIGNAL(errorConnect()),this,SLOT(manageSignal()));
  connect(&_api,SIGNAL(errorAuth()),this,SLOT(manageSignal()));
  connect(&_api,SIGNAL(errorJson(const QString)),this,SLOT(manageSignal(const QString)));
  connect(&_api,SIGNAL(errorReply(const QString)),this,SLOT(manageSignal(const QString)));
  connect(&_api,SIGNAL(isConnected()),this,SLOT(manageSignal()));
  connect(&_api,SIGNAL(isConnectedNotSafe()),this,SLOT(manageSignal()));
  connect(&_api,SIGNAL(isAuthenticated()),this,SLOT(manageSignal()));
  connect(&_api,SIGNAL(hasSensorTypes()),this,SLOT(manageSignal()));
  connect(&_api,SIGNAL(hasLocations()),this,SLOT(manageSignal()));
  connect(&_api,SIGNAL(hasSensors()),this,SLOT(manageSignal()));

  connect(&_api,SIGNAL(isAuthenticated()),&_api,SLOT(fetchSensorTypes()));
  connect(&_api,SIGNAL(hasSensorTypes()),&_api,SLOT(fetchLocations()));
  connect(&_api,SIGNAL(hasLocations()),&_api,SLOT(fetchSensors()));

  connect(ui->LocSensView, SIGNAL(clicked(QModelIndex)),this,SLOT(fetchIndex(QModelIndex)));
  connect(&_api,SIGNAL(hasData()),this,SLOT(plot()));

  ui->plot->setInteractions(QCP::Interaction::iRangeDrag|QCP::Interaction::iRangeZoom);
  ui->plot->axisRect()->setRangeDrag(Qt::Horizontal);
  ui->plot->axisRect()->setRangeZoom(Qt::Horizontal);

  ui->day1->setChecked(true);
  _currentDurationButton = ui->day1;
  _currentDuration = 1;
}

HomeCapt::~HomeCapt()
{
  delete ui;
}

void HomeCapt::manageSignal()
{
  QMetaMethod metaMethod = sender()->metaObject()->method(senderSignalIndex());
  auto signal = metaMethod.name();
  if ( signal == "errorConnect" )
  {
    ui->statusBar->showMessage(tr("Cannot reach server"));
  }
  else if ( signal == "isConnected" )
  {
    ui->user->setEnabled(true);
    ui->password->setEnabled(true);
    ui->host->setText(_api.host());
    ui->host->setReadOnly(true);
    ui->connect->setEnabled(true);
    ui->user->setFocus();
    ui->statusBar->showMessage(tr("Server reached"));
  }
  else if ( signal == "isAuthenticated")
  {
    ui->user->setDisabled(true);
    ui->password->setDisabled(true);
    ui->statusBar->showMessage(tr("Authentication succeeded"));
  }
  else if ( signal == "errorAuth" )
  {
    ui->connect->setChecked(false);
    ui->connect->setText(tr("Connect"));
    ui->statusBar->showMessage(tr("Authentication failed"));
  }
  else if ( signal == "errorJson" )
  {
    ui->statusBar->showMessage(tr("Error in reading JSON"));
  }
  else if ( signal == "hasLocations" )
  {
    ui->addLocation->setEnabled(true);
  }
  else if ( signal == "hasSensors" )
  {
    this->buildTree();
    ui->addSensor->setEnabled(true);
  }
}

void HomeCapt::manageSignal(const QString msg)
{
  QMetaMethod metaMethod = sender()->metaObject()->method(senderSignalIndex());
  auto signal = metaMethod.name();
  if ( signal == "errorJson" )
  {
    ui->statusBar->showMessage(tr("Error in reading JSON: ")+msg);
  }
  else if ( signal == "errorReply" )
  {
    ui->statusBar->showMessage(tr("Error in the reply: ")+msg);
  }

}

void HomeCapt::buildTree()
{
  auto locations = _api.locations();
  auto sensors = _api.sensors();
  _locSensModel->clear();
  auto root = _locSensModel->invisibleRootItem();
  auto makeRow = [this](QString name, int id, QString owner, int type, QString comment)
  {
    QList<QStandardItem*> list;
    QStandardItem *itemType = new QStandardItem(type>0?_api.sensorTypes()[type].quantity:"");
    itemType->setData(type,Qt::UserRole);
    list << new QStandardItem(name) << new QStandardItem(QString::number(id))
         << new QStandardItem(owner) << itemType
         << new QStandardItem(comment);
    return list;
  };
  for (auto l = locations.begin(); l != locations.end(); ++l)
  {
    QList<QStandardItem*> loc = makeRow(l->name,l->id,l->owner,-1,"");
    for (auto s = sensors.begin(); s != sensors.end(); ++s)
    {
      if ( s->location == l->id )
        loc.first()->appendRow(makeRow(s->name,s->id,s->owner,s->type,s->comment));
    }
    root->appendRow(loc);
  }
  QStringList labels;
  labels << tr("Name") << tr("Id") << tr("Owner") << tr("Type") << tr("Comment");
  _locSensModel->setHorizontalHeaderLabels(labels);
  ui->LocSensView->setModel(_locSensModel);
  ui->LocSensView->expandAll();
  for (int c = 0 ; c < _locSensModel->columnCount() ; ++c)
    ui->LocSensView->resizeColumnToContents(c);
}

void HomeCapt::fetchIndex(QModelIndex index)
{
  auto indexes = ui->LocSensView->selectionModel()->selectedIndexes();
  if (_locSensModel->itemFromIndex(index)->parent() != nullptr)
  {
    _currentType = _locSensModel->itemFromIndex(*(indexes.begin()+3))->data(Qt::UserRole).toInt();
    _currentSensor = _locSensModel->itemFromIndex(*(indexes.begin()+1))->text().toInt();
    _api.fetchData(_currentSensor);
  }
}

void HomeCapt::plot()
{
  QVector<double> x;
  QVector<double> y;
  this->processData(x,y);
  ui->plot->clearPlottables();
  QSharedPointer<QCPAxisTickerDateTime> dateTimeTicker(new QCPAxisTickerDateTime);
  ui->plot->xAxis->setTicker(dateTimeTicker);
  ui->plot->yAxis->setLabel(_api.sensorTypes()[_currentType].unit);
  auto graph = ui->plot->addGraph();
  graph->addData(x,y,true);
  ui->plot->rescaleAxes(true);
  ui->plot->replot();
}

void HomeCapt::on_host_editingFinished()
{
  if ( !ui->host->text().isEmpty() )
  {
    _api.connect(ui->host->text());
    ui->statusBar->showMessage(tr("Trying to connect"));
  }
}

void HomeCapt::on_connect_clicked(bool checked)
{
  if (checked)
  {
    _api.auth(ui->user->text(),ui->password->text());
    ui->connect->setText(tr("Disconnect"));
  }
  else
  {
    ui->host->setReadOnly(false);
    ui->user->setEnabled(true);
    ui->password->setEnabled(true);
    ui->connect->setText(tr("Connect"));
    _locSensModel->clear();
    ui->statusBar->showMessage(tr("Trying to authenticate"));
  }
}

void HomeCapt::on_addLocation_clicked()
{
    LocationMaker *maker = new LocationMaker(&_api,this);
    if (maker->exec() == QDialog::Accepted)
    {
      ui->statusBar->showMessage(tr("Location added"));
      this->buildTree();
    }
}

void HomeCapt::on_addSensor_clicked()
{
    SensorMaker *maker = new SensorMaker(&_api,this);
    if (maker->exec() == QDialog::Accepted)
    {
      ui->statusBar->showMessage(tr("Sensor added"));
      this->buildTree();
    }
}

void HomeCapt::processData(QVector<double> &time, QVector<double> &value)
{
  const QVector<HomeCaptAPI::Data> &data = _api.data();
  int ndata = data.size();
  QVector<double> tempTime;
  tempTime.reserve(ndata);
  const int nmax=24*60;
  if (_currentDurationButton != ui->max)
  {
    QDateTime limit = QDateTime::currentDateTime().addDays(-_currentDuration);
    //QDateTime limit = QDateTime::fromString("2019-05-30 18:44:30","yyyy-MM-dd HH:mm:ss").addDays(-_currentDuration);
    for( auto dt = data.rbegin(); dt != data.rend(); ++dt)
    {
      QDateTime info = QDateTime::fromString(dt->date,"yyyy-MM-dd HH:mm:ss");
      tempTime.push_back(info.toSecsSinceEpoch());
      if ( info < limit )
        break;
    }
    tempTime.pop_back();
    tempTime.squeeze();
  }
  else
  {
    for( auto dt = data.rbegin(); dt != data.rend(); ++dt)
    {
      QDateTime info = QDateTime::fromString(dt->date,"yyyy-MM-dd HH:mm:ss");
      tempTime.push_back(info.toSecsSinceEpoch());
    }
  }

  int startingPoint = ndata-tempTime.size();
  int npoints = tempTime.size();

  if ( tempTime.size() > nmax )
  {
    int groupBy = npoints/nmax;
    int ngrouped = npoints/groupBy;
    int residu = npoints-ngrouped*groupBy;
    time.resize(ngrouped+residu);
    value.resize(ngrouped+residu);
    for (int g=0; g < ngrouped; ++g)
    {
      double accumTime = 0;
      double accumValue = 0;
      for (int subg=0; subg<groupBy; ++subg)
      {
        const int id = g*groupBy+subg;
        accumTime+=tempTime[npoints-1-id];
        accumValue+=data[startingPoint+id].value;
      }
      time[g] = accumTime/groupBy;
      value[g] = accumValue/groupBy;
    }
    for (int g=ngrouped*groupBy,i=ngrouped; g < npoints; ++g,++i)
    {
      time[i]= tempTime[npoints-1-g];
      value[i]=data[startingPoint+g].value;
    }
  }
  else
  {
    time.resize(npoints);
    value.resize(npoints);
    for (int i = startingPoint,j=0; i<ndata; ++i,++j)
    {
      time[j]= tempTime[npoints-1-j];
      value[j]=data[i].value;
    }
  }
}

void HomeCapt::on_day1_clicked(bool checked)
{
  if (checked)
  {
    _currentDurationButton->setChecked(false);
    _currentDuration = 1;
    _currentDurationButton = ui->day1;
    this->plot();
  }
  else
    ui->day1->setChecked(true);
}

void HomeCapt::on_week1_clicked(bool checked)
{
  if (checked)
  {
    _currentDurationButton->setChecked(false);
    _currentDuration = 7;
    _currentDurationButton = ui->week1;
    this->plot();
  }
  else
    ui->week1->setChecked(true);
}

void HomeCapt::on_month1_clicked(bool checked)
{
  if (checked)
  {
    _currentDurationButton->setChecked(false);
    _currentDuration = 31;
    _currentDurationButton = ui->month1;
    this->plot();
  }
  else
    ui->month1->setChecked(true);
}

void HomeCapt::on_month3_clicked(bool checked)
{
  if (checked)
  {
    _currentDurationButton->setChecked(false);
    _currentDuration = 3*31;
    _currentDurationButton = ui->month3;
    this->plot();
  }
  else
    ui->month3->setChecked(true);
}

void HomeCapt::on_month6_clicked(bool checked)
{
  if (checked)
  {
    _currentDurationButton->setChecked(false);
    _currentDuration = 6*31;
    _currentDurationButton = ui->month6;
    this->plot();
  }
  else
    ui->month6->setChecked(true);

}

void HomeCapt::on_year1_clicked(bool checked)
{
  if (checked)
  {
    _currentDurationButton->setChecked(false);
    _currentDuration = 365;
    _currentDurationButton = ui->year1;
    this->plot();
  }
  else
    ui->year1->setChecked(true);

}

void HomeCapt::on_year5_clicked(bool checked)
{
  if (checked)
  {
    _currentDurationButton->setChecked(false);
    _currentDuration = 5*365;
    _currentDurationButton = ui->year5;
    this->plot();
  }
  else
    ui->year5->setChecked(true);

}

void HomeCapt::on_max_clicked(bool checked)
{
  if (checked)
  {
    _currentDurationButton->setChecked(false);
    _currentDuration = -1;
    _currentDurationButton = ui->max;
    this->plot();
  }
  else
    ui->month1->setChecked(true);

}
