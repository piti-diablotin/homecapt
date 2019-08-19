#include "homecaptapi.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>
#include <algorithm>
#include <QUrlQuery>
#include <cmath>
#include <algorithm>
#include <QDebug>

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
  _data(),
  _dataFrom()
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

QString HomeCaptAPI::host() const
{
  return _url;
}

QString HomeCaptAPI::user() const
{
  return _user;
}

bool HomeCaptAPI::isReady() const
{
  return _ready;
}

QNetworkReply::NetworkError HomeCaptAPI::getError() const
{
  return _error;
}

const QVector<HomeCaptAPI::Location> &HomeCaptAPI::locations() const
{
  return _locations;
}

const QVector<HomeCaptAPI::Sensor> &HomeCaptAPI::sensors() const
{
  return _sensors;
}

const QMap<int,HomeCaptAPI::SensorType> &HomeCaptAPI::sensorTypesMap() const
{
  return _sensorTypes;
}

const QVector<HomeCaptAPI::SensorType> HomeCaptAPI::sensorTypes() const
{
  QVector<SensorType> types(_sensorTypes.size());
  int i = 0;
  for (auto it = _sensorTypes.begin(); it != _sensorTypes.end(); ++it)
  {
    types[i++] = *it;
  }
  return types;
}

bool HomeCaptAPI::createLocation(const QString &name)
{
  emit(preLocationCreation());
  _request.setUrl(QUrl(_url+"/create_location.php"));
  QUrlQuery query;
  query.addQueryItem("location_name",name);
  _manager.post(_request,query.query().toUtf8());
  QObject::connect(&_manager,SIGNAL(finished(QNetworkReply*)), this, SLOT(checkLocationCreated(QNetworkReply*)));
  return true;
}

bool HomeCaptAPI::createSensor(const QString &name, const int location, const int type, const QString &comment)
{
  emit(preSensorCreation());
  _request.setUrl(QUrl(_url+"/create_sensor.php"));
  QUrlQuery query;
  query.addQueryItem("sensor_name",name);
  query.addQueryItem("sensor_location",QString::number(location));
  query.addQueryItem("sensor_type",QString::number(type));
  query.addQueryItem("sensor_comment",comment);
  _manager.post(_request,query.query().toUtf8());
  QObject::connect(&_manager,SIGNAL(finished(QNetworkReply*)), this, SLOT(checkSensorCreated(QNetworkReply*)));
  return true;
}

const QVector<HomeCaptAPI::Data> &HomeCaptAPI::data()
{
  return _data;
}

void HomeCaptAPI::data(QLineSeries *serie) const
{
  serie->clear();
  serie->append(_dataFrom);
}

void HomeCaptAPI::preComputeDate(const int lastDays, const int nmax)
{
  _dataFrom.clear();
  _dataFrom.reserve(nmax);
  int ndata = _data.size();
  if (ndata==0)
  {
    return;
  }
  int npoints = 0;
  if (lastDays != -1)
  {
    qint64 limit = QDateTime::currentDateTime().addDays(-lastDays).toSecsSinceEpoch();
    for( auto dt = _data.rbegin(); dt != _data.rend(); ++dt)
    {
      ++npoints;
      if ( dt->date < limit )
        break;
    }
    --npoints;
  }
  else
  {
    npoints = ndata;
  }
  int startingPoint = ndata-npoints;

  if ( npoints > nmax )
  {
    int groupBy = static_cast<int>(std::ceil(static_cast<double>(npoints)/static_cast<double>(nmax)));
    int ngrouped = npoints/groupBy;

    for (int g=0; g < ngrouped; ++g)
    {
      double accumTime = 0;
      double accumValue = 0;
      for (int subg=0; subg<groupBy; ++subg)
      {
        const int id = g*groupBy+subg;
        accumTime+=_data[startingPoint+id].date*1000.;
        accumValue+=static_cast<double>(_data[startingPoint+id].value);
      }
      _dataFrom.push_back(QPointF(accumTime/groupBy,accumValue/groupBy));
    }
    for (int g=ngrouped*groupBy; g < npoints; ++g)
    {
      _dataFrom.push_back(QPointF(_data[startingPoint+g].date*1000.,static_cast<double>(_data[startingPoint+g].value)));
    }
  }
  else
  {
    for (int i = startingPoint,j=0; i<ndata; ++i,++j)
    {
      _dataFrom.push_back(QPointF(_data[i].date*1000.,static_cast<double>(_data[i].value)));
    }
  }
}

qint64 HomeCaptAPI::xmin()
{
  return _dataFrom.size() > 0 ? static_cast<qint64>(_dataFrom.begin()->rx()) : 0;
}

qint64 HomeCaptAPI::xmax()
{
  return _dataFrom.size() > 0 ? static_cast<qint64>(_dataFrom.rbegin()->rx()) : 0;
}

double HomeCaptAPI::ymin()
{
  if (_dataFrom.size()>0)
  {
    auto miny = std::min_element(_dataFrom.begin(),_dataFrom.end(),[](QPointF p1, QPointF p2){return p1.ry()<p2.ry();});
    return std::floor(miny->ry()/10)*10;
  }
  else
  {
    return 0;
  }
}

double HomeCaptAPI::ymax()
{
  if (_dataFrom.size()>0)
  {
    auto maxy = std::max_element(_dataFrom.begin(),_dataFrom.end(),[](QPointF p1, QPointF p2){return p1.ry()<p2.ry();});
    return std::ceil(maxy->ry()/10)*10;
  }
  else
  {
    return 100;
  }
}

QString HomeCaptAPI::location(const int id)
{
   for (auto& loc : _locations)
   {
     if (loc.id == id) return loc.name;
   }
   return tr("Not found");
}

QString HomeCaptAPI::sensor(const int id)
{
   for (auto& sensor : _sensors)
   {
     if (sensor.id == id) return sensor.name;
   }
   return tr("Not found");
}

QString HomeCaptAPI::sensorUnit(const int id)
{
   for (auto& sensor : _sensors)
   {
     if (sensor.id == id) return _sensorTypes[sensor.type].unit;
   }
   return tr("Not found");
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

void HomeCaptAPI::fetchData(int sensor, int from, int npoints)
{
  QUrlQuery query;
  query.addQueryItem("sensor",QString::number(sensor));
  query.addQueryItem("starting",QString::number(from));
  query.addQueryItem("npoints",QString::number(npoints));
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
      _sensorTypes[type["id"].toInt()] = SensorType({
                                                                 type["id"].toInt(),
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
      const QJsonObject location = loc->toObject();
      _locations.append({ location["id"].toInt(),
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
      _sensors.append({ sensor["id"].toInt(),
                        sensor["type"].toInt(),
                        sensor["location"].toInt(),
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
      _locations.append({ location["id"].toInt(),
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
      _sensors.append({ sensor["id"].toInt(),
                        sensor["type"].toInt(),
                        sensor["location"].toInt(),
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
      _data[i++] = Data({ data["date"].toInt(),
                          static_cast<float>(data["value"].toDouble())
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
