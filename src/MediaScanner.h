#ifndef MEDIASCANNER_H
#define MEDIASCANNER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include "AlbumModel.h"
#include "MediaModel.h"

class MediaScanner : public QObject
{
    Q_OBJECT

public:
    explicit MediaScanner(QObject *parent = nullptr);

    // Scan a given directoy and return media files.
    QStringList scan(const QString &rootPath);

    //Returns found albums
    QVector<Album> scanAlbums(const QString &rootPath);

    //Returns found pictures and videos found in a given directory
    QVector<MediaItem> scanDirectory(const QString &directoryPath);

    // Decide whether this directory is shown or not based on .nomedia.
    void setShowNoMedia(bool show);

    bool showNoMedia() const;

private:
    bool m_showNoMedia = false;

    QStringList m_imageExtensions;
    QStringList m_videoExtensions;
    
    bool isMediaFile(const QString &filePath) const;
    bool hasNoMedia(const QString &directoryPath) const;

    bool isInsideNoMediaTree(
        const QString &directoryPath,
        const QString &rootPath
    ) const;

};

#endif // MEDIASCANNER_H