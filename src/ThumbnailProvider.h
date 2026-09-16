#ifndef THUMBNAILPROVIDER_H
#define THUMBNAILPROVIDER_H

#include <QObject>
#include <QString>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QList>
#include <QPair>
#include <QSet>


class ThumbnailWorker : public QThread
{
    Q_OBJECT

public:
    explicit ThumbnailWorker(QObject *parent = nullptr);
    ~ThumbnailWorker();

    void enqueue(
        const QString &imagePath,
        const QString &thumbnailPath
    );

    void stop();

signals:
    void thumbnailGenerated(
        const QString &imagePath,
        const QString &thumbnailPath,
        bool success
    );

protected:
    void run() override;

private:
    bool generateThumbnail(
        const QString &imagePath,
        const QString &thumbnailPath
    );

    QMutex m_mutex;
    QWaitCondition m_waitCondition;

    QList<QPair<QString, QString> > m_jobs;

    bool m_stopping = false;
};


class ThumbnailProvider : public QObject
{
    Q_OBJECT

    Q_PROPERTY(
        bool busy
        READ busy
        NOTIFY busyChanged
    )

    Q_PROPERTY(
        int pendingCount
        READ pendingCount
        NOTIFY pendingCountChanged
    )

public:
    explicit ThumbnailProvider(
        QObject *parent = nullptr
    );

    ~ThumbnailProvider();

    Q_INVOKABLE QString requestThumbnail(
        const QString &imagePath
    );

    Q_INVOKABLE void clearCache();

    bool busy() const;
    int pendingCount() const;

signals:
    void thumbnailReady(
        const QString &imagePath,
        const QString &thumbnailUrl
    );

    void busyChanged();
    void pendingCountChanged();

private slots:
    void onThumbnailGenerated(
        const QString &imagePath,
        const QString &thumbnailPath,
        bool success
    );

private:
    QString cacheDirectory() const;

    QString cacheFileFor(
        const QString &imagePath
    ) const;

    ThumbnailWorker m_worker;

    QSet<QString> m_pending;
};

#endif