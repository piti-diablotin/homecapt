#ifndef HOMECAPT_H
#define HOMECAPT_H

#include <QMainWindow>
#include "homecaptapi.h"
#include <QStandardItemModel>

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

  private:
    Ui::HomeCapt *ui;
    HomeCaptAPI _api;
    QStandardItemModel *_locSensModel;
    int _currentType;
    int _currentSensor;
};

#endif // HOMECAPT_H
