#ifndef HOMECAPTAPI_H
#define HOMECAPTAPI_H

#include <QObject>

class HomeCaptAPI : public QObject
{
    Q_OBJECT
public:
    explicit HomeCaptAPI(QObject *parent = nullptr);

signals:

public slots:
};

#endif // HOMECAPTAPI_H