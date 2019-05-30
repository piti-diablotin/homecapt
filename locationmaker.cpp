#include "locationmaker.h"
#include "ui_locationmaker.h"

LocationMaker::LocationMaker(HomeCaptAPI *api, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::LocationMaker),
  _api(api)
{
  ui->setupUi(this);
  QObject::connect(_api,SIGNAL(errorJson(QString)),this,SLOT(displayError(QString)));
  QObject::connect(_api,SIGNAL(errorReply(QString)),this,SLOT(displayError(QString)));
  QObject::connect(_api,SIGNAL(locationCreated()),this,SLOT(accept()));
}

LocationMaker::~LocationMaker()
{
  delete ui;
}

void LocationMaker::on_buttonBox_accepted()
{
  if (ui->name->text().isEmpty())
  {
    ui->info->setText(tr("Please provide a name"));
    return;
  }
  if (_api != 0 && _api->isReady())
  {
    _api->createLocation(ui->name->text().toUtf8());
  }
  else {
    ui->info->setText(tr("API not ready"));
  }
}

void LocationMaker::displayError(QString msg)
{
  ui->info->setText(msg);
}
