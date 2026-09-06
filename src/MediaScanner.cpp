#include "MediaScanner.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QMap>

MediaScanner::MediaScanner(QObject *parent)
    : QObject(parent),
      m_showNoMedia(false)
{
    // Pic files extensions
    m_imageExtensions
        << "jpg"
        << "jpeg"
        << "png"
        << "gif"
        << "bmp"
        << "webp";

    // Video files extensions
    m_videoExtensions
        << "mp4"
        << "mkv"
        << "avi"
        << "mov"
        << "webm"
        << "3gp";
}


QStringList MediaScanner::scan(const QString &rootPath)
{
    QStringList result;

    QDir rootDir(rootPath);

    if (!rootDir.exists()) {
        return result;
    }

    QDirIterator iterator(
        rootPath,
        QDir::Files | QDir::NoDotAndDotDot,
        QDirIterator::Subdirectories
    );

    while (iterator.hasNext()) {

        const QString filePath = iterator.next();

        QFileInfo fileInfo(filePath);

        if (!m_showNoMedia) {

            QString directoryPath = fileInfo.absolutePath();

            bool hiddenByNoMedia = false;

            /*
             * Comprobamos este directorio y sus padres
             * hasta llegar al directorio raíz que estamos
             * escaneando.
             */
            while (directoryPath.startsWith(rootPath)) {

                if (hasNoMedia(directoryPath)) {
                    hiddenByNoMedia = true;
                    break;
                }

                QDir dir(directoryPath);

                if (!dir.cdUp()) {
                    break;
                }

                const QString parentPath = dir.absolutePath();

                if (parentPath == directoryPath) {
                    break;
                }

                directoryPath = parentPath;
            }

            if (hiddenByNoMedia) {
                continue;
            }
        }

        if (isMediaFile(filePath)) {
            result.append(filePath);
        }
    }

    return result;
}


void MediaScanner::setShowNoMedia(bool show)
{
    m_showNoMedia = show;
}


bool MediaScanner::showNoMedia() const
{
    return m_showNoMedia;
}


bool MediaScanner::isMediaFile(const QString &filePath) const
{
    QFileInfo fileInfo(filePath);

    const QString extension =
        fileInfo.suffix().toLower();

    return m_imageExtensions.contains(extension)
            || m_videoExtensions.contains(extension);
}


bool MediaScanner::hasNoMedia(const QString &directoryPath) const
{
    QDir directory(directoryPath);

    return QFileInfo::exists(
        directory.filePath(".nomedia")
    );
}

QVector<Album> MediaScanner::scanAlbums(const QString &rootPath)
{
    QVector<Album> albums;

    const QStringList files = scan(rootPath);

    QMap<QString, Album> albumMap;

    for (const QString &filePath : files) {
        QFileInfo fileInfo(filePath);
        const QString directoryPath = fileInfo.absolutePath();
        if (!albumMap.contains(directoryPath)) {
            Album album;
            album.path = directoryPath;
            QDir directory(directoryPath);
            album.name = directory.dirName();
            album.coverPath = filePath;
            album.count = 1;
            albumMap.insert(directoryPath, album);

        } else {

            Album &album = albumMap[directoryPath];
            album.count++;
        }
    }

    for (auto it = albumMap.constBegin();
         it != albumMap.constEnd();
         ++it) {

        albums.append(it.value());
    }

    return albums;
}

QVector<MediaItem> MediaScanner::scanDirectory(
    const QString &directoryPath
)
{
    QVector<MediaItem> items;

    QDir directory(directoryPath);

    if (!directory.exists()) {
        return items;
    }

    QFileInfoList files =
        directory.entryInfoList(
            QDir::Files | QDir::NoDotAndDotDot,
            QDir::Time
        );

    for (const QFileInfo &fileInfo : files) {

        if (!isMediaFile(fileInfo.absoluteFilePath())) {
            continue;
        }

        MediaItem item;

        item.path = fileInfo.absoluteFilePath();
        item.fileName = fileInfo.fileName();

        const QString extension =
            fileInfo.suffix().toLower();

        item.isVideo =
            m_videoExtensions.contains(extension);

        items.append(item);
    }

    return items;
}