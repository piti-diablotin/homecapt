#include "homecapt.h"
#include "ui_homecapt.h"

HomeCapt::HomeCapt(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::HomeCapt)
{
    ui->setupUi(this);
}

HomeCapt::~HomeCapt()
{
    delete ui;
}
