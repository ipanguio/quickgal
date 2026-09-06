#include <QDebug>
#include "AppController.h"

AppController::AppController(
    AlbumModel *albumModel,
    MediaModel *mediaModel,
    QObject *parent
)
    : QObject(parent),
      m_albumModel(albumModel),
      m_mediaModel(mediaModel)
{
    m_scanner.setShowNoMedia(m_showHiddenAlbums);
}

bool AppController::showHiddenAlbums() const
{
    return m_showHiddenAlbums;
}

void AppController::setShowHiddenAlbums(bool show)
{
    qWarning() << "setShowHiddenAlbums:" << show;

    if (m_showHiddenAlbums == show) {
        return;
    }

    m_showHiddenAlbums = show;

    m_scanner.setShowNoMedia(show);

    emit showHiddenAlbumsChanged();

    refreshAlbums();
}

void AppController::refreshAlbums()
{
    qWarning() << "Refrescando albumes";
    qWarning() << "Mostrar .nomedia:"
               << m_showHiddenAlbums;
    
               QVector<Album> albums =
        m_scanner.scanAlbums(m_rootPath);

        qWarning() <<"Albumes encontrados:"
                   << albums.size();

    m_albumModel->setAlbums(albums);
}
void AppController::openAlbum(
    const QString &path
)
{
    qWarning() << "Abriendo album:" << path;

    QVector<MediaItem> items =
        m_scanner.scanDirectory(path);

    qWarning() << "Elementos encontrados:"
               << items.size();

    m_mediaModel->setItems(items);
}