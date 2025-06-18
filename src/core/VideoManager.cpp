#include "VideoManager.h"
#include "../log/QgisDebug.h"
#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <QRandomGenerator>
#include <QPainter>
#include <QFont>
#include <QDebug>

VideoManager::VideoManager(QObject *parent)
    : QObject(parent)
    , mVideoSourceType(VideoSourceType::SIMULATION)
    , mIsStreaming(false)
    , mIsAIRunning(false)
    , mpVideoTimer(nullptr)
    , mpVideoDisplayWidget(nullptr)
    , mpMediaPlayer(nullptr)
    , mpVideoWidget(nullptr)
    , mpAIProcess(nullptr)
    , mpTcpServer(nullptr)
    , mpTcpSocket(nullptr)
    , mTcpPort(DEFAULT_TCP_PORT)
    , mFrameCount(0)
    , mDetectionCount(0)
{
   
    logMessage("VideoManager constructor started", Qgis::MessageLevel::Info);
{
   
    QString appDir = QApplication::applicationDirPath();
    mPythonScriptPath = QDir(appDir).filePath("../python/yolo_detection.py");
    

    mPythonExecutable = "python";
    

    mpVideoTimer = new QTimer(this);
    mpVideoTimer->setInterval(VIDEO_TIMER_INTERVAL);
    connect(mpVideoTimer, &QTimer::timeout, this, &VideoManager::onVideoTimer);
    

    mpMediaPlayer = new QMediaPlayer(this);
    mpVideoWidget = new QVideoWidget(); 
    mpMediaPlayer->setVideoOutput(mpVideoWidget);
    
    connect(mpMediaPlayer, &QMediaPlayer::mediaStatusChanged,
            this, &VideoManager::onMediaStatusChanged);
    connect(mpMediaPlayer, &QMediaPlayer::positionChanged,
            this, &VideoManager::onPositionChanged);
    

    connect(mpMediaPlayer, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
            this, [this](QMediaPlayer::Error error) {
        QString errorString;
        switch (error) {
            case QMediaPlayer::ResourceError:
                errorString = "Resource error - cannot see";
                break;
            case QMediaPlayer::FormatError:
                errorString = "Format error";
                break;
            case QMediaPlayer::NetworkError:
                errorString = "Network error";
                break;
            case QMediaPlayer::AccessDeniedError:
                errorString = "Access denied";
                break;
            case QMediaPlayer::ServiceMissingError:
                errorString = "Service missing";
                break;
            default:
                errorString = "Unknown error";
                break;
        }
        logMessage("MediaPlayer error: " + errorString, Qgis::MessageLevel::Critical);
        logMessage("MediaPlayer error string: " + mpMediaPlayer->errorString(), Qgis::MessageLevel::Critical);
    });
    

    
    logMessage("VideoManager created", Qgis::MessageLevel::Success);
}
}
VideoManager::~VideoManager() {
    stopVideoStream();
    stopAIDetection();
    stopTcpServer();
    
    if (mpVideoTimer) {
        mpVideoTimer->stop();
        delete mpVideoTimer;
        mpVideoTimer = nullptr;
    }
    
    if (mpMediaPlayer) {
        mpMediaPlayer->stop();
        delete mpMediaPlayer;
        mpMediaPlayer = nullptr;
    }
    

    if (mpVideoWidget) {
        delete mpVideoWidget;
        mpVideoWidget = nullptr;
    }
    
    logMessage("VideoManager destroyed", Qgis::MessageLevel::Success);
};

bool VideoManager::initialize() {
  
    startTcpServer();
    
  
    loadVideoFrames();
    
    logMessage("VideoManager initialized", Qgis::MessageLevel::Success);
    return true;
};

void VideoManager::setVideoSource(VideoSourceType type, const QString &source) {
   
    
    if (mIsStreaming) {
      
        logMessage("Cannot change video source while streaming", Qgis::MessageLevel::Warning);
        return;
    }
    
    mVideoSourceType = type;
    mVideoSource = source;
    
   
    
    switch (type) {
        case VideoSourceType::SIMULATION:
          
            setupSimulationVideo();
            break;
        case VideoSourceType::CAMERA:
           
            setupCameraVideo();
            break;
        case VideoSourceType::FILE:
           
            setupFileVideo();
            break;
    }
    
   
    logMessage(QString("Video source set to: %1").arg(static_cast<int>(type)), 
               Qgis::MessageLevel::Info);
};

void VideoManager::setVideoDisplayWidget(VideoDisplayWidget *widget) {
   
    mpVideoDisplayWidget = widget;
    
    if (mpVideoDisplayWidget) {        
        connect(this, &VideoManager::detectionResultsReady, 
                mpVideoDisplayWidget, &VideoDisplayWidget::updateDetectionResults);
        connect(this, &VideoManager::videoStatusChanged, 
                mpVideoDisplayWidget, &VideoDisplayWidget::setVideoStatus);
        connect(mpVideoDisplayWidget, &VideoDisplayWidget::videoControlClicked,
                this, [this](bool start) {
            if (start) {
                startVideoStream();
            } else {
                stopVideoStream();
            }
        });
        
       
        if (mpVideoWidget) {
           
            mpVideoDisplayWidget->setVideoWidget(mpVideoWidget);
        } else {
            
        }
        
        logMessage("Video display widget connected", Qgis::MessageLevel::Success);
    } else {
      
        logMessage("ERROR: VideoDisplayWidget is null!", Qgis::MessageLevel::Critical);
    }
};

void VideoManager::startVideoStream() {
    if (mIsStreaming) {
        logMessage("Video stream is already running", Qgis::MessageLevel::Warning);
        return;
    }
    
    mIsStreaming = true;
    mFrameCount = 0;
    mStartTime = QDateTime::currentDateTime();
    
    
    if (mVideoSourceType == VideoSourceType::FILE && mpMediaPlayer) {
        
      
        
        mpMediaPlayer->play();
        
    
        logMessage("Starting video file playback", Qgis::MessageLevel::Info);
    }
    
    mpVideoTimer->start();
    
   
    startAIDetection();
    
    emit videoStatusChanged(true);
    logMessage("Video stream started", Qgis::MessageLevel::Success);
};

void VideoManager::stopVideoStream() {
    if (!mIsStreaming) {
        return;
    }
    
    mIsStreaming = false;
    mpVideoTimer->stop();
    
   
    if (mVideoSourceType == VideoSourceType::FILE && mpMediaPlayer) {
        mpMediaPlayer->stop();
        logMessage("Stopping video file playback", Qgis::MessageLevel::Info);
    }
    
  
    stopAIDetection();
    
    emit videoStatusChanged(false);
    logMessage("Video stream stopped", Qgis::MessageLevel::Success);
}

void VideoManager::startAIDetection() {
    if (mIsAIRunning) {
        logMessage("AI detection is already running", Qgis::MessageLevel::Warning);
        return;
    }
    
    if (!QDir(mPythonScriptPath).exists()) {
        logMessage("Python script not found: " + mPythonScriptPath, 
                   Qgis::MessageLevel::Critical);
        return;
    }
    

    mpAIProcess = new QProcess(this);
    connect(mpAIProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &VideoManager::onAIProcessFinished);
    connect(mpAIProcess, &QProcess::errorOccurred,
            this, &VideoManager::onAIProcessError);
    
 
    QStringList arguments;
    arguments << mPythonScriptPath;
    arguments << "--port" << QString::number(mTcpPort);
    
 
    switch (mVideoSourceType) {
        case VideoSourceType::SIMULATION:
            arguments << "--source" << "simulation";
            break;
        case VideoSourceType::FILE:
            arguments << "--source" << "file";
            arguments << "--file_path" << mVideoSource;
            break;
        case VideoSourceType::CAMERA:
            arguments << "--source" << "camera";
            break;
    }
    

    mpAIProcess->start(mPythonExecutable, arguments);
    
    if (mpAIProcess->waitForStarted(3000)) {
        mIsAIRunning = true;
        emit aiStatusChanged(true);
        logMessage("AI detection started", Qgis::MessageLevel::Success);
    } else {
        delete mpAIProcess;
        mpAIProcess = nullptr;
        logMessage("Failed to start AI detection", Qgis::MessageLevel::Critical);
    }
}

void VideoManager::stopAIDetection() {
    if (!mIsAIRunning || !mpAIProcess) {
        return;
    }
    
    mIsAIRunning = false;
    

    mpAIProcess->terminate();
    if (!mpAIProcess->waitForFinished(3000)) {
        mpAIProcess->kill();
        mpAIProcess->waitForFinished(1000);
    }
    
    delete mpAIProcess;
    mpAIProcess = nullptr;
    
    emit aiStatusChanged(false);
    logMessage("AI detection stopped", Qgis::MessageLevel::Success);
}

void VideoManager::onVideoTimer() {

    if (!mIsStreaming) {
        return;
    }
    
    mFrameCount++;
    

    if (mIsAIRunning && mpTcpSocket && mpTcpSocket->state() == QTcpSocket::ConnectedState) {

    }
}



void VideoManager::loadVideoFrames() {
  
    logMessage("Video system initialized", Qgis::MessageLevel::Info);
}

void VideoManager::setupSimulationVideo() {
   
    mVideoSource = "simulation";
    logMessage("Simulation video setup completed", Qgis::MessageLevel::Info);
}

void VideoManager::setupCameraVideo() {
    
    mVideoSource = "camera";
    logMessage("Camera video setup completed", Qgis::MessageLevel::Info);
}

void VideoManager::setupFileVideo() {
    qDebug() << "=== setupFileVideo called ===";
    
   
    if (mVideoSource.isEmpty()) {
        qDebug() << "WARNING: Video source is empty, this should not happen!";
        return;
    }
    
    qDebug() << "Original path:" << mVideoSource;
    

    QFileInfo fileInfo(mVideoSource);
    mVideoSource = fileInfo.absoluteFilePath();
    mVideoSource = QDir::toNativeSeparators(mVideoSource);
    
    qDebug() << "Normalized path:" << mVideoSource;
    qDebug() << "Attempting to load video file:" << mVideoSource;
    logMessage("Attempting to load video file: " + mVideoSource, Qgis::MessageLevel::Info);
    
    if (!QFile::exists(mVideoSource)) {
        qDebug() << "ERROR: Video file not found!";
        logMessage("Video file not found: " + mVideoSource, Qgis::MessageLevel::Critical);
        logMessage("Application dir: " + QApplication::applicationDirPath(), Qgis::MessageLevel::Info);
        logMessage("Working dir: " + QDir::currentPath(), Qgis::MessageLevel::Info);
        return;
    }
    
    qDebug() << "Video file exists! Size:" << fileInfo.size() << "bytes, Suffix:" << fileInfo.suffix();
    logMessage(QString("Video file info - Size: %1 bytes, Suffix: %2")
               .arg(fileInfo.size()).arg(fileInfo.suffix()), Qgis::MessageLevel::Info);
    
    
    QUrl videoUrl = QUrl::fromLocalFile(mVideoSource);
    qDebug() << "Video URL:" << videoUrl.toString();
    logMessage("Video URL: " + videoUrl.toString(), Qgis::MessageLevel::Info);
    
    if (!mpMediaPlayer) {
        qDebug() << "ERROR: mpMediaPlayer is null!";
        logMessage("ERROR: MediaPlayer is null!", Qgis::MessageLevel::Critical);
        return;
    }
    
    qDebug() << "Setting media...";
    mpMediaPlayer->setMedia(videoUrl);
    
   
    qDebug() << "Media player state:" << mpMediaPlayer->state();
    qDebug() << "Media player media status:" << mpMediaPlayer->mediaStatus();
    qDebug() << "Video available:" << mpMediaPlayer->isVideoAvailable();
    qDebug() << "Audio available:" << mpMediaPlayer->isAudioAvailable();
    
      
    mpMediaPlayer->setVolume(0);
    
 
    QTimer::singleShot(3000, this, [this]() {
        
      
        if (mpMediaPlayer->error() != QMediaPlayer::NoError) {
            qDebug() << "Error string:" << mpMediaPlayer->errorString();
        }
    });

   
    logMessage("File video setup completed: " + mVideoSource, Qgis::MessageLevel::Info);
}

void VideoManager::startTcpServer() {
    if (mpTcpServer) {
        return;
    }
    
    mpTcpServer = new QTcpServer(this);
    connect(mpTcpServer, &QTcpServer::newConnection, 
            this, &VideoManager::onNewConnection);
    
    if (mpTcpServer->listen(QHostAddress::LocalHost, mTcpPort)) {
        logMessage(QString("TCP server started on port %1").arg(mTcpPort), 
                   Qgis::MessageLevel::Success);
    } else {
        logMessage("Failed to start TCP server", Qgis::MessageLevel::Critical);
    }
}

void VideoManager::stopTcpServer() {
    if (mpTcpSocket) {
        mpTcpSocket->disconnectFromHost();
        mpTcpSocket->deleteLater(); 
        mpTcpSocket = nullptr;
    }
    
    if (mpTcpServer) {
        mpTcpServer->close();
        delete mpTcpServer;
        mpTcpServer = nullptr;
    }
    
   
    mSocketBuffer.clear();
    
    logMessage("TCP server stopped", Qgis::MessageLevel::Info);
}

void VideoManager::onNewConnection() {
    if (mpTcpSocket) {

        QTcpSocket *newSocket = mpTcpServer->nextPendingConnection();
        newSocket->disconnectFromHost();
        return;
    }
    
    mpTcpSocket = mpTcpServer->nextPendingConnection();
    connect(mpTcpSocket, &QTcpSocket::readyRead,
            this, &VideoManager::onSocketReadyRead);
    connect(mpTcpSocket, &QTcpSocket::disconnected,
            this, &VideoManager::onSocketDisconnected);
    
    logMessage("Python AI client connected", Qgis::MessageLevel::Success);
}

void VideoManager::onSocketReadyRead() {
    if (!mpTcpSocket) {
        return;
    }
    
    mSocketBuffer.append(mpTcpSocket->readAll());
    

    while (true) {
        int endIndex = mSocketBuffer.indexOf('\n');
        if (endIndex == -1) {
            break;
        }
        
        QByteArray jsonData = mSocketBuffer.left(endIndex);
        mSocketBuffer.remove(0, endIndex + 1);
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(jsonData, &error);
        
        if (error.error == QJsonParseError::NoError) {
            processDetectionData(doc.object());
        } else {
            logMessage("Invalid JSON received from AI client", 
                       Qgis::MessageLevel::Warning);
        }
    }
}

void VideoManager::onSocketDisconnected() {
    mpTcpSocket = nullptr;
    logMessage("Python AI client disconnected", Qgis::MessageLevel::Info);
}

void VideoManager::processDetectionData(const QJsonObject &data) {
    if (!data.contains("detections")) {
        return;
    }
    
    QJsonArray detectionsArray = data["detections"].toArray();
    QList<DetectionResult> results;
    
    for (const auto &detection : detectionsArray) {
        QJsonObject detObj = detection.toObject();
        
        DetectionResult result;
        result.name = detObj["class"].toString();
        result.confidence = detObj["confidence"].toDouble();
        result.x = detObj["x"].toInt();
        result.y = detObj["y"].toInt();
        result.width = detObj["width"].toInt();
        result.height = detObj["height"].toInt();
        result.timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
        

        if (result.name == "person") {
            result.type = DetectionType::PERSON;
        } else if (result.name == "manhole") {
            result.type = DetectionType::MANHOLE_COVER;
        } else if (result.name == "bicycle" || result.name == "motorcycle") {
            result.type = DetectionType::ELECTRIC_BIKE;
        } else if (result.name == "drowning") {
            result.type = DetectionType::DROWNING_POINT;
        } else {
            result.type = DetectionType::UNKNOWN;
        }
        

        result.isRisk = detObj["risk"].toBool();
        result.riskLevel = detObj["risk_level"].toString();
        
        results.append(result);
    }
    

    if (data.contains("frame_image")) {
        QString frameBase64 = data["frame_image"].toString();
        if (!frameBase64.isEmpty()) {

            QByteArray imageData = QByteArray::fromBase64(frameBase64.toUtf8());
            QPixmap framePixmap;
            if (framePixmap.loadFromData(imageData, "JPEG")) {
                qDebug() << "=== 接收到检测框图像 ===" << framePixmap.size();

                if (mpVideoDisplayWidget) {
                    mpVideoDisplayWidget->updateVideoFrame(framePixmap);
                }
            } else {
                qDebug() << "image error";
            }
        }
    }
    
    if (!results.isEmpty()) {
        mDetectionCount += results.size();
        qDebug() << "=== VideoManager sending" << results.size() << "detection results ===";
        for (const auto &result : results) {
            qDebug() << "Sending detection:" << result.name 
                     << "type:" << static_cast<int>(result.type)
                     << "confidence:" << result.confidence
                     << "risk:" << result.isRisk;
        }
        emit detectionResultsReady(results);
        qDebug() << "=== detectionResultsReady signal emitted ===";
    }
}

void VideoManager::onAIProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    mIsAIRunning = false;
    
    if (exitStatus == QProcess::CrashExit) {
        logMessage("AI process crashed", Qgis::MessageLevel::Critical);
        emit errorOccurred("AI CRUSH");
    } else {
        logMessage("AI process finished normally", Qgis::MessageLevel::Info);
    }
    
    emit aiStatusChanged(false);
}

void VideoManager::onAIProcessError(QProcess::ProcessError error) {
    QString errorString;
    switch (error) {
        case QProcess::FailedToStart:
            errorString = "Failed to start AI process";
            break;
        case QProcess::Crashed:
            errorString = "AI process crashed";
            break;
        case QProcess::Timedout:
            errorString = "AI process timed out";
            break;
        default:
            errorString = "Unknown AI process error";
            break;
    }
    
    logMessage("AI process error: " + errorString, Qgis::MessageLevel::Critical);
    emit errorOccurred("AIcv error: " + errorString);
}

void VideoManager::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    QString statusString;
    switch (status) {
        case QMediaPlayer::UnknownMediaStatus:
            statusString = "Unknown";
            break;
        case QMediaPlayer::NoMedia:
            statusString = "No Media";
            break;
        case QMediaPlayer::LoadingMedia:
            statusString = "Loading";
            logMessage("adding file...", Qgis::MessageLevel::Info);
            break;
        case QMediaPlayer::LoadedMedia:
            statusString = "Loaded";
            logMessage("add succ", Qgis::MessageLevel::Success);

            if (mpMediaPlayer->isVideoAvailable()) {
                logMessage("usfule", Qgis::MessageLevel::Success);
            } else {
                logMessage("unusful", Qgis::MessageLevel::Warning);
            }
            if (mpMediaPlayer->isAudioAvailable()) {
                logMessage("ying ", Qgis::MessageLevel::Info);
            }
            logMessage(QString("time %1 ms").arg(mpMediaPlayer->duration()), Qgis::MessageLevel::Info);
            break;
        case QMediaPlayer::StalledMedia:
            statusString = "Stalled";
            logMessage("stop", Qgis::MessageLevel::Warning);
            break;
        case QMediaPlayer::BufferingMedia:
            statusString = "Buffering";
            logMessage("waiting...", Qgis::MessageLevel::Info);
            break;
        case QMediaPlayer::BufferedMedia:
            statusString = "Buffered";
            logMessage("playing", Qgis::MessageLevel::Success);
            break;
        case QMediaPlayer::EndOfMedia:
            statusString = "End of Media";
            logMessage("restart...", Qgis::MessageLevel::Info);

            if (mIsStreaming && mpMediaPlayer) {
                mpMediaPlayer->setPosition(0);
                mpMediaPlayer->play();
            }
            break;
        case QMediaPlayer::InvalidMedia:
            statusString = "Invalid Media";
            logMessage("unformat", Qgis::MessageLevel::Critical);
            break;
    }
    
    logMessage(QString("Media status changed to: %1").arg(statusString), Qgis::MessageLevel::Info);
}

void VideoManager::onPositionChanged(qint64 position) {

    if (mIsStreaming && mFrameCount % 30 == 0) { 
        qint64 duration = mpMediaPlayer->duration();
        if (duration > 0) {
            int progress = (position * 100) / duration;
            logMessage(QString("Video playback progress: %1%").arg(progress), 
                       Qgis::MessageLevel::Info);
        }
    }
}

