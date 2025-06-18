#include "VideoDisplayWidget.h"
#include "../log/QgisDebug.h"
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QSplitter>
#include <QApplication>


FullScreenVideoViewer::FullScreenVideoViewer(QWidget *parent) 
    : QDialog(parent), mpCurrentVideoWidget(nullptr) {
    setupUI();
    
 
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    setModal(true);
    setStyleSheet("background-color: black;");
    
   
    resize(640, 480);
    
  
    moveToBottomRight();
}

void FullScreenVideoViewer::setupUI() {
    mpLayout = new QVBoxLayout(this);
    mpLayout->setContentsMargins(5, 5, 5, 5);
    mpLayout->setSpacing(0);
    
    mpVideoLabel = new QLabel(this);
    mpVideoLabel->setAlignment(Qt::AlignCenter);
    mpVideoLabel->setScaledContents(true);
    mpVideoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mpVideoLabel->setStyleSheet("background-color: black; color: white;");
    mpVideoLabel->setText("ESC to exit");
    
    mpLayout->addWidget(mpVideoLabel);
}

void FullScreenVideoViewer::setVideoContent(const QPixmap &pixmap) {
    if (!pixmap.isNull()) {
        mpVideoLabel->setPixmap(pixmap);
        mpVideoLabel->setScaledContents(true);
        mpVideoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
}

void FullScreenVideoViewer::setVideoWidget(QWidget *videoWidget) {
    if (mpCurrentVideoWidget) {
        mpLayout->removeWidget(mpCurrentVideoWidget);
    }
    
    mpCurrentVideoWidget = videoWidget;
    if (videoWidget) {
        mpLayout->removeWidget(mpVideoLabel);
        mpVideoLabel->hide();
        

      
        videoWidget->setMinimumSize(320, 240);
        videoWidget->setMaximumSize(620, 460);
        videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        
        mpLayout->addWidget(videoWidget);
        videoWidget->show();
    }
}

void FullScreenVideoViewer::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        accept(); 
    } else {
        QDialog::keyPressEvent(event);
    }
}

void FullScreenVideoViewer::mouseDoubleClickEvent(QMouseEvent *event) {
    Q_UNUSED(event)
    accept(); 
}

void FullScreenVideoViewer::moveToBottomRight() {
 
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    
  
    int margin = 20;
    int x = screenGeometry.width() - width() - margin;
    int y = screenGeometry.height() - height() - margin;
    
 
    move(x, y);
}


ClickableVideoLabel::ClickableVideoLabel(QWidget *parent) : QLabel(parent) {
    setCursor(Qt::PointingHandCursor);
    setToolTip("double click to enlarge the video");
}

void ClickableVideoLabel::mouseDoubleClickEvent(QMouseEvent *event) {
    Q_UNUSED(event)
    emit doubleClicked();
}


VideoDisplayWidget::VideoDisplayWidget(QWidget *parent) 
    : QWidget(parent)
    , mPersonCount(0)
    , mVehicleCount(0)
    , mRiskCount(0)
    , mTotalDetections(0)
    , mIsVideoPlaying(false)
    , mVideoResolution("1920x1080")
    , mCurrentFPS(30)
    , mpFullScreenViewer(nullptr)
    , mpCurrentVideoWidget(nullptr)
{
    setupUI();
    
 
    mpFullScreenViewer = new FullScreenVideoViewer(this);
    
    mpUpdateTimer = new QTimer(this);
    connect(mpUpdateTimer, &QTimer::timeout, this, &VideoDisplayWidget::updateVideoInfo);
    mpUpdateTimer->start(1000); 
    
    logMessage("VideoDisplayWidget created", Qgis::MessageLevel::Success);
}

VideoDisplayWidget::~VideoDisplayWidget() {
  
    if (mpUpdateTimer) {
        mpUpdateTimer->stop();
    }
    
   
    if (mpFullScreenViewer) {
        mpFullScreenViewer->close();
       
    }
    
   
    if (mpResultsList) {
        mpResultsList->clear();
    }
   
    mpCurrentVideoWidget = nullptr;
    
    logMessage("VideoDisplayWidget destroyed", Qgis::MessageLevel::Success);
}

void VideoDisplayWidget::setupUI() {
    mpMainLayout = new QVBoxLayout(this);
    mpMainLayout->setContentsMargins(2, 2, 2, 2);
    mpMainLayout->setSpacing(5);
    
    setupVideoArea();
    setupControlArea();
    setupResultsArea();
    setupRiskArea();
    
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #555555;
            border-radius: 8px;
            margin-top: 1ex;
            padding-top: 10px;
            background-color: #353535;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
            color: #CCCCCC;
        }
        QPushButton {
            background-color: #505050;
            border: 1px solid #606060;
            border-radius: 4px;
            color: #DDD;
            padding: 5px 10px;
            min-width: 60px;
        }
        QPushButton:hover {
            background-color: #606060;
        }
        QPushButton:pressed {
            background-color: #404040;
        }
        QLabel {
            color: #CCCCCC;
        }
        QTextEdit {
            background-color: #404040;
            border: 1px solid #555555;
            color: #CCCCCC;
        }
        QListWidget {
            background-color: #404040;
            border: 1px solid #555555;
            color: #CCCCCC;
        }
        QProgressBar {
            border: 1px solid #555555;
            border-radius: 3px;
            background-color: #404040;
        }
        QProgressBar::chunk {
            background-color: #4CAF50;
            border-radius: 2px;
        }
    )");
}

void VideoDisplayWidget::setupVideoArea() {
    mpVideoGroup = new QGroupBox("video", this);
    QVBoxLayout *videoLayout = new QVBoxLayout(mpVideoGroup);
    
  
    mpVideoLabel = new ClickableVideoLabel(mpVideoGroup);
    mpVideoLabel->setMinimumSize(240, 120);
    mpVideoLabel->setMaximumSize(320, 240);
    mpVideoLabel->setScaledContents(true);
    mpVideoLabel->setStyleSheet("border: 1px solid #666666; background-color: #2D2D2D;");
    mpVideoLabel->setAlignment(Qt::AlignCenter);
    mpVideoLabel->setText("wait video...");
    
    
    connect(mpVideoLabel, &ClickableVideoLabel::doubleClicked, 
            this, &VideoDisplayWidget::onVideoLabelDoubleClicked);
    
    mpVideoStatusLabel = new QLabel("video is not connected", mpVideoGroup);
    mpVideoProgressBar = new QProgressBar(mpVideoGroup);
    mpVideoProgressBar->setVisible(false);
    
    videoLayout->addWidget(mpVideoLabel);
    videoLayout->addWidget(mpVideoStatusLabel);
    videoLayout->addWidget(mpVideoProgressBar);
    
    mpMainLayout->addWidget(mpVideoGroup);
}

void VideoDisplayWidget::setVideoWidget(QWidget *videoWidget) {
    if (!videoWidget) return;
    
  
    mpCurrentVideoWidget = videoWidget;
    
    QVBoxLayout *videoLayout = qobject_cast<QVBoxLayout*>(mpVideoGroup->layout());
    if (videoLayout) {
        videoLayout->removeWidget(mpVideoLabel);
        mpVideoLabel->hide();
        
        videoWidget->setMinimumSize(240, 120);
        videoWidget->setMaximumSize(320, 240);
        videoLayout->insertWidget(0, videoWidget);
        
       
        videoWidget->installEventFilter(this);
        
        logMessage("Video widget set successfully", Qgis::MessageLevel::Success);
    }
}

void VideoDisplayWidget::updateVideoFrame(const QPixmap &frame) {
    if (!frame.isNull()) {
        mpVideoLabel->setPixmap(frame);
        mpVideoLabel->show();
        qDebug() << "=== VideoDisplayWidget::updateVideoFrame ===" << frame.size();
    }
}

void VideoDisplayWidget::onVideoLabelDoubleClicked() {
    if (mpVideoLabel->pixmap()) {
        mpFullScreenViewer->setVideoContent(*mpVideoLabel->pixmap());
        mpFullScreenViewer->exec(); // 使用exec()而不是show()以确保模态显示
    } else if (mpCurrentVideoWidget) {
        QWidget *parent = mpCurrentVideoWidget->parentWidget();
        
   
        QSizePolicy originalPolicy = mpCurrentVideoWidget->sizePolicy();
        QSize originalMinSize = mpCurrentVideoWidget->minimumSize();
        QSize originalMaxSize = mpCurrentVideoWidget->maximumSize();
        
        mpFullScreenViewer->setVideoWidget(mpCurrentVideoWidget);
        mpFullScreenViewer->exec();
        
      
        if (parent) {
            QVBoxLayout *videoLayout = qobject_cast<QVBoxLayout*>(mpVideoGroup->layout());
            if (videoLayout) {
                videoLayout->insertWidget(0, mpCurrentVideoWidget);
                
             
                mpCurrentVideoWidget->setSizePolicy(originalPolicy);
                mpCurrentVideoWidget->setMinimumSize(originalMinSize);
                mpCurrentVideoWidget->setMaximumSize(originalMaxSize);
            }
        }
    }
}


bool VideoDisplayWidget::eventFilter(QObject *obj, QEvent *event) {
    if (obj == mpCurrentVideoWidget && event->type() == QEvent::MouseButtonDblClick) {
        onVideoLabelDoubleClicked();
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

void VideoDisplayWidget::setupControlArea() {
    mpControlGroup = new QGroupBox("video control", this);
    QVBoxLayout *controlLayout = new QVBoxLayout(mpControlGroup);
    

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    mpStartStopBtn = new QPushButton("start", mpControlGroup);
    mpClearBtn = new QPushButton("erase", mpControlGroup);
    mpSaveBtn = new QPushButton("save", mpControlGroup);
    
    mpStartStopBtn->setStyleSheet("QPushButton { background-color: #4CAF50; }");
    mpClearBtn->setStyleSheet("QPushButton { background-color: #FF9800; }");
    mpSaveBtn->setStyleSheet("QPushButton { background-color: #2196F3; }");
    
    buttonLayout->addWidget(mpStartStopBtn);
    buttonLayout->addWidget(mpClearBtn);
    buttonLayout->addWidget(mpSaveBtn);
    
 
    QHBoxLayout *infoLayout = new QHBoxLayout();
    mpFpsLabel = new QLabel("FPS: 0", mpControlGroup);
    mpResolutionLabel = new QLabel("分辨率: 1920x1080", mpControlGroup);
    
    infoLayout->addWidget(mpFpsLabel);
    infoLayout->addWidget(mpResolutionLabel);
    infoLayout->addStretch();
    
    controlLayout->addLayout(buttonLayout);
    controlLayout->addLayout(infoLayout);
    
 
    connect(mpStartStopBtn, &QPushButton::clicked, this, [this]() {
        bool start = !mIsVideoPlaying;
        emit videoControlClicked(start);
    });
    connect(mpClearBtn, &QPushButton::clicked, this, &VideoDisplayWidget::onClearResults);
    connect(mpSaveBtn, &QPushButton::clicked, this, &VideoDisplayWidget::onSaveResults);
    
    mpMainLayout->addWidget(mpControlGroup);
}

void VideoDisplayWidget::setupResultsArea() {
    mpResultsGroup = new QGroupBox("result", this);
    QVBoxLayout *resultsLayout = new QVBoxLayout(mpResultsGroup);
    
    mpResultsList = new QListWidget(mpResultsGroup);
    mpResultsList->setMaximumHeight(30);
    
    resultsLayout->addWidget(mpResultsList);
    mpMainLayout->addWidget(mpResultsGroup);
}

void VideoDisplayWidget::setupRiskArea() {
    mpRiskGroup = new QGroupBox("risk ring", this);
    QVBoxLayout *riskLayout = new QVBoxLayout(mpRiskGroup);
    

    mpRiskStatusLabel = new QLabel("risk is mormal", mpRiskGroup);
    mpRiskStatusLabel->setStyleSheet("color: #4CAF50; font-weight: bold;");
    

    QHBoxLayout *statsLayout = new QHBoxLayout();
    mpPersonCountLabel = new QLabel("person: 0", mpRiskGroup);
    mpVehicleCountLabel = new QLabel("electricScooter: 0", mpRiskGroup);
    mpRiskCountLabel = new QLabel("risk: 0", mpRiskGroup);
    
    mpRiskCountLabel->setStyleSheet("color: #FF5722; font-weight: bold;");
    
    statsLayout->addWidget(mpPersonCountLabel);
    statsLayout->addWidget(mpVehicleCountLabel);
    statsLayout->addWidget(mpRiskCountLabel);
    statsLayout->addStretch();
    

    mpRiskDetails = new QTextEdit(mpRiskGroup);
    mpRiskDetails->setMaximumHeight(50);
    mpRiskDetails->setPlainText("not exist risk");
    
    riskLayout->addWidget(mpRiskStatusLabel);
    riskLayout->addLayout(statsLayout);
    riskLayout->addWidget(mpRiskDetails);
    
    mpMainLayout->addWidget(mpRiskGroup);
}



void VideoDisplayWidget::updateDetectionResults(const QList<DetectionResult> &results) {
    qDebug() << "=== VideoDisplayWidget::updateDetectionResults called ===";
    qDebug() << "Received" << results.size() << "detection results";
    
    for (const auto &result : results) {
        qDebug() << "Processing detection:" << result.name 
                 << "confidence:" << result.confidence
                 << "position:" << result.x << "," << result.y
                 << "risk:" << result.isRisk;
        addDetectionToResults(result);
    }
    updateRiskStatistics();
    
    qDebug() << "Updated statistics - Person:" << mPersonCount 
             << "Vehicle:" << mVehicleCount 
             << "Risk:" << mRiskCount 
             << "Total:" << mTotalDetections;
}

void VideoDisplayWidget::setVideoStatus(bool isPlaying) {
    mIsVideoPlaying = isPlaying;
    
    if (isPlaying) {
        mpStartStopBtn->setText("pause");
        mpStartStopBtn->setStyleSheet("QPushButton { background-color: #F44336; }");
        mpVideoStatusLabel->setText("video is playing");
        mpVideoProgressBar->setVisible(true);
    } else {
        mpStartStopBtn->setText("start");
        mpStartStopBtn->setStyleSheet("QPushButton { background-color: #4CAF50; }");
        mpVideoStatusLabel->setText("video is pause");
        mpVideoProgressBar->setVisible(false);
    }
}

void VideoDisplayWidget::addDetectionToResults(const DetectionResult &result) {
    QString itemText = QString("[%1] %2 (%.1f%%) - %3")
                       .arg(result.timestamp)
                       .arg(getDetectionTypeString(result.type))
                       .arg(result.confidence * 100)
                       .arg(result.isRisk ? "risk" : "normal");
    
    mpResultsList->addItem(itemText);
    
 
    if (mpResultsList->count() > 100) {
        delete mpResultsList->takeItem(0);
    }
    

    mpResultsList->scrollToBottom();
    

    mTotalDetections++;
    

    switch (result.type) {
        case DetectionType::PERSON:
            mPersonCount++;
            break;
        case DetectionType::ELECTRIC_BIKE:
            mVehicleCount++;
            break;
        default:
            break;
    }
    

    if (result.isRisk) {
        mRiskCount++;
    }
}

void VideoDisplayWidget::updateRiskStatistics() {

    mpPersonCountLabel->setText(QString("person: %1").arg(mPersonCount));
    mpVehicleCountLabel->setText(QString("electricScooter: %1").arg(mVehicleCount));
    mpRiskCountLabel->setText(QString("risk: %1").arg(mRiskCount));
    
    // 更新风险状态
    if (mRiskCount > 0) {
        mpRiskStatusLabel->setText("risk ring");
        mpRiskStatusLabel->setStyleSheet("color: #FF5722; font-weight: bold;");
    } else {
        mpRiskStatusLabel->setText("normal");
        mpRiskStatusLabel->setStyleSheet("color: #4CAF50; font-weight: bold;");
    }
}

QString VideoDisplayWidget::getDetectionTypeString(DetectionType type) {
    switch (type) {
        case DetectionType::PERSON:
            return "person";
        case DetectionType::MANHOLE_COVER:
            return "Manhole cover";
        case DetectionType::ELECTRIC_BIKE:
            return "electric scooter";
        case DetectionType::DROWNING_POINT:
            return "water risk";
        default:
            return "unkonw";
    }
}

QString VideoDisplayWidget::getRiskLevelColor(const QString &riskLevel) {
    if (riskLevel == "high") return "#FF5722";
    if (riskLevel == "midel") return "#FF9800";
    if (riskLevel == "low") return "#FFC107";
    return "#4CAF50";
}

void VideoDisplayWidget::onClearResults() {
    mpResultsList->clear();
    mPersonCount = 0;
    mVehicleCount = 0;
    mRiskCount = 0;
    mTotalDetections = 0;
    
    updateRiskStatistics();
    mpRiskDetails->setPlainText("no risk");
    
    emit clearResultsRequested();
    logMessage("Detection results cleared", Qgis::MessageLevel::Info);
}

void VideoDisplayWidget::onSaveResults() {
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "save result",
        QString("detection_results_%1.txt").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "Text Files (*.txt)"
    );
    
    if (!fileName.isEmpty()) {
        emit saveResultsRequested();
        QMessageBox::information(this, "save", "in" + fileName);
    }
}

void VideoDisplayWidget::updateVideoInfo() {
    if (mIsVideoPlaying) {
        mpFpsLabel->setText(QString("FPS: %1").arg(mCurrentFPS));
        mpResolutionLabel->setText(QString("grid: %1").arg(mVideoResolution));
    }
}


DetectionResultItem::DetectionResultItem(const DetectionResult &result, QWidget *parent)
    : QWidget(parent) {
    setupUI(result);
}

void DetectionResultItem::setupUI(const DetectionResult &result) {
    mpLayout = new QHBoxLayout(this);
    mpLayout->setContentsMargins(5, 2, 5, 2);
    

    mpTypeLabel = new QLabel(this);
    switch (result.type) {
        case DetectionType::PERSON:
            mpTypeLabel->setText("👤");
            break;
        case DetectionType::MANHOLE_COVER:
            mpTypeLabel->setText("🔘");
            break;
        case DetectionType::ELECTRIC_BIKE:
            mpTypeLabel->setText("🛵");
            break;
        case DetectionType::DROWNING_POINT:
            mpTypeLabel->setText("💧");
            break;
        default:
            mpTypeLabel->setText("unkown");
            break;
    }
    

    mpConfidenceLabel = new QLabel(formatConfidence(result.confidence), this);
    

    mpPositionLabel = new QLabel(QString("(%1,%2)").arg(result.x).arg(result.y), this);
    

    mpRiskLabel = new QLabel(this);
    if (result.isRisk) {
        mpRiskLabel->setText("⚠️");
        mpRiskLabel->setStyleSheet("color: #FF5722;");
    } else {
        mpRiskLabel->setText("ok");
        mpRiskLabel->setStyleSheet("color: #4CAF50;");
    }
    

    mpTimeLabel = new QLabel(result.timestamp, this);
    mpTimeLabel->setStyleSheet("color: #888888; font-size: 10px;");
    
    mpLayout->addWidget(mpTypeLabel);
    mpLayout->addWidget(mpConfidenceLabel);
    mpLayout->addWidget(mpPositionLabel);
    mpLayout->addWidget(mpRiskLabel);
    mpLayout->addStretch();
    mpLayout->addWidget(mpTimeLabel);
    
    setStyleSheet("DetectionResultItem { border-bottom: 1px solid #555555; }");
}

QString DetectionResultItem::formatConfidence(float confidence) {
    return QString("%1%").arg(static_cast<int>(confidence * 100));
}

 
