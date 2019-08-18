#include "locationmodel.h"
#include "../homecapt/homecaptapi.h"

/**
 * Seee https://doc.qt.io/qt-5/qtquick-modelviewsdata-cppmodels.html
 */
LocationModel::LocationModel(QObject *parent)
  : QAbstractListModel(parent),
    _api(nullptr)
{
}

int LocationModel::rowCount(const QModelIndex &parent) const
{
  // For list models only the root node (an invalid parent) should return the list's size. For all
  // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
  if (parent.isValid() || !_api)
    return 0;

  // FIXME: Implement me!
  return _api->locations().size();
}

QVariant LocationModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid() || !_api)
    return QVariant();

  const HomeCaptAPI::Location loc = _api->locations().at(index.row());
  // FIXME: Implement me!
  switch(role)
  {
    case idRole:
      return QVariant(loc.id);
    case nameRole:
      return QVariant(loc.name);
    case ownerRole:
      return QVariant(loc.owner);
  }
  return QVariant();
}

bool LocationModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (!_api)
    return false;

  if (data(index, role) != value) {
    // FIXME: Implement me!
    emit dataChanged(index, index, QVector<int>() << role);
    return true;
  }
  return false;
}

Qt::ItemFlags LocationModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return Qt::NoItemFlags;

  return Qt::ItemIsEditable; // FIXME: Implement me!
}

QHash<int, QByteArray> LocationModel::roleNames() const
{
  QHash<int, QByteArray> hash;
  hash[idRole] = "id";
  hash[nameRole] = "name";
  hash[ownerRole] = "owner";
  return hash;
}

HomeCaptAPI* LocationModel::homecaptapi() const
{
  return _api;
}

void LocationModel::setHomecaptapi(HomeCaptAPI *api)
{
  beginResetModel();
  if (_api != nullptr) _api->disconnect(this);

  _api= api;

  if (_api)
  {
    connect(_api,&HomeCaptAPI::preLocationCreation, this, [=] {
      const int index = _api->locations().size();
      beginInsertRows(QModelIndex(),index,index);
    });
    connect(_api,&HomeCaptAPI::locationCreated, this, [=] {
      endInsertRows();
    });
    connect(_api,SIGNAL(hasLocations()),this, SLOT(forceUpdateModel()));
  }
  endResetModel();
}

void LocationModel::forceUpdateModel()
{
  beginResetModel();
  endResetModel();
}
