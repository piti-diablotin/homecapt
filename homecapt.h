#ifndef HOMECAPT_H
#define HOMECAPT_H

#include <QMainWindow>

namespace Ui {
class HomeCapt;
}

class HomeCapt : public QMainWindow
{
    Q_OBJECT

public:
    explicit HomeCapt(QWidget *parent = 0);
    ~HomeCapt();

private:
    Ui::HomeCapt *ui;
};

#endif // HOMECAPT_H
