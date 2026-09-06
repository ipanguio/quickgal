#include "MediaModel.h"

MediaModel::MediaModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int MediaModel::rowCount(
    const QModelIndex &parent
) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_items.size();
}

QVariant MediaModel::data(
    const QModelIndex &index,
    int role
) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() < 0 ||
        index.row() >= m_items.size()) {
        return QVariant();
    }

    const MediaItem &item =
        m_items.at(index.row());

    switch (role) {

    case PathRole:
        return item.path;

    case FileNameRole:
        return item.fileName;

    case IsVideoRole:
        return item.isVideo;

    default:
        return QVariant();
    }
}

QHash<int, QByteArray> MediaModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[PathRole] = "path";
    roles[FileNameRole] = "fileName";
    roles[IsVideoRole] = "isVideo";

    return roles;
}

void MediaModel::setItems(
    const QVector<MediaItem> &items
)
{
    beginResetModel();

    m_items = items;

    endResetModel();
}

void MediaModel::clear()
{
    beginResetModel();

    m_items.clear();

    endResetModel();
}