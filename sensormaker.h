#ifndef SENSORMAKER_H
#define SENSORMAKER_H

#include <QDialog>

namespace Ui {
  class SensorMaker;
}

class SensorMaker : public QDialog
{
    Q_OBJECT

  public:
    explicit SensorMaker(QWidget *parent = 0);
    ~SensorMaker();

  private:
    Ui::SensorMaker *ui;
};

#endif // SENSORMAKER_H
