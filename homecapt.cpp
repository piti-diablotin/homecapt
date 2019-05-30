#include "homecapt.h"
#include "ui_homecapt.h"
#include <QMetaObject>
#include <QMetaMethod>
#include <QDebug>
#include "locationmaker.h"
#include "sensormaker.h"

HomeCapt::HomeCapt(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::HomeCapt),
  _api(this),
  _locSensModel(new QStandardItemModel(this))
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

  connect(ui->LocSensView, SIGNAL(clicked(QModelIndex)),this,SLOT(plotIndex(QModelIndex)));
}

HomeCapt::~HomeCapt()
{
  delete ui;
}

void HomeCapt::manageSignal()
{
  QMetaMethod metaMethod = sender()->metaObject()->method(senderSignalIndex());
  auto signal = metaMethod.name();
  qDebug() << "signal" << signal;
  if ( signal == "errorConnect" )
  {
    qDebug() << "Error connect!";
  }
  else if ( signal == "isConnected" )
  {
    ui->user->setEnabled(true);
    ui->password->setEnabled(true);
    ui->host->setText(_api.host());
    ui->host->setReadOnly(true);
    ui->connect->setEnabled(true);
    ui->user->setFocus();
  }
  else if ( signal == "isAuthenticated")
  {
    qDebug() << "OK !";
    ui->user->setDisabled(true);
    ui->password->setDisabled(true);
  }
  else if ( signal == "errorAuth" )
  {
    qDebug() << "PB auth !";
    ui->connect->setChecked(false);
    ui->connect->setText(tr("Connect"));
  }
  else if ( signal == "errorJson" )
  {
    qDebug() << " Pb with json";
  }
  else if ( signal == "hasLocations" )
  {
    ui->addLocation->setEnabled(true);
  }
  else if ( signal == "hasSensors" )
  {
    qDebug() << "Ready";
    this->buildTree();
    ui->addSensor->setEnabled(true);
  }
}

void HomeCapt::manageSignal(const QString msg)
{
  QMetaMethod metaMethod = sender()->metaObject()->method(senderSignalIndex());
  auto signal = metaMethod.name();
  qDebug() << "signal" << signal;
  if ( signal == "errorJson" )
  {
    qDebug() << msg;
  }
  else if ( signal == "errorReply" )
  {
    qDebug() << msg;
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

void HomeCapt::plotIndex(QModelIndex index)
{
  auto indexes = ui->LocSensView->selectionModel()->selectedIndexes();
  if (_locSensModel->itemFromIndex(index)->parent() != nullptr)
  {
    int type = _locSensModel->itemFromIndex(*(indexes.begin()+3))->data(Qt::UserRole).toInt();
    int id = _locSensModel->itemFromIndex(*(indexes.begin()+1))->text().toInt();
    ui->temporary->setText(QString::number(id)+" "+QString::number(type) );
  }
}

void HomeCapt::on_host_editingFinished()
{
  if ( !ui->host->text().isEmpty() )
    _api.connect(ui->host->text());
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
  }
}

void HomeCapt::on_addLocation_clicked()
{
    LocationMaker *maker = new LocationMaker(&_api,this);
    if (maker->exec() == QDialog::Accepted)
    {
      this->buildTree();
    }
}

void HomeCapt::on_addSensor_clicked()
{
    SensorMaker *maker = new SensorMaker(&_api,this);
    if (maker->exec() == QDialog::Accepted)
    {
      this->buildTree();
    }
}
