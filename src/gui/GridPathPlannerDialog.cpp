
#include "GridPathPlannerDialog.h"
#include "../log/QgisDebug.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>

GridPathPlannerDialog::GridPathPlannerDialog(QWidget *parent)
    : QDialog(parent)
    , mPlanner(new GridPathPlanner(this))
    , mPlanningInProgress(false) {
    
    setWindowTitle("planner by fishnet");
    setMinimumSize(900, 700);
    resize(1100, 800);
    
    initializeUI();
    connectSignals();
    setInitialValues();
    updateUIState(true);
    
    logMessage("Grid path planner dialog initialized", Qgis::MessageLevel::Info);
}

GridPathPlannerDialog::~GridPathPlannerDialog() {
    
}

GridPlanningParameters GridPathPlannerDialog::getPlanningParameters() const {
    GridPlanningParameters params;
    
    params.fishnetShapefile = mFishnetEdit->text();
    params.riskPointsShapefile = mRiskPointsEdit->text();
    params.outputPath = mOutputEdit->text();
    params.startPoint = QVector3D(
        static_cast<float>(mStartXSpin->value()),
        static_cast<float>(mStartYSpin->value()),
        static_cast<float>(mStartZSpin->value())
    );
    params.algorithm = mAlgorithmCombo->currentText();
    params.maxRiskPointDistance = mMaxDistanceSpin->value();
    
    return params;
}

LinePlanningParameters GridPathPlannerDialog::getLinePlanningParameters() const {
    LinePlanningParameters params;
    
    params.fishnetShapefile = mFishnetEdit->text();
    params.riskLinesShapefile = mRiskLinesEdit->text();
    params.outputPath = mOutputEdit->text();
    params.startPoint = QVector3D(
        static_cast<float>(mStartXSpin->value()),
        static_cast<float>(mStartYSpin->value()),
        static_cast<float>(mStartZSpin->value())
    );
    params.algorithm = mAlgorithmCombo->currentText();
    params.maxRiskLineDistance = mMaxRiskLineDistanceSpin->value();
    
    return params;
}

AreaPlanningParameters GridPathPlannerDialog::getAreaPlanningParameters() const {
    AreaPlanningParameters params;
    
    params.fishnetShapefile = mFishnetEdit->text();
    params.outputPath = mOutputEdit->text();
    params.startPoint = QVector3D(
        static_cast<float>(mStartXSpin->value()),
        static_cast<float>(mStartYSpin->value()),
        static_cast<float>(mStartZSpin->value())
    );
    params.algorithm = mAlgorithmCombo->currentText();
    params.coverageThreshold = mCoverageThresholdSpin->value() / 100.0;
    params.maxIterations = mMaxIterationsSpin->value();
    params.gridSpacing = mGridSpacingSpin->value();
    
    return params;
}

void GridPathPlannerDialog::setPlanningParameters(const GridPlanningParameters& params) {
    mFishnetEdit->setText(params.fishnetShapefile);
    mRiskPointsEdit->setText(params.riskPointsShapefile);
    mOutputEdit->setText(params.outputPath);
    mStartXSpin->setValue(params.startPoint.x());
    mStartYSpin->setValue(params.startPoint.y());
    mStartZSpin->setValue(params.startPoint.z());
    mMaxDistanceSpin->setValue(params.maxRiskPointDistance);
    
    int algorithmIndex = mAlgorithmCombo->findText(params.algorithm);
    if (algorithmIndex >= 0) {
        mAlgorithmCombo->setCurrentIndex(algorithmIndex);
    }
}

void GridPathPlannerDialog::resetToDefaults() {
    GridPlanningParameters defaultParams;
    setPlanningParameters(defaultParams);
    mResultsText->clear();
    mProgressBar->setValue(0);
    mProgressLabel->setText("start to plan");
    mCurrentResult = GridPlanningResult();
}

void GridPathPlannerDialog::onProgressUpdated(int percentage, const QString& message) {
    mProgressBar->setValue(percentage);
    mProgressLabel->setText(message);
    
    // 在结果文本中添加进度信息
    mResultsText->append(QString("[%1%] %2").arg(percentage).arg(message));
    QCoreApplication::processEvents(); // 强制更新UI
}

void GridPathPlannerDialog::onPlanningCompleted(const GridPlanningResult& result) {
    mCurrentResult = result;
    mPlanningInProgress = false;
    updateUIState(true);
    
    if (result.success) {
        QString successMessage = QString("planning completed!\npath length: %1 meters\nvisited risk points: %2/%3")
                                .arg(result.totalPathLength, 0, 'f', 2)
                                .arg(result.visitedRiskPoints)
                                .arg(result.totalRiskPoints);
        
        mResultsText->append("\n=== planning completed ===");
        mResultsText->append(successMessage);
        mResultsText->append(mPlanner->getStatistics(result));
        
        showSuccess("planning completed!");
        
        // 启用导出和预览按钮
        mExportBtn->setEnabled(true);
        mPreviewBtn->setEnabled(true);
        
    } else {
        showError(QString("planning failed: %1").arg(result.errorMessage));
    }
}

void GridPathPlannerDialog::onLinePlanningCompleted(const LinePlanningResult& result) {
    mCurrentLineResult = result;
    mPlanningInProgress = false;
    updateUIState(true);
    
    if (result.success) {
        QString successMessage = QString("line planning completed!\npath length: %1 meters\ncompleted risk lines: %2/%3")
                                .arg(result.totalPathLength, 0, 'f', 2)
                                .arg(result.completedRiskLines)
                                .arg(result.totalRiskLines);
        
        mResultsText->append("\n=== line planning completed ===");
        mResultsText->append(successMessage);
        mResultsText->append(QString("algorithm used: %1").arg(result.algorithm));
        
        showSuccess("line planning completed!");
        

        mExportBtn->setEnabled(true);
        mPreviewBtn->setEnabled(true);
        
    } else {
        showError(QString("line planning failed: %1").arg(result.errorMessage));
    }
}

void GridPathPlannerDialog::onAreaPlanningCompleted(const AreaPlanningResult& result) {
    mCurrentAreaResult = result;
    mPlanningInProgress = false;
    updateUIState(true);
    
    if (result.success) {
        QString successMessage = QString("area planning completed!\npath length: %1 meters\ncoverage rate: %2%")
                                .arg(result.totalPathLength, 0, 'f', 2)
                                .arg(result.coverageRate * 100.0, 0, 'f', 1);
        
        mResultsText->append("\n=== area planning completed ===");
        mResultsText->append(successMessage);
        mResultsText->append(QString("algorithm used: %1").arg(result.algorithm));
        mResultsText->append(QString("covered grid cells: %1/%2").arg(result.coveredGridCells).arg(result.totalGridCells));
        mResultsText->append(QString("uncovered areas: %1").arg(result.uncoveredAreas.size()));
        
        showSuccess("area planning completed!");
        

        mExportBtn->setEnabled(true);
        mPreviewBtn->setEnabled(true);
        
    } else {
        showError(QString("area planning failed: %1").arg(result.errorMessage));
    }
}

void GridPathPlannerDialog::onPlanningFailed(const QString& errorMessage) {
    mPlanningInProgress = false;
    updateUIState(true);
    showError(QString("planning failed: %1").arg(errorMessage));
    mResultsText->append(QString("\n=== planning failed ===\n%1").arg(errorMessage));
}

void GridPathPlannerDialog::browseFishnetFile() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "select fishnet shapefile",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "Shapefile (*.shp);;all files (*.*)"
    );
    
    if (!fileName.isEmpty()) {
        mFishnetEdit->setText(fileName);
        validateInput();
    }
}

void GridPathPlannerDialog::browseRiskPointsFile() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "select risk points shapefile",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "Shapefile (*.shp);;all files (*.*)"
    );
    
    if (!fileName.isEmpty()) {
        mRiskPointsEdit->setText(fileName);
        validateInput();
    }
}

void GridPathPlannerDialog::browseRiskLinesFile() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "select risk lines shapefile",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "Shapefile (*.shp);;all files (*.*)"
    );
    
    if (!fileName.isEmpty()) {
        mRiskLinesEdit->setText(fileName);
        validateInput();
    }
}

void GridPathPlannerDialog::browseOutputDirectory() {
    QString dirName = QFileDialog::getExistingDirectory(
        this,
        "select output directory",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
    );
    
    if (!dirName.isEmpty()) {
        mOutputEdit->setText(dirName);
        validateInput();
    }
}

void GridPathPlannerDialog::executePlanning() {
    if (mPlanningInProgress) {
        showError("planning is in progress, please wait for completion");
        return;
    }
    
    QString planningMode = mPlanningModeCombo->currentText();
    
    if (planningMode == "Point Events") {
     
        GridPlanningParameters params = getPlanningParameters();
        
        auto validation = mPlanner->validateParameters(params);
        if (!validation.first) {
            showError(validation.second);
            return;
        }
        
   
        mPlanningInProgress = true;
        updateUIState(false);
        mResultsText->clear();
        mResultsText->append("start point event planning...");
        
    
        mProgressBar->setValue(0);
        mProgressLabel->setText("preparing...");
        
   
        mPlanner->asyncExecutePlanning(params);
        
    } else if (planningMode == "Line Events") {
    
        LinePlanningParameters lineParams = getLinePlanningParameters();
        
        auto validation = mPlanner->validateLineParameters(lineParams);
        if (!validation.first) {
            showError(validation.second);
            return;
        }
        
   
        mPlanningInProgress = true;
        updateUIState(false);
        mResultsText->clear();
        mResultsText->append("start line event planning...");
        
 
        mProgressBar->setValue(0);
        mProgressLabel->setText("preparing...");
        
  
        mPlanner->asyncExecuteLinePlanning(lineParams);
        
    } else if (planningMode == "Area Events") {

        AreaPlanningParameters areaParams = getAreaPlanningParameters();
        

        auto validation = mPlanner->validateAreaParameters(areaParams);
        if (!validation.first) {
            showError(validation.second);
            return;
        }

        mPlanningInProgress = true;
        updateUIState(false);
        mResultsText->clear();
        mResultsText->append("start area event planning...");
        

        mProgressBar->setValue(0);
        mProgressLabel->setText("preparing...");

        mPlanner->asyncExecuteAreaPlanning(areaParams);
        
    } else {
        showError("unknown planning mode");
    }
}

void GridPathPlannerDialog::exportResults() {
    QString planningMode = mPlanningModeCombo->currentText();
    
    if (planningMode == "Point Events") {

        if (!mCurrentResult.success) {
            showError("no point event results to export");
            return;
        }
        
        QString outputPath = mOutputEdit->text();
        if (outputPath.isEmpty()) {
            outputPath = QFileDialog::getExistingDirectory(
                this,
                "select export directory",
                QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
            );
            
            if (outputPath.isEmpty()) return;
        }
        
        if (mPlanner->exportPathToFiles(mCurrentResult, outputPath)) {
            showSuccess(QString("point event results exported to: %1").arg(outputPath));
            mResultsText->append(QString("\nexport completed: %1").arg(outputPath));
        } else {
            showError("point event export failed");
        }
        
    } else if (planningMode == "Line Events") {

        if (!mCurrentLineResult.success) {
            showError("no line event results to export");
            return;
        }
        
        QString outputPath = mOutputEdit->text();
        if (outputPath.isEmpty()) {
            outputPath = QFileDialog::getExistingDirectory(
                this,
                "select export directory",
                QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
            );
            
            if (outputPath.isEmpty()) return;
        }
        
        if (mPlanner->exportLinePathToFiles(mCurrentLineResult, outputPath)) {
            showSuccess(QString("line event results exported to: %1").arg(outputPath));
            mResultsText->append(QString("\nline event export completed: %1").arg(outputPath));
        } else {
            showError("line event export failed");
        }
        
    } else if (planningMode == "Area Events") {

        if (!mCurrentAreaResult.success) {
            showError("no area event results to export");
            return;
        }
        
        QString outputPath = mOutputEdit->text();
        if (outputPath.isEmpty()) {
            outputPath = QFileDialog::getExistingDirectory(
                this,
                "select export directory",
                QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
            );
            
            if (outputPath.isEmpty()) return;
        }
        
        if (mPlanner->exportAreaPathToFiles(mCurrentAreaResult, outputPath)) {
            showSuccess(QString("area event results exported to: %1").arg(outputPath));
            mResultsText->append(QString("\narea event export completed: %1").arg(outputPath));
        } else {
            showError("area event export failed");
        }
        
    } else {
        showError("unknown planning mode");
    }
}

void GridPathPlannerDialog::previewResults() {
    QString planningMode = mPlanningModeCombo->currentText();
    
    if (planningMode == "Point Events") {

        if (!mCurrentResult.success) {
            showError("no point event results to preview");
            return;
        }
        
        emit showResults(mCurrentResult);
        
    } else if (planningMode == "Line Events") {

        if (!mCurrentLineResult.success) {
            showError("no line event results to preview");
            return;
        }
        

        QString stats = mPlanner->getLineStatistics(mCurrentLineResult);
        mResultsText->append("\n=== 线事件规划预览 ===");
        mResultsText->append(stats);
        
    } else if (planningMode == "Area Events") {

        if (!mCurrentAreaResult.success) {
            showError("no area event results to preview");
            return;
        }
        

        emit showAreaResults(mCurrentAreaResult);
        

        QString stats = mPlanner->getAreaStatistics(mCurrentAreaResult);
        mResultsText->append("\n=== 面事件规划预览 ===");
        mResultsText->append(stats);
        
    } else {
        showError("unknown planning mode");
    }
}

void GridPathPlannerDialog::resetForm() {
    resetToDefaults();
    mCurrentLineResult = LinePlanningResult(); 
    mCurrentAreaResult = AreaPlanningResult(); 
    mExportBtn->setEnabled(false);
    mPreviewBtn->setEnabled(false);
}

void GridPathPlannerDialog::validateInput() {
    bool valid = true;
    

    if (!validateFilePath(mFishnetEdit->text())) {
        valid = false;
    }
    
    QString planningMode = mPlanningModeCombo->currentText();
    
    if (planningMode == "Point Events") {

        if (!validateFilePath(mRiskPointsEdit->text())) {
            valid = false;
        }
    } else if (planningMode == "Line Events") {

        if (!validateFilePath(mRiskLinesEdit->text())) {
            valid = false;
        }
    } else if (planningMode == "Area Events") {

    }
    
    mExecuteBtn->setEnabled(valid && !mPlanningInProgress);
}

void GridPathPlannerDialog::onAlgorithmChanged() {
    QString algorithm = mAlgorithmCombo->currentText();
    updateAlgorithmDescription(algorithm);
}

void GridPathPlannerDialog::onPlanningModeChanged() {
    QString planningMode = mPlanningModeCombo->currentText();
    

    mAlgorithmCombo->clear();
    
    if (planningMode == "Point Events") {
        mAlgorithmCombo->addItems({"TSP_Dijkstra", "Dijkstra", "AStar"});
    } else if (planningMode == "Line Events") {
        mAlgorithmCombo->addItems({"Line_TSP_Dijkstra", "Line_TSP_AStar", "Line_Greedy"});
    } else if (planningMode == "Area Events") {
        mAlgorithmCombo->addItems({"Area_Spiral", "Area_Grid"});
    }
    
    updateAlgorithmDescription(mAlgorithmCombo->currentText());
    validateInput();
}

void GridPathPlannerDialog::initializeUI() {
    mMainLayout = new QVBoxLayout(this);
    

    mInputGroup = createInputGroup();
    mParametersGroup = createParametersGroup();
    mAlgorithmGroup = createAlgorithmGroup();
    mOutputGroup = createOutputGroup();
    mProgressGroup = createProgressGroup();
    mResultsGroup = createResultsGroup();
    

    mMainLayout->addWidget(mInputGroup);
    mMainLayout->addWidget(mParametersGroup);
    mMainLayout->addWidget(mAlgorithmGroup);
    mMainLayout->addWidget(mOutputGroup);
    mMainLayout->addWidget(mProgressGroup);
    mMainLayout->addWidget(mResultsGroup);
    

    createButtons();
}

QGroupBox* GridPathPlannerDialog::createInputGroup() {
    QGroupBox* group = new QGroupBox("input files");
    QGridLayout* layout = new QGridLayout(group);
    

    layout->addWidget(new QLabel("fishnet file:"), 0, 0);
    mFishnetEdit = new QLineEdit();
    layout->addWidget(mFishnetEdit, 0, 1);
    mFishnetBrowseBtn = new QPushButton("browse...");
    layout->addWidget(mFishnetBrowseBtn, 0, 2);
    

    layout->addWidget(new QLabel("risk points file:"), 1, 0);
    mRiskPointsEdit = new QLineEdit();
    layout->addWidget(mRiskPointsEdit, 1, 1);
    mRiskPointsBrowseBtn = new QPushButton("browse...");
    layout->addWidget(mRiskPointsBrowseBtn, 1, 2);
    

    layout->addWidget(new QLabel("risk lines file:"), 2, 0);
    mRiskLinesEdit = new QLineEdit();
    layout->addWidget(mRiskLinesEdit, 2, 1);
    mRiskLinesBrowseBtn = new QPushButton("browse...");
    layout->addWidget(mRiskLinesBrowseBtn, 2, 2);
    
    return group;
}

QGroupBox* GridPathPlannerDialog::createParametersGroup() {
    QGroupBox* group = new QGroupBox("planning parameters");
    QGridLayout* layout = new QGridLayout(group);
    

    layout->addWidget(new QLabel("start point X:"), 0, 0);
    mStartXSpin = new QDoubleSpinBox();
    mStartXSpin->setRange(-999999, 999999999);
    mStartXSpin->setDecimals(3);
    layout->addWidget(mStartXSpin, 0, 1);
    
    layout->addWidget(new QLabel("start point Y:"), 0, 2);
    mStartYSpin = new QDoubleSpinBox();
    mStartYSpin->setRange(-999999, 999999999);
    mStartYSpin->setDecimals(3);
    layout->addWidget(mStartYSpin, 0, 3);
    
    layout->addWidget(new QLabel("start point Z:"), 1, 0);
    mStartZSpin = new QDoubleSpinBox();
    mStartZSpin->setRange(0, 1000);
    mStartZSpin->setDecimals(1);
    layout->addWidget(mStartZSpin, 1, 1);
    

    layout->addWidget(new QLabel("max risk point distance(meters):"), 1, 2);
    mMaxDistanceSpin = new QDoubleSpinBox();
    mMaxDistanceSpin->setRange(1.0, 500.0);
    mMaxDistanceSpin->setDecimals(1);
    layout->addWidget(mMaxDistanceSpin, 1, 3);
    

    layout->addWidget(new QLabel("max risk line distance(meters):"), 2, 0);
    mMaxRiskLineDistanceSpin = new QDoubleSpinBox();
    mMaxRiskLineDistanceSpin->setRange(1.0, 500.0);
    mMaxRiskLineDistanceSpin->setDecimals(1);
    mMaxRiskLineDistanceSpin->setValue(50.0);
    layout->addWidget(mMaxRiskLineDistanceSpin, 2, 1);
    

    layout->addWidget(new QLabel("coverage threshold(%):"), 2, 2);
    mCoverageThresholdSpin = new QDoubleSpinBox();
    mCoverageThresholdSpin->setRange(0.1, 100.0);
    mCoverageThresholdSpin->setDecimals(1);
    mCoverageThresholdSpin->setValue(95.0);
    layout->addWidget(mCoverageThresholdSpin, 2, 3);
    
    layout->addWidget(new QLabel("max iterations:"), 3, 0);
    mMaxIterationsSpin = new QSpinBox();
    mMaxIterationsSpin->setRange(100, 10000);
    mMaxIterationsSpin->setValue(1000);
    layout->addWidget(mMaxIterationsSpin, 3, 1);
    
    layout->addWidget(new QLabel("grid spacing(meters):"), 3, 2);
    mGridSpacingSpin = new QDoubleSpinBox();
    mGridSpacingSpin->setRange(1.0, 100.0);
    mGridSpacingSpin->setDecimals(1);
    mGridSpacingSpin->setValue(10.0);
    layout->addWidget(mGridSpacingSpin, 3, 3);
    
    return group;
}

QGroupBox* GridPathPlannerDialog::createAlgorithmGroup() {
    QGroupBox* group = new QGroupBox("algorithm selection");
    QVBoxLayout* layout = new QVBoxLayout(group);

    QHBoxLayout* modeLayout = new QHBoxLayout();
    modeLayout->addWidget(new QLabel("planning mode:"));
    mPlanningModeCombo = new QComboBox();
    mPlanningModeCombo->addItems({"Point Events", "Line Events", "Area Events"});
    modeLayout->addWidget(mPlanningModeCombo);
    modeLayout->addStretch();
    layout->addLayout(modeLayout);


    QHBoxLayout* algoLayout = new QHBoxLayout();
    algoLayout->addWidget(new QLabel("algorithm:"));
    mAlgorithmCombo = new QComboBox();
    mAlgorithmCombo->addItems({"TSP_Dijkstra", "Dijkstra", "AStar"});
    algoLayout->addWidget(mAlgorithmCombo);
    algoLayout->addStretch();
    layout->addLayout(algoLayout);
    

    mAlgorithmDescLabel = new QLabel();
    mAlgorithmDescLabel->setWordWrap(true);
    mAlgorithmDescLabel->setStyleSheet("QLabel { color: #666; font-style: italic; }");
    layout->addWidget(mAlgorithmDescLabel);
    
    return group;
}

QGroupBox* GridPathPlannerDialog::createOutputGroup() {
    QGroupBox* group = new QGroupBox("output settings");
    QHBoxLayout* layout = new QHBoxLayout(group);
    
    layout->addWidget(new QLabel("output directory:"));
    mOutputEdit = new QLineEdit();
    layout->addWidget(mOutputEdit);
    mOutputBrowseBtn = new QPushButton("browse...");
    layout->addWidget(mOutputBrowseBtn);
    
    return group;
}

QGroupBox* GridPathPlannerDialog::createProgressGroup() {
    QGroupBox* group = new QGroupBox("planning progress");
    QVBoxLayout* layout = new QVBoxLayout(group);
    
    mProgressBar = new QProgressBar();
    layout->addWidget(mProgressBar);
    
    mProgressLabel = new QLabel("start to plan");
    layout->addWidget(mProgressLabel);
    
    return group;
}

QGroupBox* GridPathPlannerDialog::createResultsGroup() {
    QGroupBox* group = new QGroupBox("planning results");
    QVBoxLayout* layout = new QVBoxLayout(group);
    
    mResultsText = new QTextEdit();
    mResultsText->setMaximumHeight(200);
    mResultsText->setReadOnly(true);
    layout->addWidget(mResultsText);
    
    return group;
}

void GridPathPlannerDialog::createButtons() {
    mButtonLayout = new QHBoxLayout();
    
    mExecuteBtn = new QPushButton("execute planning");
    mExportBtn = new QPushButton("export results");
    mPreviewBtn = new QPushButton("preview results");
    mResetBtn = new QPushButton("reset");
    mCloseBtn = new QPushButton("close");
    
    mExportBtn->setEnabled(false);
    mPreviewBtn->setEnabled(false);
    
    mButtonLayout->addWidget(mExecuteBtn);
    mButtonLayout->addWidget(mExportBtn);
    mButtonLayout->addWidget(mPreviewBtn);
    mButtonLayout->addWidget(mResetBtn);
    mButtonLayout->addStretch();
    mButtonLayout->addWidget(mCloseBtn);
    
    mMainLayout->addLayout(mButtonLayout);
}

void GridPathPlannerDialog::connectSignals() {

    connect(mFishnetBrowseBtn, &QPushButton::clicked, this, &GridPathPlannerDialog::browseFishnetFile);
    connect(mRiskPointsBrowseBtn, &QPushButton::clicked, this, &GridPathPlannerDialog::browseRiskPointsFile);
    connect(mRiskLinesBrowseBtn, &QPushButton::clicked, this, &GridPathPlannerDialog::browseRiskLinesFile);
    connect(mOutputBrowseBtn, &QPushButton::clicked, this, &GridPathPlannerDialog::browseOutputDirectory);
    

    connect(mExecuteBtn, &QPushButton::clicked, this, &GridPathPlannerDialog::executePlanning);
    connect(mExportBtn, &QPushButton::clicked, this, &GridPathPlannerDialog::exportResults);
    connect(mPreviewBtn, &QPushButton::clicked, this, &GridPathPlannerDialog::previewResults);
    connect(mResetBtn, &QPushButton::clicked, this, &GridPathPlannerDialog::resetForm);
    connect(mCloseBtn, &QPushButton::clicked, this, &QDialog::accept);
    

    connect(mFishnetEdit, &QLineEdit::textChanged, this, &GridPathPlannerDialog::validateInput);
    connect(mRiskPointsEdit, &QLineEdit::textChanged, this, &GridPathPlannerDialog::validateInput);
    connect(mRiskLinesEdit, &QLineEdit::textChanged, this, &GridPathPlannerDialog::validateInput);
    

    connect(mAlgorithmCombo, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            this, &GridPathPlannerDialog::onAlgorithmChanged);
    

    connect(mPlanningModeCombo, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            this, &GridPathPlannerDialog::onPlanningModeChanged);
    

    connect(mPlanner, &GridPathPlanner::progressUpdated, this, &GridPathPlannerDialog::onProgressUpdated);
    connect(mPlanner, &GridPathPlanner::planningCompleted, this, &GridPathPlannerDialog::onPlanningCompleted);
    connect(mPlanner, &GridPathPlanner::linePlanningCompleted, this, &GridPathPlannerDialog::onLinePlanningCompleted);
    connect(mPlanner, &GridPathPlanner::areaPlanningCompleted, this, &GridPathPlannerDialog::onAreaPlanningCompleted);
    connect(mPlanner, &GridPathPlanner::planningFailed, this, &GridPathPlannerDialog::onPlanningFailed);
}

void GridPathPlannerDialog::setInitialValues() {
    GridPlanningParameters defaultParams;
    setPlanningParameters(defaultParams);
    updateAlgorithmDescription(mAlgorithmCombo->currentText());
}

void GridPathPlannerDialog::updateUIState(bool enabled) {
    mFishnetEdit->setEnabled(enabled);
    mFishnetBrowseBtn->setEnabled(enabled);
    mRiskPointsEdit->setEnabled(enabled);
    mRiskPointsBrowseBtn->setEnabled(enabled);
    mRiskLinesEdit->setEnabled(enabled);
    mRiskLinesBrowseBtn->setEnabled(enabled);
    mStartXSpin->setEnabled(enabled);
    mStartYSpin->setEnabled(enabled);
    mStartZSpin->setEnabled(enabled);
    mMaxDistanceSpin->setEnabled(enabled);
    mMaxRiskLineDistanceSpin->setEnabled(enabled);
    mCoverageThresholdSpin->setEnabled(enabled);
    mMaxIterationsSpin->setEnabled(enabled);
    mGridSpacingSpin->setEnabled(enabled);
    mPlanningModeCombo->setEnabled(enabled);
    mAlgorithmCombo->setEnabled(enabled);
    mOutputEdit->setEnabled(enabled);
    mOutputBrowseBtn->setEnabled(enabled);
    mResetBtn->setEnabled(enabled);
    
    if (enabled) {
        validateInput();
    } else {
        mExecuteBtn->setEnabled(false);
    }
}

bool GridPathPlannerDialog::validateFilePath(const QString& filePath) {
    return !filePath.isEmpty() && QFileInfo::exists(filePath);
}

void GridPathPlannerDialog::showError(const QString& message) {
    QMessageBox::critical(this, "error", message);
}

void GridPathPlannerDialog::showSuccess(const QString& message) {
    QMessageBox::information(this, "    成功", message);
}

void GridPathPlannerDialog::updateAlgorithmDescription(const QString& algorithm) {
    QString description;
    
    if (algorithm == "TSP_Dijkstra") {
        description = "TSP+Dijkstra";
    } else if (algorithm == "Dijkstra") {
        description = "Dijkstra";
    } else if (algorithm == "AStar") {
        description = "A*";
    } else if (algorithm == "Line_TSP_Dijkstra") {
        description = "Line TSP with Dijkstra";
    } else if (algorithm == "Line_TSP_AStar") {
        description = "Line TSP with A*";
    } else if (algorithm == "Line_Greedy") {
        description = "Line Greedy Algorithm";
    } else if (algorithm == "Area_Spiral") {
        description = "Area Spiral Algorithm";
    } else if (algorithm == "Area_Grid") {
        description = "Area Grid Algorithm";
    }
    
    mAlgorithmDescLabel->setText(description);
} 