#include "homecaptapi.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>
#include <algorithm>

QJsonArray HomeCaptAPI::getResult(const QByteArray reply)
{
  QString data = reply;
  QJsonObject check = QJsonDocument::fromJson(data.toUtf8()).object();
  if ( check.isEmpty() || check["return_code"].toInt() != 0 ) {
    emit(errorJson(check["msg"].toString()));
    return QJsonArray();
  }
  else
  {
    return check["result"].toArray();
  }
}

HomeCaptAPI::HomeCaptAPI(QObject *parent) : QObject(parent),
  _url(),
  _user(),
  _password(),
  _request(),
  _manager(this),
  _error(QNetworkReply::NoError),
  _sensors(),
  _locations()
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
  _request.setUrl(QUrl(_url));
  QObject::connect(&_manager,SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinishedAuth(QNetworkReply*)));
  _manager.get(_request);
}

QString HomeCaptAPI::host()
{
  return _url;
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

void HomeCaptAPI::replyFinishedConnect(QNetworkReply *rep)
{
  //qDebug() << "Error" <<  rep->error();
  if (rep->error()==QNetworkReply::AuthenticationRequiredError)
  {
    //qDebug() << "Need auth" << QString(rep->readAll());
    QObject::disconnect(&_manager,SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinishedConnect(QNetworkReply*)));
    if ( rep->sslConfiguration().protocol() == QSsl::SecureProtocols )
    {
      //qDebug() << "SSL OK";
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
  //qDebug() << rep->isFinished();
  //qDebug() << QString(rep->readAll());
  if (rep->error()==QNetworkReply::NoError)
  {
    QObject::disconnect(&_manager,SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinishedAuth(QNetworkReply*)));
    //qDebug() << "efore";
    //qDebug() << QString(rep->readAll());
    getResult(rep->readAll());
    emit(isAuthenticated());
  }
  else
  {
    _error = rep->error();
    emit(errorAuth());
  }

}

void HomeCaptAPI::buildLocations(QNetworkReply *rep)
{
  if (rep->error()==QNetworkReply::NoError)
  {
    QObject::disconnect(&_manager,SIGNAL(finished(QNetworkReply*)), this, SLOT(buildLocations(QNetworkReply*)));
    QJsonArray result = getResult(rep->readAll());
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
