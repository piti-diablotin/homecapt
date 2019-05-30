#ifndef SENSORMAKER_H
#define SENSORMAKER_H

#include <QDialog>
#include "homecaptapi.h"

namespace Ui {
  class SensorMaker;
}

class SensorMaker : public QDialog
{
    Q_OBJECT

  public:
    explicit SensorMaker(HomeCaptAPI *api = 0, QWidget *parent = 0);
    ~SensorMaker();

  private slots:
    void on_buttonBox_accepted();
    void displayError(QString msg);

  private:
    Ui::SensorMaker *ui;
    HomeCaptAPI *_api;
};

#endif // SENSORMAKER_H
