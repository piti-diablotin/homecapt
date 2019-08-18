#ifndef SENSORMODEL_H
#define SENSORMODEL_H

#include <QAbstractListModel>
#include "../homecapt/homecaptapi.h"

class HomeCaptAPI;

class SensorModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(HomeCaptAPI *api READ homecaptapi WRITE setHomecaptapi)
    Q_PROPERTY(int location READ location WRITE setLocation)

  public:
    explicit SensorModel(QObject *parent = nullptr);

    enum {
      idRole = Qt::UserRole,
      typeRole,
      locationRole,
      ownerRole,
      nameRole,
      commentRole
    };

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    virtual QHash<int, QByteArray> roleNames() const override;

    HomeCaptAPI *homecaptapi() const;
    void setHomecaptapi(HomeCaptAPI *api);

    int location() const;
    void setLocation(int location);

  private:
    int _location;
    HomeCaptAPI *_api;
    QVector<HomeCaptAPI::Sensor> _sensors;

  protected slots:
    void forceUpdateModel();
};

#endif // SENSORMODEL_H
