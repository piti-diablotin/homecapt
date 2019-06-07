#ifndef QPLOT_H
#define QPLOT_H

#include <QWidget>
#include "qcustomplot.h"

class QPlot : public QCustomPlot
{
    Q_OBJECT
  public:
    explicit QPlot(QWidget *parent = nullptr);

  signals:

  public slots:

  protected slots:
    void updateTracer(QMouseEvent *event);

  private:
    QCPItemTracer *_itemTracerDot;
    QCPItemTracer *_itemTracerCrosshair;
    QCPItemText *_itemText;
};

#endif // QPLOT_H
