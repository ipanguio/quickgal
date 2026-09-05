#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QCoreApplication>
#include <QDebug>

#include "MediaScanner.h"
#include "AlbumModel.h"
#include "AppController.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    MediaScanner scanner;
    scanner.setShowNoMedia(false);

    QVector<Album> albums =
            scanner.scanAlbums("/home/phablet/Pictures");
    
    qWarning() << "Albumes encontrados:" << albums.size();

    AlbumModel albumModel;

    AppController appController(&albumModel);

    appController.refreshAlbums();
    
    // albumModel.setAlbums(albums);

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty(
        "albumModel",
        &albumModel
    );

    QString qmlPath =
            QCoreApplication::applicationDirPath()
            + "/qml/Main.qml";
    
    qWarning() << "Loading QML:" << qmlPath;

    engine.load(QUrl::fromLocalFile(qmlPath));

    if (engine.rootObjects().isEmpty()) {
        qWarning() << "ERROR: Main.qml no se pudo cargar";
        return -1;
    }

    qWarning() << "QML cargado correctamente";

    return app.exec();
}