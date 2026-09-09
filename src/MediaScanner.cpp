#include "MediaScanner.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QMap>
#include <algorithm>

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
             * We check this directoy and its parents
             * until we reach the root directory we are scanning
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

    QDir rootDir(rootPath);

    if (!rootDir.exists()) {
        return albums;
    }

    QStringList directories;
    directories.append(rootPath);

    QDirIterator dirIterator(
        rootPath,
        QDir::Dirs | QDir::NoDotAndDotDot,
        QDirIterator::Subdirectories
    );

    while (dirIterator.hasNext()) {
        directories.append(dirIterator.next());
    }

    for (const QString &directoryPath : directories) {

        /*
         * If the directory is hidden by a .nomedia file
         * we skip it and its subdirectories
         */

        if (!m_showNoMedia && hasNoMedia(directoryPath)) {
            continue;
        }

        QDir directory(directoryPath);

        QFileInfoList files =
            directory.entryInfoList(
                QDir::Files | QDir::NoDotAndDotDot,
                QDir::Time
            );
        
        Album album;
        
        album.name = directory.dirName();
        album.path = directoryPath;
        album.count = 0;

        for (const QFileInfo &fileInfo : files) {
            const QString filePath = fileInfo.absoluteFilePath();

            if (!isMediaFile(filePath)) {
                continue;
            }

            if (album.count == 0) {
                album.coverPath = filePath;
            }

            ++album.count;
        }

        if (album.count > 0) {
            albums.append(album);
        }          
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

        item.size = fileInfo.size();

        item.modified =
            fileInfo.lastModified();

        item.created =
            fileInfo.birthTime();

        if (!item.created.isValid()) {
            item.created = item.modified;
        }

        items.append(item);
    }

    return items;
}