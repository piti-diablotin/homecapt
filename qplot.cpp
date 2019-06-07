#include "qplot.h"

QPlot::QPlot(QWidget *parent) : QCustomPlot(parent),
  _itemTracerDot(new QCPItemTracer(this)),
  _itemTracerCrosshair(new QCPItemTracer(this)),
  _itemText(new QCPItemText(this))
{
  //_itemTracerDot->setInterpolating(true);
  _itemTracerDot->setStyle(QCPItemTracer::tsCircle);
  _itemTracerDot->setPen(QPen(Qt::red));
  _itemTracerDot->setBrush(Qt::red);
  _itemTracerDot->setSize(7);
  //_itemTracerCrosshair->setInterpolating(true);
  _itemTracerCrosshair->setPen(QPen(Qt::red));
  _itemTracerCrosshair->setBrush(Qt::red);
  _itemText->position->setType(QCPItemPosition::ptPlotCoords);
  _itemText->setPositionAlignment(Qt::AlignLeft|Qt::AlignBottom);
  _itemText->setTextAlignment(Qt::AlignLeft);
  _itemText->setPadding(QMargins(10,0,0,0));
  connect(this,SIGNAL(mouseMove(QMouseEvent*)),this,SLOT(updateTracer(QMouseEvent*)));
}

void QPlot::updateTracer(QMouseEvent *event)
{
  double x = this->xAxis->pixelToCoord(event->pos().x());
  double y = this->yAxis->pixelToCoord(event->pos().y());
  _itemTracerDot->setGraph(this->graph());
  _itemTracerDot->setGraphKey(x);
  //_itemTracerDot->updatePosition();
  _itemTracerCrosshair->setGraph(this->graph());
  _itemTracerCrosshair->setGraphKey(x);
  //_itemTracerCrosshair->updatePosition();
  QDateTime dt = QDateTime::fromSecsSinceEpoch(x);
  _itemText->setText(dt.toString("yyyy-MM-dd HH:mm:ss")+"\n"+QString::number(_itemTracerDot->position->coords().y())+this->yAxis->label());
  _itemText->position->setCoords(x,y);
  this->replot();
}
