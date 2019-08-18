#include "sensormodel.h"

SensorModel::SensorModel(QObject *parent)
  : QAbstractListModel(parent),
    _location(-1),
    _api(nullptr),
    _sensors()
{
}

int SensorModel::rowCount(const QModelIndex &parent) const
{
  // For list models only the root node (an invalid parent) should return the list's size. For all
  // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
  if (parent.isValid())
    return 0;

  // FIXME: Implement me!
  return _sensors.size();
}

QVariant SensorModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid() || _sensors.isEmpty())
    return QVariant();

  HomeCaptAPI::Sensor sensor = _sensors[index.row()];
  // FIXME: Implement me!
  switch(role)
  {
    case idRole:
      return QVariant(sensor.id);
    case typeRole:
      return QVariant(sensor.type);
    case locationRole:
      return QVariant(sensor.location);
    case ownerRole:
      return QVariant(sensor.owner);
    case nameRole:
      return QVariant(sensor.name);
    case commentRole:
      return QVariant(sensor.comment);
  }
  return QVariant();
}

bool SensorModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (data(index, role) != value) {
    // FIXME: Implement me!
    emit dataChanged(index, index, QVector<int>() << role);
    return true;
  }
  return false;
}

Qt::ItemFlags SensorModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return Qt::NoItemFlags;

  return Qt::ItemIsEditable; // FIXME: Implement me!
}

QHash<int, QByteArray> SensorModel::roleNames() const
{
  QHash<int, QByteArray> hash;
  hash[idRole] = "id";
  hash[typeRole] = "type";
  hash[locationRole] = "location";
  hash[ownerRole] = "owner";
  hash[nameRole] = "name";
  hash[commentRole] = "comment";
  return hash;
}

HomeCaptAPI *SensorModel::homecaptapi() const
{
  return _api;
}

void SensorModel::setHomecaptapi(HomeCaptAPI *api)
{
  beginResetModel();
  if (_api != nullptr) _api->disconnect(this);

  _api= api;

  if (_api)
  {
    this->setLocation(_location);
    connect(_api,SIGNAL(hasSensors()),this, SLOT(forceUpdateModel()));
  }
  endResetModel();
}

int SensorModel::location() const
{
  return _location;
}

void SensorModel::setLocation(int location)
{
  beginResetModel();
  if (!_api)
  {
   return;
  }
  for(auto& loc : _api->locations())
  {
    if (loc.id==location) {
      _location = location;
      _sensors.clear();
      for(auto sensor : _api->sensors())
      {
        if (sensor.location == _location)
          _sensors.append(sensor);
      }
      endResetModel();
      return;
    }
  }
  endResetModel();
}

void SensorModel::forceUpdateModel()
{
  this->setLocation(_location);
}
