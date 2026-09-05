#include <QDebug>
#include "AppController.h"

AppController::AppController(
    AlbumModel *albumModel,
    QObject *parent
)
    : QObject(parent),
      m_albumModel(albumModel)
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

    m_albumModel->setAlbums(albums);
}