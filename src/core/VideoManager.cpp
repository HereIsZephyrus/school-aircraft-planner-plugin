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
    qDebug() << "=== VideoManager constructor started ===";
    logMessage("VideoManager constructor started", Qgis::MessageLevel::Info);
{
    // 设置Python脚本路径
    QString appDir = QApplication::applicationDirPath();
    mPythonScriptPath = QDir(appDir).filePath("../python/yolo_detection.py");
    
    // 检测Python可执行文件
    mPythonExecutable = "python";
    
    // 创建视频定时器
    mpVideoTimer = new QTimer(this);
    mpVideoTimer->setInterval(VIDEO_TIMER_INTERVAL);
    connect(mpVideoTimer, &QTimer::timeout, this, &VideoManager::onVideoTimer);
    
    // 创建媒体播放器和视频控件
    mpMediaPlayer = new QMediaPlayer(this);
    mpVideoWidget = new QVideoWidget();
    mpMediaPlayer->setVideoOutput(mpVideoWidget);
    
    connect(mpMediaPlayer, &QMediaPlayer::mediaStatusChanged,
            this, &VideoManager::onMediaStatusChanged);
    connect(mpMediaPlayer, &QMediaPlayer::positionChanged,
            this, &VideoManager::onPositionChanged);
    
    // 添加错误处理
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
    }
    
    if (mpMediaPlayer) {
        mpMediaPlayer->stop();
        delete mpMediaPlayer;
    }
    
    logMessage("VideoManager destroyed", Qgis::MessageLevel::Success);
};

bool VideoManager::initialize() {
    // 初始化TCP服务器
    startTcpServer();
    
    // 加载模拟视频帧
    loadVideoFrames();
    
    logMessage("VideoManager initialized", Qgis::MessageLevel::Success);
    return true;
};

void VideoManager::setVideoSource(VideoSourceType type, const QString &source) {
    qDebug() << "=== setVideoSource called ===";
    qDebug() << "Type:" << static_cast<int>(type) << "Source:" << source;
    
    if (mIsStreaming) {
        qDebug() << "Cannot change video source while streaming";
        logMessage("Cannot change video source while streaming", Qgis::MessageLevel::Warning);
        return;
    }
    
    mVideoSourceType = type;
    mVideoSource = source;
    
    qDebug() << "Setting video source type to:" << static_cast<int>(type);
    
    switch (type) {
        case VideoSourceType::SIMULATION:
            qDebug() << "Setting up simulation video";
            setupSimulationVideo();
            break;
        case VideoSourceType::CAMERA:
            qDebug() << "Setting up camera video";
            setupCameraVideo();
            break;
        case VideoSourceType::FILE:
            qDebug() << "Setting up file video";
            setupFileVideo();
            break;
    }
    
    qDebug() << "=== setVideoSource completed ===";
    logMessage(QString("Video source set to: %1").arg(static_cast<int>(type)), 
               Qgis::MessageLevel::Info);
};

void VideoManager::setVideoDisplayWidget(VideoDisplayWidget *widget) {
    qDebug() << "=== setVideoDisplayWidget called ===";
    qDebug() << "Widget pointer:" << widget;
    
    mpVideoDisplayWidget = widget;
    
    if (mpVideoDisplayWidget) {
        qDebug() << "VideoDisplayWidget is valid, connecting signals...";
        
        // 连接信号槽（不再需要videoFrameReady，因为视频直接显示在QVideoWidget中）
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
        
        // 设置真正的视频控件到显示区域
        if (mpVideoWidget) {
            qDebug() << "Setting video widget to display widget...";
            mpVideoDisplayWidget->setVideoWidget(mpVideoWidget);
        } else {
            qDebug() << "WARNING: mpVideoWidget is null!";
        }
        
        logMessage("Video display widget connected", Qgis::MessageLevel::Success);
    } else {
        qDebug() << "ERROR: VideoDisplayWidget is null!";
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
    
    // 根据视频源类型执行不同的启动逻辑
    if (mVideoSourceType == VideoSourceType::FILE && mpMediaPlayer) {
        // 对于视频文件，启动媒体播放器
        qDebug() << "About to start video playback...";
        qDebug() << "Media player state before play:" << mpMediaPlayer->state();
        qDebug() << "Media status before play:" << mpMediaPlayer->mediaStatus();
        
        mpMediaPlayer->play();
        
        qDebug() << "Media player state after play:" << mpMediaPlayer->state();
        qDebug() << "Media status after play:" << mpMediaPlayer->mediaStatus();
        logMessage("Starting video file playback", Qgis::MessageLevel::Info);
    }
    
    mpVideoTimer->start();
    
    // 同时启动AI识别
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
    
    // 根据视频源类型执行不同的停止逻辑
    if (mVideoSourceType == VideoSourceType::FILE && mpMediaPlayer) {
        mpMediaPlayer->stop();
        logMessage("Stopping video file playback", Qgis::MessageLevel::Info);
    }
    
    // 停止AI识别
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
    
    // 创建AI进程
    mpAIProcess = new QProcess(this);
    connect(mpAIProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &VideoManager::onAIProcessFinished);
    connect(mpAIProcess, &QProcess::errorOccurred,
            this, &VideoManager::onAIProcessError);
    
    // 设置Python脚本参数
    QStringList arguments;
    arguments << mPythonScriptPath;
    arguments << "--port" << QString::number(mTcpPort);
    
    // 根据视频源类型设置不同的参数
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
    
    // 启动Python脚本
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
    
    // 终止Python进程
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
    // 这个定时器现在只用于AI检测的心跳
    // 真正的视频播放由QMediaPlayer和QVideoWidget处理
    if (!mIsStreaming) {
        return;
    }
    
    mFrameCount++;
    
    // 发送帧数据给Python识别系统
    if (mIsAIRunning && mpTcpSocket && mpTcpSocket->state() == QTcpSocket::ConnectedState) {
        // Python端会独立读取视频文件进行识别
    }
}



void VideoManager::loadVideoFrames() {
    // 不再需要预加载模拟帧
    logMessage("Video system initialized", Qgis::MessageLevel::Info);
}

void VideoManager::setupSimulationVideo() {
    // 模拟视频设置
    mVideoSource = "simulation";
    logMessage("Simulation video setup completed", Qgis::MessageLevel::Info);
}

void VideoManager::setupCameraVideo() {
    // 摄像头设置
    mVideoSource = "camera";
    logMessage("Camera video setup completed", Qgis::MessageLevel::Info);
}

void VideoManager::setupFileVideo() {
    qDebug() << "=== setupFileVideo called ===";
    
    // 文件视频设置 - mVideoSource应该已经由MainWindow设置好了
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
    
    // 设置媒体文件
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
    
    // 添加调试信息
    qDebug() << "Media player state:" << mpMediaPlayer->state();
    qDebug() << "Media player media status:" << mpMediaPlayer->mediaStatus();
    qDebug() << "Video available:" << mpMediaPlayer->isVideoAvailable();
    qDebug() << "Audio available:" << mpMediaPlayer->isAudioAvailable();
    
        // 设置音量为0（静音）
    mpMediaPlayer->setVolume(0);
    
    // 添加延迟检查，看看媒体是否成功加载
    QTimer::singleShot(3000, this, [this]() {
        qDebug() << "=== 3秒后检查媒体状态 ===";
        qDebug() << "Media player state:" << mpMediaPlayer->state();
        qDebug() << "Media player status:" << mpMediaPlayer->mediaStatus();
        qDebug() << "Video available:" << mpMediaPlayer->isVideoAvailable();
        qDebug() << "Audio available:" << mpMediaPlayer->isAudioAvailable();
        qDebug() << "Duration:" << mpMediaPlayer->duration() << "ms";
        qDebug() << "Error:" << mpMediaPlayer->error();
        if (mpMediaPlayer->error() != QMediaPlayer::NoError) {
            qDebug() << "Error string:" << mpMediaPlayer->errorString();
        }
    });

    qDebug() << "=== setupFileVideo completed ===";
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
        mpTcpSocket = nullptr;
    }
    
    if (mpTcpServer) {
        mpTcpServer->close();
        delete mpTcpServer;
        mpTcpServer = nullptr;
    }
    
    logMessage("TCP server stopped", Qgis::MessageLevel::Info);
}

void VideoManager::onNewConnection() {
    if (mpTcpSocket) {
        // 已有连接，拒绝新连接
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
    
    // 处理完整的JSON消息
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
        
        // 根据类别设置检测类型
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
        
        // 评估风险等级
        result.isRisk = detObj["risk"].toBool();
        result.riskLevel = detObj["risk_level"].toString();
        
        results.append(result);
    }
    
    // 检查是否包含检测框图像
    if (data.contains("frame_image")) {
        QString frameBase64 = data["frame_image"].toString();
        if (!frameBase64.isEmpty()) {
            // 解码base64图像
            QByteArray imageData = QByteArray::fromBase64(frameBase64.toUtf8());
            QPixmap framePixmap;
            if (framePixmap.loadFromData(imageData, "JPEG")) {
                qDebug() << "=== 接收到检测框图像 ===" << framePixmap.size();
                // 发送带检测框的图像到显示控件
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
            // 输出视频信息
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
            // 循环播放
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
    // 视频播放位置变化时的处理
    // 这里可以更新当前帧信息或进度
    if (mIsStreaming && mFrameCount % 30 == 0) { // 每秒更新一次日志
        qint64 duration = mpMediaPlayer->duration();
        if (duration > 0) {
            int progress = (position * 100) / duration;
            logMessage(QString("Video playback progress: %1%").arg(progress), 
                       Qgis::MessageLevel::Info);
        }
    }
}

