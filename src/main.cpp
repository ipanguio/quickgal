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

    AlbumModel albumModel;

    AppController appController(&albumModel);
    appController.refreshAlbums();
    
    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty(
        "albumModel",
        &albumModel
    );

    engine.rootContext()->setContextProperty(
        "appController",
        &appController
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