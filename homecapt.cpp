#include "homecapt.h"
#include "ui_homecapt.h"
#include <QMetaObject>
#include <QMetaMethod>
#include <QDebug>

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
  connect(&_api,SIGNAL(hasLocations()),this,SLOT(manageSignal()));
  connect(&_api,SIGNAL(hasSensors()),this,SLOT(manageSignal()));
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
    _api.fetchLocations();
  }
  else if ( signal == "errorAuth" )
  {
    qDebug() << "PB auth !";
  }
  else if ( signal == "errorJson" )
  {
    qDebug() << " Pb with json";
  }
  else if ( signal == "hasLocations" )
  {
    _api.fetchSensors();
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
  auto root = _locSensModel->invisibleRootItem();
  auto makeRow = [](QString name, int id, QString owner, int type, QString comment)
  {
    QList<QStandardItem*> list;
    list << new QStandardItem(name) << new QStandardItem(QString::number(id))
         << new QStandardItem(owner) << new QStandardItem(type>0?QString::number(type):"")
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
    _locSensModel->clear();
  }
}
