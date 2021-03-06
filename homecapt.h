#ifndef HOMECAPT_H
#define HOMECAPT_H

#include <QMainWindow>
#include "homecaptapi.h"
#include <QStandardItemModel>
#include <QPushButton>
#include <QVector>
#include "qcustomplot.h"

namespace Ui {
  class HomeCapt;
}

class HomeCapt : public QMainWindow
{
    Q_OBJECT

  public:
    explicit HomeCapt(QWidget *parent = 0);
    ~HomeCapt();

  private slots:
    void manageSignal();
    void manageSignal(const QString msg);
    void buildTree();
    void fetchIndex(QModelIndex index);
    void plot();
    void on_host_editingFinished();

    void on_connect_clicked(bool checked);

    void on_addLocation_clicked();

    void on_addSensor_clicked();

    void on_day1_clicked();

    void on_week1_clicked();

    void on_month1_clicked();

    void on_month3_clicked();

    void on_month6_clicked();

    void on_year1_clicked();

    void on_year5_clicked();

    void on_max_clicked();

    void on_npoints_valueChanged(int arg1);

  private:
    Ui::HomeCapt *ui;
    HomeCaptAPI _api;
    QStandardItemModel *_locSensModel;
    int _currentType;
    int _currentSensor;
    int _currentDuration;
    QPushButton *_currentDurationButton;

    void processData(QVector<double> &time, QVector<double> &value);
    void adjustData(QPushButton* button);
};

#endif // HOMECAPT_H
