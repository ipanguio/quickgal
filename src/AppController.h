#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include <QString>
#include <QStringList>

#include "MediaScanner.h"
#include "AlbumModel.h"
#include "MediaModel.h"

class AppController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool showHiddenAlbums
               READ showHiddenAlbums
               WRITE setShowHiddenAlbums
               NOTIFY showHiddenAlbumsChanged)

public:
    explicit AppController(
        AlbumModel *albumModel,
        MediaModel *mediaModel,
        QObject *parent = nullptr
    );

    bool showHiddenAlbums() const;

    void setShowHiddenAlbums(bool show);

    Q_INVOKABLE void refreshAlbums();

    Q_INVOKABLE void openAlbum(
        const QString &path
    );

    Q_INVOKABLE void sortMedia(
        const QString &field,
        bool ascending
    );

    Q_INVOKABLE void sortAlbums(
        const QString &field,
        bool ascending
    );

    Q_INVOKABLE bool deleteFile(const QString &path);

signals:
    void showHiddenAlbumsChanged();

private:
    MediaScanner m_scanner;
    AlbumModel *m_albumModel;
    MediaModel *m_mediaModel;

    bool m_showHiddenAlbums = false;

    QStringList mediaRoots() const;

    QString m_rootPath =
        "/home/phablet/Pictures";
};


#endif