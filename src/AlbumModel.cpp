#include "AlbumModel.h"

AlbumModel::AlbumModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int AlbumModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_albums.size();
}

QVariant AlbumModel::data(
    const QModelIndex &index,
    int role
) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() < 0 || index.row() >= m_albums.size()) {
        return QVariant();
    }

    const Album &album = m_albums.at(index.row());

    switch (role) {

    case NameRole:
        return album.name;

    case PathRole:
        return album.path;

    case CoverPathRole:
        return album.coverPath;

    case CountRole:
        return album.count;

    default:
        return QVariant();
    }
}

QHash<int, QByteArray> AlbumModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[NameRole] = "name";
    roles[PathRole] = "path";
    roles[CoverPathRole] = "coverPath";
    roles[CountRole] = "count";

    return roles;
}

void AlbumModel::clear()
{
    beginResetModel();

    m_albums.clear();

    endResetModel();
}

void AlbumModel::addAlbum(const Album &album)
{
    const int newRow = m_albums.size();

    beginInsertRows(
        QModelIndex(),
        newRow,
        newRow
    );

    m_albums.append(album);

    endInsertRows();
}

void AlbumModel::setAlbums(const QVector<Album> &albums)
{
    beginResetModel();

    m_albums = albums;

    endResetModel();
}

void AlbumModel::sortAlbums(
    const QString &field,
    bool ascending
)
{
    beginResetModel();

    std::sort(
        m_albums.begin(),
        m_albums.end(),
        [&field, ascending](
            const Album &a,
            const Album &b
        )
        {
            int result = 0;

            if (field == "name") {
                result =
                    QString::localeAwareCompare(
                        a.name,
                        b.name
                    );
            }
            else if (field == "path") {
                result =
                    QString::localeAwareCompare(
                        a.path,
                        b.path
                    );
            }
            else if (field == "size") {
                if (a.size < b.size)
                    result = -1;
                else if (a.size > b.size)
                    result = 1;
            }
            else if (field == "modified") {
                if (a.modified < b.modified)
                    result = -1;
                else if (a.modified > b.modified)
                    result = 1;
            }
            else if (field == "created") {
                if (a.created < b.created)
                    result = -1;
                else if (a.created > b.created)
                    result = 1;
            }

            return ascending
                ? result < 0
                : result > 0;
        }
    );

    endResetModel();
}

const QVector<Album> &AlbumModel::albums() const
{
    return m_albums;
}