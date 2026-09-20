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
#include <QMediaPlayer>
#include <QVideoProbe>
#include <QVideoFrame>
#include <QAbstractVideoBuffer>
#include <QEventLoop>
#include <QTimer>


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
    const QString &filePath,
    const QString &thumbnailPath
)
{
    return generateImageThumbnail(
        filePath,
        thumbnailPath
    );
}

bool ThumbnailWorker::generateImageThumbnail(
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

        reader.setScaledSize(decodeSize);
    }

    QImage image = reader.read();

    if (image.isNull()) {
        qWarning()
            << "No se pudo generar thumbnail de imagen:"
            << imagePath
            << reader.errorString();

        return false;
    }

    if (
        image.width() > 320 ||
        image.height() > 320
    ) {
        image = image.scaled(
            320,
            320,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );
    }

    return image.save(
        thumbnailPath,
        "JPG",
        80
    );
}

bool ThumbnailProvider::generateVideoThumbnail(
    const QString &videoPath,
    const QString &thumbnailPath
)
{
    QMediaPlayer player;
    QVideoProbe probe;

    QImage grabbedImage;
    bool frameReady = false;
    bool failed = false;

    QEventLoop loop;

    QTimer timeout;
    timeout.setSingleShot(true);

    connect(
        &timeout,
        &QTimer::timeout,
        &loop,
        &QEventLoop::quit
    );

    connect(
        &probe,
        &QVideoProbe::videoFrameProbed,
        [&](const QVideoFrame &frame)
        {
            qDebug()
                << "VIDEO FRAME"
                << videoPath
                << "valid:" << frame.isValid()
                << "size:" << frame.size()
                << "pixelFormat:" << frame.pixelFormat();
            
            QVideoFrame clone(frame);

            if (!clone.isValid()) {
                qWarning()
                    << "Frame de video no válido"
                    << videoPath;
                return;
            }

            if (!clone.map(
                    QAbstractVideoBuffer::ReadOnly)) {
                qWarning()
                    << "No se puede mapear frame:"
                    << videoPath
                    << "pixelFormat:"
                    << clone.pixelFormat();
                
                return;
            }

            QImage::Format imageFormat =
                QVideoFrame::imageFormatFromPixelFormat(
                    clone.pixelFormat()
                );
                qDebug()
                    << "QImage format:"
                    << imageFormat;

            if (imageFormat != QImage::Format_Invalid) {
                QImage image(
                    clone.bits(),
                    clone.width(),
                    clone.height(),
                    clone.bytesPerLine(),
                    imageFormat
                );

                grabbedImage = image.copy();
                frameReady = true;
            } else {
                qWarning()
                    <<"Formato de frame no convertible a QImage:"
                    << clone.pixelFormat()
                    << "video:"
                    << videoPath;
            }

            clone.unmap();

            if (frameReady) {
                player.stop();
                loop.quit();
            }
        }
    );

    connect(
        &player,
        static_cast<void (QMediaPlayer::*)(
            QMediaPlayer::MediaStatus)>(
            &QMediaPlayer::mediaStatusChanged
        ),
        [&](QMediaPlayer::MediaStatus status)
        {
            if (status == QMediaPlayer::LoadedMedia ||
                status == QMediaPlayer::BufferedMedia) {

                player.play();
                player.setPosition(1000);
            }

            if (status == QMediaPlayer::InvalidMedia) {
                failed = true;
                loop.quit();
            }
        }
    );

    connect(
        &player,
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        &QMediaPlayer::errorOccurred,
#else
        static_cast<void (QMediaPlayer::*)(QMediaPlayer::Error)>(
            &QMediaPlayer::error
        ),
#endif
        [&](QMediaPlayer::Error error)
        {
            Q_UNUSED(error);

            failed = true;
            loop.quit();
        }
    );

    if (!probe.setSource(&player)) {
        qWarning()
            << "No se pudo asociar QVideoProbe al reproductor";

        return false;
    }

    player.setMuted(true);
    player.setVolume(0);
    player.setMedia(
        QUrl::fromLocalFile(videoPath)
    );

    timeout.start(5000);
    loop.exec();
    timeout.stop();

    if (failed ||
        !frameReady ||
        grabbedImage.isNull()) {

        qWarning()
            << "No se pudo generar thumbnail de vídeo:"
            << videoPath;

        return false;
    }

    grabbedImage = grabbedImage.scaled(
        320,
        320,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );

    return grabbedImage.save(
        thumbnailPath,
        "JPG",
        80
    );
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

        if (isVideoFile(imagePath)) {

            qDebug()
                << "Generando thumbnail de video EN HILO PRINCIPAL:"
                << imagePath;

            const bool success = generateVideoThumbnail(imagePath, thumbnailPath);

            onThumbnailGenerated(imagePath, thumbnailPath, success);

        } else {
            m_worker.enqueue(
                imagePath,
                thumbnailPath
            );
        }
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

bool ThumbnailProvider::isVideoFile(
    const QString &filePath
) const
{
    const QString suffix =
        QFileInfo(filePath)
            .suffix()
            .toLower();

    return
        suffix == "mp4" ||
        suffix == "mkv" ||
        suffix == "avi" ||
        suffix == "mov" ||
        suffix == "webm" ||
        suffix == "3gp";
}