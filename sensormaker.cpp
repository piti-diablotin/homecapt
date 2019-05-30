#include "sensormaker.h"
#include "ui_sensormaker.h"

SensorMaker::SensorMaker(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SensorMaker)
{
  ui->setupUi(this);
}

SensorMaker::~SensorMaker()
{
  delete ui;
}
