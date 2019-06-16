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
    QList<Sensor> _sensors;
    QList<Location> _locations;
    QMap<int,SensorType> _sensorTypes;
    QVector<Data> _data;

    QJsonArray getResult(const QByteArray reply);


  public:
    explicit HomeCaptAPI(QObject *parent = nullptr);
    Q_INVOKABLE QString host();
    Q_INVOKABLE QString user();
    Q_INVOKABLE bool isReady();
    QNetworkReply::NetworkError getError();
    Q_INVOKABLE const QList<Location> &locations();
    Q_INVOKABLE const QList<Sensor> &sensors();
    Q_INVOKABLE const QMap<int,SensorType> &sensorTypes();
    Q_INVOKABLE void createLocation(const QString &name);
    Q_INVOKABLE void createSensor(const QString &name, const int location, const int type, const QString &comment);
    Q_INVOKABLE const QVector<Data> &data();

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
    void locationCreated();
    void sensorCreated();
    void hasData();


  public slots:
    void connect(const QString &url);
    void auth(const QString &user, const QString &password);
    void fetchSensorTypes();
    void fetchSensors();
    void fetchLocations();
    void fetchData(int sensor);

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
