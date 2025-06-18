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
#include <QDialog>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QSizePolicy>
#include <QDesktopWidget>
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
    int x, y, width, height; 
    bool isRisk;             
    QString riskLevel;       
    QString timestamp;      
};


class FullScreenVideoViewer : public QDialog {
    Q_OBJECT

public:
    FullScreenVideoViewer(QWidget *parent = nullptr);
    ~FullScreenVideoViewer() = default;
    

    void setVideoContent(const QPixmap &pixmap);
    void setVideoWidget(QWidget *videoWidget);
    

    void moveToBottomRight();
    
protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    
private:
    void setupUI();
    
    QVBoxLayout *mpLayout;
    QLabel *mpVideoLabel;
    QWidget *mpCurrentVideoWidget;
};


class ClickableVideoLabel : public QLabel {
    Q_OBJECT

public:
    ClickableVideoLabel(QWidget *parent = nullptr);

signals:
    void doubleClicked();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
};


class VideoDisplayWidget : public QWidget {
    Q_OBJECT

public:
    VideoDisplayWidget(QWidget *parent = nullptr);
    ~VideoDisplayWidget();

    void updateVideoFrame(const QPixmap &frame);
    
    void updateDetectionResults(const QList<DetectionResult> &results);
    
    void setVideoStatus(bool isPlaying);
    
    void setVideoWidget(QWidget *videoWidget);
    
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    
private slots:
    void onClearResults();
    void onSaveResults();
    void updateVideoInfo();
    void onVideoLabelDoubleClicked();

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
    
    QVBoxLayout *mpMainLayout;
    
    // 视频显示区域
    QGroupBox *mpVideoGroup;
    ClickableVideoLabel *mpVideoLabel;
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
    
    int mPersonCount;
    int mVehicleCount;
    int mRiskCount;
    int mTotalDetections;
    
    bool mIsVideoPlaying;
    QString mVideoResolution;
    int mCurrentFPS;
    
    QTimer *mpUpdateTimer;
    

    FullScreenVideoViewer *mpFullScreenViewer;
    QWidget *mpCurrentVideoWidget;
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