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

    QJsonArray getResult(const QByteArray reply);


  public:
    explicit HomeCaptAPI(QObject *parent = nullptr);
    void connect(const QString &url);
    void auth(const QString &user, const QString &password);
    QString host();
    bool isReady();
    QNetworkReply::NetworkError getError();
    const QList<Location> &locations();
    const QList<Sensor> &sensors();
    const QMap<int,SensorType> &sensorTypes();
    void createLocation(const QString &name);
    void createSensor(const QString &name, const int location, const int type, const QString &comment);

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


  public slots:
    void fetchSensorTypes();
    void fetchSensors();
    void fetchLocations();

  protected slots:
    void replyFinishedConnect(QNetworkReply *rep);
    void replyFinishedAuth(QNetworkReply *rep);
    void buildSensorTypes(QNetworkReply *rep);
    void buildLocations(QNetworkReply *rep);
    void buildSensors(QNetworkReply *rep);
    void checkLocationCreated(QNetworkReply *rep);
    void checkSensorCreated(QNetworkReply *rep);
};

#endif // HOMECAPTAPI_H
