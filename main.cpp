#include "homecapt.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HomeCapt w;
    w.show();

    return a.exec();
}
