#ifndef MEDIAMODEL_H
#define MEDIAMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <QVector>
#include <QDateTime>
#include <QString>

struct MediaItem
{
    QString path;
    QString fileName;
    bool isVideo = false;

    qint64 size = 0;

    QDateTime modified;
    QDateTime created;
};

class MediaModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(
        bool videoOnly
        READ videoOnly
        NOTIFY videoOnlyChanged
    )

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

    bool videoOnly() const;

    Q_PROPERTY(
        bool videoOnly
        READ videoOnly
        NOTIFY videoOnlyChanged
    )

    void setItems(const QVector<MediaItem> &items);

    void clear();

    void sortItems(
        const QString &field,
        bool ascending
    );

signals:
    void videoOnlyChanged();

private:
    QVector<MediaItem> m_items;
};

#endif