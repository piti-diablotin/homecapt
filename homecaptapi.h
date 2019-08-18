#ifndef HOMECAPTAPI_H
#define HOMECAPTAPI_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QString>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QVector>
#include <QtCharts/QLineSeries>

QT_CHARTS_USE_NAMESPACE

class HomeCaptAPI : public QObject
{
    Q_OBJECT
  public:

    typedef struct {
        int id;
        QString quantity;
        QString unit;
    } SensorType;

    typedef struct {
        int id;
        int type;
        int location;
        QString owner;
        QString name;
        QString comment;
    } Sensor;

    typedef struct {
        int id;
        QString owner;
        QString name;
    } Location;

    typedef struct {
        int date;
        float value;
    } Data;

  private:

    bool _ready;
    QString _url;
    QString _user;
    QString _password;
    QNetworkRequest _request;
    QNetworkAccessManager _manager;
    QNetworkReply::NetworkError _error;
    QVector<Sensor> _sensors;
    QVector<Location> _locations;
    QMap<int,SensorType> _sensorTypes;
    QVector<Data> _data;
    QList<QPointF> _dataFrom;
    QJsonArray getResult(const QByteArray reply);


  public:
    explicit HomeCaptAPI(QObject *parent = nullptr);
    Q_INVOKABLE QString host() const;
    Q_INVOKABLE QString user() const;
    Q_INVOKABLE bool isReady() const;
    QNetworkReply::NetworkError getError() const;
    Q_INVOKABLE const QVector<Location> &locations() const;
    Q_INVOKABLE const QVector<Sensor> &sensors() const;
    Q_INVOKABLE const QMap<int,SensorType> &sensorTypesMap() const;
    Q_INVOKABLE const QVector<SensorType> sensorTypes() const;
    Q_INVOKABLE bool createLocation(const QString &name);
    Q_INVOKABLE bool createSensor(const QString &name, const int location, const int type, const QString &comment);
    Q_INVOKABLE const QVector<Data> &data();
    Q_INVOKABLE void data(QLineSeries* serie) const;
    Q_INVOKABLE void preComputeDate(const int lastDays, const int nmax);
    Q_INVOKABLE qint64 xmin();
    Q_INVOKABLE qint64 xmax();
    Q_INVOKABLE double ymin();
    Q_INVOKABLE double ymax();
    Q_INVOKABLE QString location(const int id);
    Q_INVOKABLE QString sensor(const int id);
    Q_INVOKABLE QString sensorUnit(const int id);


  signals:
    void errorConnect();
    void errorAuth();
    void errorJson(const QString);
    void errorReply(const QString);
    void isConnected();
    void isConnectedNotSafe();
    void isAuthenticated();
    void hasSensorTypes();
    void hasLocations();
    void hasSensors();
    void preLocationCreation();
    void locationCreated();
    void sensorCreated();
    void preSensorCreation();
    void hasData();


  public slots:
    void connect(const QString &url);
    void auth(const QString &user, const QString &password);
    void fetchSensorTypes();
    void fetchSensors();
    void fetchLocations();
    void fetchData(int sensor, int from=0, int npoints=-1);

  protected slots:
    void replyFinishedConnect(QNetworkReply *rep);
    void replyFinishedAuth(QNetworkReply *rep);
    void buildSensorTypes(QNetworkReply *rep);
    void buildLocations(QNetworkReply *rep);
    void buildSensors(QNetworkReply *rep);
    void checkLocationCreated(QNetworkReply *rep);
    void checkSensorCreated(QNetworkReply *rep);
    void buildData(QNetworkReply *rep);
};

#endif // HOMECAPTAPI_H
