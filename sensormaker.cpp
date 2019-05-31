#include "sensormaker.h"
#include "ui_sensormaker.h"

SensorMaker::SensorMaker(HomeCaptAPI *api,QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SensorMaker),
  _api(api)
{
  ui->setupUi(this);
  if (_api == 0 || !_api->isReady()) this->reject();
  auto Locations = _api->locations();
  for ( auto loc = Locations.begin(); loc != Locations.end(); ++loc)
  {
    ui->location->addItem(loc->name,loc->id);
  }
  auto Types = _api->sensorTypes();
  for ( auto t = Types.begin(); t != Types.end(); ++t)
  {
    ui->type->addItem(t->quantity,t->id);
  }

  QObject::connect(_api,SIGNAL(errorJson(QString)),this,SLOT(displayError(QString)));
  QObject::connect(_api,SIGNAL(errorReply(QString)),this,SLOT(displayError(QString)));
  QObject::connect(_api,SIGNAL(sensorCreated()),this,SLOT(accept()));
}

SensorMaker::~SensorMaker()
{
  delete ui;
}

void SensorMaker::on_buttonBox_accepted()
{
  if (ui->name->text().isEmpty())
  {
    ui->info->setText(tr("Please provide a name"));
    return;
  }
  else if (ui->location->count() == 0)
  {
    ui->info->setText(tr("First create a location"));
    return;
  }
  if (_api != 0 && _api->isReady())
  {
    _api->createSensor(ui->name->text(),ui->location->currentData().toInt(),ui->type->currentData().toInt(),ui->comment->text());
  }
  else {
    ui->info->setText(tr("API not ready"));
  }
}

void SensorMaker::displayError(QString msg)
{
  ui->info->setText(msg);
}
