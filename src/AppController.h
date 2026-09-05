#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>

#include "MediaScanner.h"
#include "AlbumModel.h"

class AppController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool showHiddenAlbums
               READ showHiddenAlbums
               WRITE setShowHiddenAlbums
               NOTIFY showHiddenAlbumsChanged)

public:
    explicit AppController(AlbumModel *albumModel,
                           QObject *parent = nullptr);

    bool showHiddenAlbums() const;

    void setShowHiddenAlbums(bool show);

    Q_INVOKABLE void refreshAlbums();

signals:
    void showHiddenAlbumsChanged();

private:
    MediaScanner m_scanner;
    AlbumModel *m_albumModel;

    bool m_showHiddenAlbums = false;

    QString m_rootPath =
        "/home/phablet/Pictures";
};

#endif