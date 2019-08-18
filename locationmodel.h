#ifndef LOCATIONLIST_H
#define LOCATIONLIST_H

#include <QAbstractListModel>
#include "../homecapt/homecaptapi.h"

class HomeCaptAPI;

class LocationModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(HomeCaptAPI *api READ homecaptapi WRITE setHomecaptapi)

  public:
    explicit LocationModel(QObject *parent = nullptr);

    enum {
      idRole = Qt::UserRole,
      nameRole,
      ownerRole
    };

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    virtual QHash<int, QByteArray> roleNames() const override;

    HomeCaptAPI* homecaptapi() const;
    void setHomecaptapi(HomeCaptAPI *api);

  private:
    HomeCaptAPI *_api;

  protected slots:
    void forceUpdateModel();
};

#endif // LOCATIONLIST_H
