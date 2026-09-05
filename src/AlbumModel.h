#ifndef ALBUMMODEL_H
#define ALBUMMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <QVector>

struct Album
{
    QString name;
    QString path;
    QString coverPath;
    int count = 0;
};

class AlbumModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum AlbumRoles {
        NameRole = Qt::UserRole + 1,
        PathRole,
        CoverPathRole,
        CountRole
    };

    explicit AlbumModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(
        const QModelIndex &index,
        int role = Qt::DisplayRole
    ) const override;

    QHash<int, QByteArray> roleNames() const override;

    void clear();

    void addAlbum(const Album &album);

    void setAlbums(const QVector<Album> &albums);

    const QVector<Album> &albums() const;

private:
    QVector<Album> m_albums;
};

#endif // ALBUMMODEL_H