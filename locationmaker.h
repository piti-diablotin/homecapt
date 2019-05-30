#ifndef LOCATIONMAKER_H
#define LOCATIONMAKER_H

#include <QDialog>
#include "homecaptapi.h"

namespace Ui {
  class LocationMaker;
}

class LocationMaker : public QDialog
{
    Q_OBJECT

  public:
    explicit LocationMaker(HomeCaptAPI *api = 0, QWidget *parent = 0);
    ~LocationMaker();

  private slots:
    void on_buttonBox_accepted();
    void displayError(QString msg);

  private:
    Ui::LocationMaker *ui;
    HomeCaptAPI *_api;
};

#endif // LOCATIONMAKER_H
