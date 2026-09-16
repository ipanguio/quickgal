#include "ThumbnailProvider.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QStandardPaths>
#include <QUrl>
#include <QDebug>


/*
 * ThumbnailWorker
 */

ThumbnailWorker::ThumbnailWorker(
    QObject *parent
)
    : QThread(parent)
{
}


ThumbnailWorker::~ThumbnailWorker()
{
    stop();
    wait();
}


void ThumbnailWorker::enqueue(
    const QString &imagePath,
    const QString &thumbnailPath
)
{
    {
        QMutexLocker locker(&m_mutex);

        m_jobs.append(
            qMakePair(
                imagePath,
                thumbnailPath
            )
        );
    }

    m_waitCondition.wakeOne();
}


void ThumbnailWorker::stop()
{
    {
        QMutexLocker locker(&m_mutex);
        m_stopping = true;
    }

    m_waitCondition.wakeAll();
}


void ThumbnailWorker::run()
{
    while (true) {

        QPair<QString, QString> job;

        {
            QMutexLocker locker(&m_mutex);

            while (
                m_jobs.isEmpty()
                && !m_stopping
            ) {
                m_waitCondition.wait(
                    &m_mutex
                );
            }

            if (m_stopping) {
                break;
            }

            job = m_jobs.takeFirst();
        }

        const bool success =
            generateThumbnail(
                job.first,
                job.second
            );

        emit thumbnailGenerated(
            job.first,
            job.second,
            success
        );
    }
}


bool ThumbnailWorker::generateThumbnail(
    const QString &imagePath,
    const QString &thumbnailPath
)
{
    QImageReader reader(imagePath);

    reader.setAutoTransform(true);

    QSize sourceSize = reader.size();

    if (sourceSize.isValid()) {

        QSize decodeSize = sourceSize;

        decodeSize.scale(
            QSize(320, 320),
            Qt::KeepAspectRatio
        );

        reader.setScaledSize(
            decodeSize
        );
    }

    QImage image = reader.read();

    if (image.isNull()) {

        qWarning()
            << "No se pudo generar thumbnail:"
            << imagePath
            << reader.errorString();

        return false;
    }

    /*
     * Some formats may ignore setScaledSize().
     */
    if (
        image.width() > 320
        || image.height() > 320
    ) {
        image = image.scaled(
            320,
            320,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );
    }

    if (!image.save(
            thumbnailPath,
            "JPG",
            80)) {

        qWarning()
            << "No se pudo guardar thumbnail:"
            << thumbnailPath;

        return false;
    }

    return true;
}


/*
 * ThumbnailProvider
 */

ThumbnailProvider::ThumbnailProvider(
    QObject *parent
)
    : QObject(parent)
{
    connect(
        &m_worker,
        &ThumbnailWorker::thumbnailGenerated,
        this,
        &ThumbnailProvider::onThumbnailGenerated
    );

    m_worker.start();
}


ThumbnailProvider::~ThumbnailProvider()
{
    m_worker.stop();
    m_worker.wait();
}


bool ThumbnailProvider::busy() const
{
    return !m_pending.isEmpty();
}


int ThumbnailProvider::pendingCount() const
{
    return m_pending.size();
}


QString ThumbnailProvider::cacheDirectory() const
{
    QString path =
        QStandardPaths::writableLocation(
            QStandardPaths::CacheLocation
        );

    path += "/thumbnails";

    QDir directory;

    if (!directory.mkpath(path)) {

        qWarning()
            << "No se pudo crear cache:"
            << path;
    }

    return path;
}


QString ThumbnailProvider::cacheFileFor(
    const QString &imagePath
) const
{
    QFileInfo info(imagePath);

    QByteArray key =
        imagePath.toUtf8()
        + "|"
        + QByteArray::number(info.size())
        + "|"
        + QByteArray::number(
            info.lastModified()
                .toMSecsSinceEpoch()
        );

    const QByteArray hash =
        QCryptographicHash::hash(
            key,
            QCryptographicHash::Sha1
        ).toHex();

    return cacheDirectory()
        + "/"
        + QString::fromLatin1(hash)
        + ".jpg";
}


QString ThumbnailProvider::requestThumbnail(
    const QString &imagePath
)
{
    if (imagePath.isEmpty()) {
        return QString();
    }

    const QString thumbnailPath =
        cacheFileFor(imagePath);

    /*
     * Already cached: return immediately.
     */
    if (QFileInfo::exists(thumbnailPath)) {

        return QUrl::fromLocalFile(
            thumbnailPath
        ).toString();
    }

    /*
     * Do not queue the same image twice.
     */
    if (!m_pending.contains(imagePath)) {

        const bool wasBusy = busy();

        m_pending.insert(imagePath);

        emit pendingCountChanged();

        if (!wasBusy) {
            emit busyChanged();
        }

        m_worker.enqueue(
            imagePath,
            thumbnailPath
        );
    }

    /*
     * Empty source while thumbnail is generated.
     */
    return QString();
}


void ThumbnailProvider::onThumbnailGenerated(
    const QString &imagePath,
    const QString &thumbnailPath,
    bool success
)
{
    const bool wasBusy = busy();

    m_pending.remove(imagePath);

    emit pendingCountChanged();

    if (success) {

        const QString url =
            QUrl::fromLocalFile(
                thumbnailPath
            ).toString();

        emit thumbnailReady(
            imagePath,
            url
        );
    }

    if (wasBusy != busy()) {
        emit busyChanged();
    }
}


void ThumbnailProvider::clearCache()
{
    QDir directory(
        cacheDirectory()
    );

    const QFileInfoList files =
        directory.entryInfoList(
            QDir::Files
            | QDir::NoDotAndDotDot
        );

    for (const QFileInfo &fileInfo : files) {

        QFile::remove(
            fileInfo.absoluteFilePath()
        );
    }
}