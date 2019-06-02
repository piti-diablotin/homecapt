#include "homecaptapi.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>
#include <algorithm>
#include <QUrlQuery>

QJsonArray HomeCaptAPI::getResult(const QByteArray reply)
{
  QString data = reply;
  QJsonObject check = QJsonDocument::fromJson(data.toUtf8()).object();
  if ( check.isEmpty() || check["return_code"].toInt() != 0 ) {
    emit(errorJson(check["message"].toString()));
    return QJsonArray();
  }
  else
  {
    return check["result"].toArray();
  }
}

HomeCaptAPI::HomeCaptAPI(QObject *parent) : QObject(parent),
  _ready(false),
  _url(),
  _user(),
  _password(),
  _request(),
  _manager(this),
  _error(QNetworkReply::NoError),
  _sensors(),
  _locations(),
  _sensorTypes(),
  _data()
{

}

void HomeCaptAPI::connect(const QString &url)
{
  _url = url;
  if ( !_url.startsWith("http://",Qt::CaseInsensitive) && !_url.startsWith("https://",Qt::CaseInsensitive) )
    _url.insert(0,"http://");
  _request.setUrl(QUrl(_url));
  _request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::NoLessSafeRedirectPolicy);

  QObject::connect(&_manager,SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinishedConnect(QNetworkReply*)));

  _manager.get(_request);

}

void HomeCaptAPI::auth(const QString &user, const QString &password)
{
  _user = user;
  _password = password;
  QString concatenated = _user+ ":" + _password;
  QByteArray data = concatenated.toUtf8().toBase64();
  QString headerData = "Basic " + data;
  _request.setRawHeader("Authorization", headerData.toUtf8());
  _request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
  _request.setUrl(QUrl(_url));
  QObject::connect(&_manager,SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinishedAuth(QNetworkReply*)));
  _manager.get(_request);
}

QString HomeCaptAPI::host()
{
  return _url;
}

bool HomeCaptAPI::isReady()
{
  return _ready;
}

QNetworkReply::NetworkError HomeCaptAPI::getError()
{
  return _error;
}

const QList<HomeCaptAPI::Location> &HomeCaptAPI::locations()
{
  return _locations;
}

const QList<HomeCaptAPI::Sensor> &HomeCaptAPI::sensors()
{
  return _sensors;
}

const QMap<int,HomeCaptAPI::SensorType> &HomeCaptAPI::sensorTypes()
{
  return _sensorTypes;
}

void HomeCaptAPI::createLocation(const QString &name)
{
  _request.setUrl(QUrl(_url+"/create_location.php"));
  QUrlQuery query;
  query.addQueryItem("location_name",name);
  _manager.post(_request,query.query().toUtf8());
  QObject::connect(&_manager,SIGNAL(finished(QNetworkReply*)), this, SLOT(checkLocationCreated(QNetworkReply*)));
}

void HomeCaptAPI::createSensor(const QString &name, const int location, const int type, const QString &comment)
{
  _request.setUrl(QUrl(_url+"/create_sensor.php"));
  QUrlQuery query;
  query.addQueryItem("sensor_name",name);
  query.addQueryItem("sensor_location",QString::number(location));
  query.addQueryItem("sensor_type",QString::number(type));
  query.addQueryItem("sensor_comment",comment);
  _manager.post(_request,query.query().toUtf8());
  QObject::connect(&_manager,SIGNAL(finished(QNetworkReply*)), this, SLOT(checkSensorCreated(QNetworkReply*)));
}

const QVector<HomeCaptAPI::Data> &HomeCaptAPI::data()
{
  return _data;
}

void HomeCaptAPI::fetchSensorTypes()
{
  _request.setUrl(QUrl(_url+"/get_sensortypes.php"));
  _manager.get(_request);
  QObject::connect(&_manager,SIGNAL(finished(QNetworkReply*)), this, SLOT(buildSensorTypes(QNetworkReply*)));
}

void HomeCaptAPI::fetchSensors()
{
  _request.setUrl(QUrl(_url+"/get_sensors.php"));
  _manager.get(_request);
  QObject::connect(&_manager,SIGNAL(finished(QNetworkReply*)), this, SLOT(buildSensors(QNetworkReply*)));
}

void HomeCaptAPI::fetchLocations()
{
  _request.setUrl(QUrl(_url+"/get_locations.php"));
  _manager.get(_request);
  QObject::connect(&_manager,SIGNAL(finished(QNetworkReply*)), this, SLOT(buildLocations(QNetworkReply*)));
}

void HomeCaptAPI::fetchData(int sensor)
{
  QUrlQuery query;
  query.addQueryItem("sensor",QString::number(sensor));
  _request.setUrl(QUrl(_url+"/get_data.php?"+query.query()));
  QObject::connect(&_manager,SIGNAL(finished(QNetworkReply*)), this, SLOT(buildData(QNetworkReply*)));
  _manager.get(_request);
}

void HomeCaptAPI::replyFinishedConnect(QNetworkReply *rep)
{
  if (rep->error()==QNetworkReply::AuthenticationRequiredError)
  {
    QObject::disconnect(&_manager,SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinishedConnect(QNetworkReply*)));
    if ( rep->sslConfiguration().protocol() == QSsl::SecureProtocols )
    {
      _url.replace("http://","https://");
      _request.setUrl(QUrl(_url));
      emit(isConnected());
    }
    else
      emit(isConnectedNotSafe());
  }
  else
  {
    _url="";
    _error = rep->error();
    emit(errorConnect());
  }
}

void HomeCaptAPI::replyFinishedAuth(QNetworkReply *rep)
{
  if (rep->error()==QNetworkReply::NoError)
  {
    QObject::disconnect(&_manager,SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinishedAuth(QNetworkReply*)));
    getResult(rep->readAll());
    emit(isAuthenticated());
    _ready = true;
  }
  else
  {
    _error = rep->error();
    emit(errorAuth());
  }
}

void HomeCaptAPI::buildSensorTypes(QNetworkReply *rep)
{
  if (rep->error()==QNetworkReply::NoError)
  {
    QObject::disconnect(&_manager,SIGNAL(finished(QNetworkReply*)), this, SLOT(buildSensorTypes(QNetworkReply*)));
    QJsonArray result = getResult(rep->readAll());
    _sensorTypes.clear();
    for ( auto t = result.begin(); t != result.end(); ++t){
      QJsonObject type = t->toObject();
      _sensorTypes[type["id"].toString().toInt()] = SensorType({
                                                                 type["id"].toString().toInt(),
                                                                 type["quantity"].toString(),
                                                                 type["unit"].toString()
                                                               });
    }
    emit(hasSensorTypes());
  }
  else
  {
    emit(errorReply(rep->errorString()));
  }

}

void HomeCaptAPI::buildLocations(QNetworkReply *rep)
{
  if (rep->error()==QNetworkReply::NoError)
  {
    QObject::disconnect(&_manager,SIGNAL(finished(QNetworkReply*)), this, SLOT(buildLocations(QNetworkReply*)));
    QJsonArray result = getResult(rep->readAll());
    _locations.clear();
    for ( auto loc = result.begin(); loc != result.end(); ++loc ){
      QJsonObject location = loc->toObject();
      _locations << Location({ location["id"].toString().toInt(),
                               location["owner"].toString(),
                               location["name"].toString()
                             });
    }
    std::sort(_locations.begin(),_locations.end(),
              [](const Location &l1, const Location &l2){return l1.name.toLower()<l2.name.toLower();});
    emit(hasLocations());
  }
  else
  {
    emit(errorReply(rep->errorString()));
  }
}

void HomeCaptAPI::buildSensors(QNetworkReply *rep)
{
  if (rep->error()==QNetworkReply::NoError)
  {
    QObject::disconnect(&_manager,SIGNAL(finished(QNetworkReply*)), this, SLOT(buildSensors(QNetworkReply*)));
    QJsonArray result = getResult(rep->readAll());
    _sensors.clear();
    for ( auto sens = result.begin(); sens != result.end(); ++sens){
      QJsonObject sensor = sens->toObject();
      _sensors << Sensor({ sensor["id"].toString().toInt(),
                           sensor["type"].toString().toInt(),
                           sensor["location"].toString().toInt(),
                           sensor["owner"].toString(),
                           sensor["name"].toString(),
                           sensor["comment"].toString()
                         });
    }
    std::sort(_sensors.begin(),_sensors.end(),
              [](const Sensor s1,const Sensor s2){return s1.name.toLower()<s2.name.toLower();});
    emit(hasSensors());
  }
  else
  {
    emit(errorReply(rep->errorString()));
  }
}

void HomeCaptAPI::checkLocationCreated(QNetworkReply *rep)
{
  if (rep->error()==QNetworkReply::NoError)
  {
    QObject::disconnect(&_manager,SIGNAL(finished(QNetworkReply*)), this, SLOT(checkLocationCreated(QNetworkReply*)));
    QJsonArray result = getResult(rep->readAll());
    if (result.isEmpty())
    {
      emit(errorJson(tr("Result is empty!")));
      return;
    }
    else
    {
      QJsonObject location = result.begin()->toObject();
      _locations << Location({ location["id"].toString().toInt(),
                               location["owner"].toString(),
                               location["location"].toString()
                             });
      std::sort(_locations.begin(),_locations.end(),
                [](const Location &l1, const Location &l2){return l1.name.toLower()<l2.name.toLower();});

      emit(locationCreated());
    }
  }
  else
  {
    emit(errorReply(rep->errorString()));
  }
}

void HomeCaptAPI::checkSensorCreated(QNetworkReply *rep)
{
  if (rep->error()==QNetworkReply::NoError)
  {
    QObject::disconnect(&_manager,SIGNAL(finished(QNetworkReply*)), this, SLOT(checkSensorCreated(QNetworkReply*)));
    QJsonArray result = getResult(rep->readAll());
    if (result.isEmpty())
    {
      emit(errorJson(tr("Result is empty!")));
      return;
    }
    else
    {
      QJsonObject sensor = result.begin()->toObject();
      _sensors << Sensor({ sensor["id"].toString().toInt(),
                           sensor["type"].toString().toInt(),
                           sensor["location"].toString().toInt(),
                           sensor["owner"].toString(),
                           sensor["name"].toString(),
                           sensor["comment"].toString()
                         });
      std::sort(_sensors.begin(),_sensors.end(),
                [](const Sensor s1,const Sensor s2){return s1.name.toLower()<s2.name.toLower();});
      emit(sensorCreated());
    }
  }
  else
  {
    emit(errorReply(rep->errorString()));
  }
}

void HomeCaptAPI::buildData(QNetworkReply *rep)
{
  if (rep->error()==QNetworkReply::NoError)
  {
    QObject::disconnect(&_manager,SIGNAL(finished(QNetworkReply*)), this, SLOT(buildData(QNetworkReply*)));
    QJsonArray result = getResult(rep->readAll());
    _data.resize(result.size());
    _data.squeeze();
    int i = 0;
    for ( auto dat = result.begin(); dat != result.end(); ++dat ){
      QJsonObject data = dat->toObject();
      _data[i++] = Data({ data["date"].toString().toInt(),
                         data["value"].toString().toFloat()
                       });
    }
    emit(hasData());
  }
  else
  {
    _data.clear();
    emit(errorReply(rep->errorString()));
  }
}
