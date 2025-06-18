#ifndef VIDEODISPLAYWIDGET_H
#define VIDEODISPLAYWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTextEdit>
#include <QScrollArea>
#include <QTimer>
#include <QListWidget>
#include <QPushButton>
#include <QProgressBar>
#include <QFrame>
#include <QPainter>
#include <QVideoWidget>
#include <memory>

// 检测目标类型
enum class DetectionType {
    PERSON = 0,        // 人流
    MANHOLE_COVER,     // 井盖
    ELECTRIC_BIKE,     // 电瓶车
    DROWNING_POINT,    // 溺水点
    UNKNOWN
};

// 检测结果结构
struct DetectionResult {
    DetectionType type;
    QString name;
    float confidence;
    int x, y, width, height;  // 边界框坐标
    bool isRisk;              // 是否为风险目标
    QString riskLevel;        // 风险等级: "低", "中", "高"
    QString timestamp;        // 检测时间戳
};

// 视频显示控件
class VideoDisplayWidget : public QWidget {
    Q_OBJECT

public:
    VideoDisplayWidget(QWidget *parent = nullptr);
    ~VideoDisplayWidget() = default;

    // 更新视频帧
    void updateVideoFrame(const QPixmap &frame);
    
    // 更新检测结果
    void updateDetectionResults(const QList<DetectionResult> &results);
    
    // 设置视频状态
    void setVideoStatus(bool isPlaying);
    
    // 设置视频控件
    void setVideoWidget(QWidget *videoWidget);
    
private slots:
    void onClearResults();
    void onSaveResults();
    void updateVideoInfo();

signals:
    void videoControlClicked(bool start);
    void clearResultsRequested();
    void saveResultsRequested();

private:
    void setupUI();
    void setupVideoArea();
    void setupControlArea();
    void setupResultsArea();
    void setupRiskArea();
    
    QString getDetectionTypeString(DetectionType type);
    QString getRiskLevelColor(const QString &riskLevel);
    void addDetectionToResults(const DetectionResult &result);
    void updateRiskStatistics();
    
    // UI组件
    QVBoxLayout *mpMainLayout;
    
    // 视频显示区域
    QGroupBox *mpVideoGroup;
    QLabel *mpVideoLabel;
    QLabel *mpVideoStatusLabel;
    QProgressBar *mpVideoProgressBar;
    
    // 控制区域
    QGroupBox *mpControlGroup;
    QPushButton *mpStartStopBtn;
    QPushButton *mpClearBtn;
    QPushButton *mpSaveBtn;
    QLabel *mpFpsLabel;
    QLabel *mpResolutionLabel;
    
    // 检测结果区域
    QGroupBox *mpResultsGroup;
    QListWidget *mpResultsList;
    QScrollArea *mpResultsScrollArea;
    
    // 风险预警区域
    QGroupBox *mpRiskGroup;
    QLabel *mpRiskStatusLabel;
    QLabel *mpPersonCountLabel;
    QLabel *mpVehicleCountLabel;
    QLabel *mpRiskCountLabel;
    QTextEdit *mpRiskDetails;
    
    // 统计数据
    int mPersonCount;
    int mVehicleCount;
    int mRiskCount;
    int mTotalDetections;
    
    // 视频信息
    bool mIsVideoPlaying;
    QString mVideoResolution;
    int mCurrentFPS;
    
    QTimer *mpUpdateTimer;
};

// 识别结果项控件
class DetectionResultItem : public QWidget {
    Q_OBJECT

public:
    DetectionResultItem(const DetectionResult &result, QWidget *parent = nullptr);
    ~DetectionResultItem() = default;

private:
    void setupUI(const DetectionResult &result);
    QString formatConfidence(float confidence);
    
    QHBoxLayout *mpLayout;
    QLabel *mpTypeLabel;
    QLabel *mpConfidenceLabel;
    QLabel *mpPositionLabel;
    QLabel *mpRiskLabel;
    QLabel *mpTimeLabel;
};



#endif // VIDEODISPLAYWIDGET_H 