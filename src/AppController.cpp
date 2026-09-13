#include <QDir>
#include <QFileInfo>
#include <QStringList>
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
    if (m_showHiddenAlbums == show) {
        return;
    }

    m_showHiddenAlbums = show;

    emit showHiddenAlbumsChanged();

    m_scanner.setShowNoMedia(show);

    refreshAlbums();

    qWarning()
    << "Mostrar albumes ocultos cambiado a:"
    << show;
}

void AppController::refreshAlbums()
{
    QVector<Album> allAlbums;

        const QStringList roots = mediaRoots();
        for (const QString &rootPath : roots) {
            qWarning()
                << "Escaneando raiz:"
                << rootPath;

            QVector<Album> albums = m_scanner.scanAlbums(rootPath);

            allAlbums += albums;
        }

        qWarning() <<"Albumes encontrados:"
                   << allAlbums.size();


    m_albumModel->setAlbums(allAlbums);
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
void AppController::sortMedia(
    const QString &field,
    bool ascending
)
{
    qWarning()
        << "Ordenando por:"
        << field
        << "ascendente:"
        << ascending;

    m_mediaModel->sortItems(
        field,
        ascending
    );
}
void AppController::sortAlbums(
    const QString &field,
    bool ascending
)
{
    qWarning()
        << "Ordenando albumes por:"
        << field
        << "ascendente:"
        << ascending;

    m_albumModel->sortAlbums(
        field,
        ascending
    );
}
QStringList AppController::mediaRoots() const
{
    QStringList roots;

    const QString internalPictures =
        "/home/phablet/Pictures";

    if (QDir(internalPictures).exists()) {
        roots << internalPictures;
    }

    QDir mediaDir("/media/phablet");

    const QFileInfoList devices =
        mediaDir.entryInfoList(
            QDir::Dirs | QDir::NoDotAndDotDot
        );

    for (const QFileInfo &deviceInfo : devices) {

        const QString picturesPath =
            deviceInfo.absoluteFilePath()
            + "/Pictures";

        if (QDir(picturesPath).exists()) {
            roots << picturesPath;
        }
    }

    return roots;
}