#ifndef VIDEOMANAGER_H
#define VIDEOMANAGER_H

#include <QObject>
#include <QTimer>
#include <QPixmap>
#include <QProcess>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMutex>
#include <QQueue>
#include <QDateTime>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <memory>
#include "../gui/VideoDisplayWidget.h"

// 视频来源类型
enum class VideoSourceType {
    SIMULATION,    
    CAMERA,      
    FILE       
};


class VideoManager : public QObject {
    Q_OBJECT

private:
    VideoManager(QObject *parent = nullptr);
    ~VideoManager();

public:
    static VideoManager& getInstance() {
        static VideoManager instance;
        return instance;
    }
    
    VideoManager(const VideoManager&) = delete;
    VideoManager& operator=(const VideoManager&) = delete;


    bool initialize();
    

    void setVideoSource(VideoSourceType type, const QString &source = "");
    

    void startVideoStream();
    void stopVideoStream();
    

    bool isVideoStreaming() const { return mIsStreaming; }
    

    void setVideoDisplayWidget(VideoDisplayWidget *widget);
    

    void startAIDetection();
    void stopAIDetection();
    

    bool isAIDetectionRunning() const { return mIsAIRunning; }

private slots:
    void onVideoTimer();
    void onAIProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onAIProcessError(QProcess::ProcessError error);
    void onNewConnection();
    void onSocketReadyRead();
    void onSocketDisconnected();
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onPositionChanged(qint64 position);

signals:
    void videoFrameReady(const QPixmap &frame);
    void detectionResultsReady(const QList<DetectionResult> &results);
    void videoStatusChanged(bool isStreaming);
    void aiStatusChanged(bool isRunning);
    void errorOccurred(const QString &error);

private:
    void setupSimulationVideo();
    void setupCameraVideo();
    void setupFileVideo();
    
    void startTcpServer();
    void stopTcpServer();
    
    void processDetectionData(const QJsonObject &data);
  
    void loadVideoFrames();
    
    // 视频相关
    VideoSourceType mVideoSourceType;
    QString mVideoSource;
    bool mIsStreaming;
    QTimer *mpVideoTimer;
    VideoDisplayWidget *mpVideoDisplayWidget;
    

    
    // 媒体播放器（用于文件播放）
    QMediaPlayer *mpMediaPlayer;
    QVideoWidget *mpVideoWidget;
    QPixmap mCurrentVideoFrame;
    

    
    // AI识别相关
    bool mIsAIRunning;
    QProcess *mpAIProcess;
    QString mPythonScriptPath;
    QString mPythonExecutable;
    
    // TCP通信相关
    QTcpServer *mpTcpServer;
    QTcpSocket *mpTcpSocket;
    quint16 mTcpPort;
    QByteArray mSocketBuffer;
    
    // 线程安全
    QMutex mMutex;
    
    // 统计信息
    int mFrameCount;
    int mDetectionCount;
    QDateTime mStartTime;
    
    // 配置参数
    static constexpr int DEFAULT_FPS = 30;
    static constexpr quint16 DEFAULT_TCP_PORT = 8888;
    static constexpr int VIDEO_TIMER_INTERVAL = 33; // ~30 FPS
};

// 视频帧数据结构
struct VideoFrame {
    QPixmap pixmap;
    QString timestamp;
    int frameIndex;
    bool hasDetections;
    QList<DetectionResult> detections;
};

#endif // VIDEOMANAGER_H 