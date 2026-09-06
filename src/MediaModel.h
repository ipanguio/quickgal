#ifndef MEDIAMODEL_H
#define MEDIAMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <QVector>

struct MediaItem
{
    QString path;
    QString fileName;
    bool isVideo = false;
};

class MediaModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum MediaRoles {
        PathRole = Qt::UserRole + 1,
        FileNameRole,
        IsVideoRole
    };

    explicit MediaModel(QObject *parent = nullptr);

    int rowCount(
        const QModelIndex &parent = QModelIndex()
    ) const override;

    QVariant data(
        const QModelIndex &index,
        int role = Qt::DisplayRole
    ) const override;

    QHash<int, QByteArray> roleNames() const override;

    void setItems(const QVector<MediaItem> &items);

    void clear();

private:
    QVector<MediaItem> m_items;
};

#endif