

#include "RiskEventPlannerDialog.h"
#include "../log/QgisDebug.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>

RiskEventPlannerDialog::RiskEventPlannerDialog(QWidget *parent)
    : QDialog(parent)
    , mPlanner(new RiskEventPlanner(this))
    , mPlanningInProgress(false) {
    
    setWindowTitle("campus risk event path planner");
    setMinimumSize(800, 600);
    resize(1000, 700);
    
    initializeUI();
    connectSignals();
    setInitialValues();
    updateUIState(true);
    
    logMessage("risk event path planner dialog initialized", Qgis::MessageLevel::Info);
}

RiskEventPlannerDialog::~RiskEventPlannerDialog() {
    
}

PlanningParameters RiskEventPlannerDialog::getPlanningParameters() const {
    PlanningParameters params;
    
    params.flightZoneShapefile = mFlightZoneEdit->text();
    params.riskEventShapefile = mRiskEventEdit->text();
    params.outputPath = mOutputEdit->text();
    params.startPoint = QVector3D(
        static_cast<float>(mStartXSpin->value()),
        static_cast<float>(mStartYSpin->value()),
        static_cast<float>(mStartZSpin->value())
    );
    params.triangulationSpacing = mSpacingSpin->value();
    params.optimizationAlgorithm = mAlgorithmCombo->currentText();
    
    return params;
}

void RiskEventPlannerDialog::setPlanningParameters(const PlanningParameters& params) {
    mFlightZoneEdit->setText(params.flightZoneShapefile);
    mRiskEventEdit->setText(params.riskEventShapefile);
    mOutputEdit->setText(params.outputPath);
    mStartXSpin->setValue(params.startPoint.x());
    mStartYSpin->setValue(params.startPoint.y());
    mStartZSpin->setValue(params.startPoint.z());
    mSpacingSpin->setValue(params.triangulationSpacing);
    
    int algorithmIndex = mAlgorithmCombo->findText(params.optimizationAlgorithm);
    if (algorithmIndex >= 0) {
        mAlgorithmCombo->setCurrentIndex(algorithmIndex);
    }
}

void RiskEventPlannerDialog::resetToDefaults() {
    PlanningParameters defaultParams;
    setPlanningParameters(defaultParams);
    mResultsText->clear();
    mProgressBar->setValue(0);
    mProgressLabel->setText("ready");
    mCurrentResult = PlanningResult();
}

void RiskEventPlannerDialog::onProgressUpdated(int percentage, const QString& message) {
    mProgressBar->setValue(percentage);
    mProgressLabel->setText(message);
    

    mResultsText->append(QString("[%1%] %2").arg(percentage).arg(message));
}

void RiskEventPlannerDialog::onPlanningCompleted(const PlanningResult& result) {
    mCurrentResult = result;
    mPlanningInProgress = false;
    updateUIState(true);
    
    if (result.success) {
        QString successMessage = QString("path planning completed!\n total path length: %1 meters\n risk event points count: %2")
                                .arg(result.totalPathLength, 0, 'f', 2)
                                .arg(result.riskEventCount);
        
        mResultsText->append("\n=== planning completed ===");
        mResultsText->append(successMessage);
        mResultsText->append(mPlanner->getStatistics());
        
        showSuccess("path planning completed!");
        

        mExportBtn->setEnabled(true);
        mPreviewBtn->setEnabled(true);
        
    } else {
        showError(QString("planning failed: %1").arg(result.errorMessage));
    }
}

void RiskEventPlannerDialog::onPlanningFailed(const QString& errorMessage) {
    mPlanningInProgress = false;
    updateUIState(true);
    showError(QString("planning failed: %1").arg(errorMessage));
    mResultsText->append(QString("\n=== planning failed ===\n%1").arg(errorMessage));
}

void RiskEventPlannerDialog::browseFlightZoneFile() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "select flight zone shapefile",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "Shapefile (*.shp);;all files (*.*)"
    );
    
    if (!fileName.isEmpty()) {
        mFlightZoneEdit->setText(fileName);
        validateInput();
    }
}

void RiskEventPlannerDialog::browseRiskEventFile() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "select risk event points shapefile",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "Shapefile (*.shp);;all files (*.*)"
    );
    
    if (!fileName.isEmpty()) {
        mRiskEventEdit->setText(fileName);
        validateInput();
    }
}

void RiskEventPlannerDialog::browseOutputDirectory() {
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

void RiskEventPlannerDialog::executePlanning() {
    if (mPlanningInProgress) {
        showError("planning is in progress, please wait for completion");
        return;
    }
    

    PlanningParameters params = getPlanningParameters();
    

    mResultsText->append(QString("DEBUG: UI Start Point: (%1, %2, %3)")
                        .arg(mStartXSpin->value())
                        .arg(mStartYSpin->value()) 
                        .arg(mStartZSpin->value()));
    mResultsText->append(QString("DEBUG: Params Start Point: (%1, %2, %3)")
                        .arg(params.startPoint.x())
                        .arg(params.startPoint.y())
                        .arg(params.startPoint.z()));
    
    auto validation = mPlanner->validateParameters(params);
    if (!validation.first) {
        showError(validation.second);
        return;
    }
    

    mPlanningInProgress = true;
    updateUIState(false);
    mResultsText->clear();
    mResultsText->append("start executing path planning...");
    

    mProgressBar->setValue(0);
    mProgressLabel->setText("preparing...");
    

    mPlanner->asyncExecutePlanning(params);
}

void RiskEventPlannerDialog::exportResults() {
    if (!mCurrentResult.success) {
        showError("no results to export");
        return;
    }
    
    QString outputPath = mOutputEdit->text();
    if (outputPath.isEmpty()) {
        showError("please select output directory");
        return;
    }
    

    QString pathShpFile = QDir(outputPath).filePath("optimal_path.shp");
    if (mPlanner->exportPathToShapefile(pathShpFile)) {
        showSuccess(QString("path exported to: %1").arg(pathShpFile));
        mResultsText->append(QString("path exported successfully: %1").arg(pathShpFile));
    } else {
        showError("path export failed");
    }
    

    QString triangulationShpFile = QDir(outputPath).filePath("triangulation.shp");
    if (mPlanner->exportTriangulationToShapefile(triangulationShpFile)) {
        mResultsText->append(QString("triangulation exported successfully: %1").arg(triangulationShpFile));
    }
}

void RiskEventPlannerDialog::previewResults() {
    if (!mCurrentResult.success) {
        showError("no results to preview");
        return;
    }
    

    emit showResults(mCurrentResult);
    

    QString statistics = mPlanner->getStatistics();
    mResultsText->append("\n=== detailed statistics ===");
    mResultsText->append(statistics);
}

void RiskEventPlannerDialog::resetForm() {
    if (mPlanningInProgress) {
        int ret = QMessageBox::question(this, "confirm reset", 
                                       "planning is in progress, confirm reset?",
                                       QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes) {
            return;
        }
    }
    
    resetToDefaults();
    updateUIState(true);
}

void RiskEventPlannerDialog::validateInput() {
    bool flightZoneValid = validateFilePath(mFlightZoneEdit->text());
    bool riskEventValid = validateFilePath(mRiskEventEdit->text());
    bool outputValid = !mOutputEdit->text().isEmpty();
    
    bool allValid = flightZoneValid && riskEventValid && outputValid;
    mExecuteBtn->setEnabled(allValid && !mPlanningInProgress);
}

void RiskEventPlannerDialog::initializeUI() {
    mMainLayout = new QVBoxLayout(this);
    

    mInputGroup = createInputGroup();
    mParametersGroup = createParametersGroup();
    mOutputGroup = createOutputGroup();
    mProgressGroup = createProgressGroup();
    mResultsGroup = createResultsGroup();
    

    mMainLayout->addWidget(mInputGroup);
    mMainLayout->addWidget(mParametersGroup);
    mMainLayout->addWidget(mOutputGroup);
    mMainLayout->addWidget(mProgressGroup);
    mMainLayout->addWidget(mResultsGroup);
    

    createButtons();
    mMainLayout->addLayout(mButtonLayout);
}

QGroupBox* RiskEventPlannerDialog::createInputGroup() {
    QGroupBox* group = new QGroupBox("input files");
    QGridLayout* layout = new QGridLayout(group);
    

    layout->addWidget(new QLabel("flight zone shapefile:"), 0, 0);
    mFlightZoneEdit = new QLineEdit();
    mFlightZoneEdit->setPlaceholderText("select flight zone shapefile...");
    layout->addWidget(mFlightZoneEdit, 0, 1);
    mFlightZoneBrowseBtn = new QPushButton("browse...");
    layout->addWidget(mFlightZoneBrowseBtn, 0, 2);
    

    layout->addWidget(new QLabel("risk event points shapefile:"), 1, 0);
    mRiskEventEdit = new QLineEdit();
    mRiskEventEdit->setPlaceholderText("select risk event points shapefile...");
    layout->addWidget(mRiskEventEdit, 1, 1);
    mRiskEventBrowseBtn = new QPushButton("browse...");
    layout->addWidget(mRiskEventBrowseBtn, 1, 2);
    
    return group;
}

QGroupBox* RiskEventPlannerDialog::createParametersGroup() {
    QGroupBox* group = new QGroupBox("planning parameters");
    QGridLayout* layout = new QGridLayout(group);
    

    layout->addWidget(new QLabel("start point coordinates:"), 0, 0);
    QHBoxLayout* startLayout = new QHBoxLayout();
    
    startLayout->addWidget(new QLabel("X:"));
    mStartXSpin = new QDoubleSpinBox();
    mStartXSpin->setRange(-999999, 999999);
    mStartXSpin->setDecimals(2);
    mStartXSpin->setValue(558856.516); 
    startLayout->addWidget(mStartXSpin);
    
    startLayout->addWidget(new QLabel("Y:"));
    mStartYSpin = new QDoubleSpinBox();
    mStartYSpin->setRange(-9999999, 9999999);
    mStartYSpin->setDecimals(2);
    mStartYSpin->setValue(3371566.848); 
    startLayout->addWidget(mStartYSpin);
    
    startLayout->addWidget(new QLabel("Z:"));
    mStartZSpin = new QDoubleSpinBox();
    mStartZSpin->setRange(0, 1000);
    mStartZSpin->setDecimals(2);
    mStartZSpin->setValue(50.0); 
    startLayout->addWidget(mStartZSpin);
    
    layout->addLayout(startLayout, 0, 1, 1, 2);
    

    layout->addWidget(new QLabel("triangulation spacing(meters):"), 1, 0);
    mSpacingSpin = new QDoubleSpinBox();
    mSpacingSpin->setRange(1.0, 1000.0);
    mSpacingSpin->setDecimals(1);
    mSpacingSpin->setValue(10.0);
    layout->addWidget(mSpacingSpin, 1, 1);
    
                        
    layout->addWidget(new QLabel("optimization algorithm:"), 2, 0);
    mAlgorithmCombo = new QComboBox();
    mAlgorithmCombo->addItems({"NearestNeighbor", "GeneticAlgorithm"});
    layout->addWidget(mAlgorithmCombo, 2, 1);
    
    return group;
}

QGroupBox* RiskEventPlannerDialog::createOutputGroup() {
    QGroupBox* group = new QGroupBox("output settings");
    QGridLayout* layout = new QGridLayout(group);
    
    layout->addWidget(new QLabel("output directory:"), 0, 0);
    mOutputEdit = new QLineEdit();
    mOutputEdit->setPlaceholderText("select output directory...");
    layout->addWidget(mOutputEdit, 0, 1);
    mOutputBrowseBtn = new QPushButton("browse...");
    layout->addWidget(mOutputBrowseBtn, 0, 2);
    
    return group;
}

QGroupBox* RiskEventPlannerDialog::createProgressGroup() {
    QGroupBox* group = new QGroupBox("execution progress");
    QVBoxLayout* layout = new QVBoxLayout(group);
    
    mProgressLabel = new QLabel("ready");
    layout->addWidget(mProgressLabel);
    
    mProgressBar = new QProgressBar();
    mProgressBar->setRange(0, 100);
    mProgressBar->setValue(0);
    layout->addWidget(mProgressBar);
    
    return group;
}

QGroupBox* RiskEventPlannerDialog::createResultsGroup() {
    QGroupBox* group = new QGroupBox("planning results");
    QVBoxLayout* layout = new QVBoxLayout(group);
    
    mResultsText = new QTextEdit();
    mResultsText->setMinimumHeight(200);
    mResultsText->setReadOnly(true);
    layout->addWidget(mResultsText);
    
    return group;
}

void RiskEventPlannerDialog::createButtons() {
    mButtonLayout = new QHBoxLayout();
    
    mExecuteBtn = new QPushButton("execute planning");
    mExecuteBtn->setEnabled(false);
    mButtonLayout->addWidget(mExecuteBtn);
    
    mExportBtn = new QPushButton("export results");
    mExportBtn->setEnabled(false);
    mButtonLayout->addWidget(mExportBtn);
    
    mPreviewBtn = new QPushButton("preview results");
    mPreviewBtn->setEnabled(false);
    mButtonLayout->addWidget(mPreviewBtn);
    
    mResetBtn = new QPushButton("reset");
    mButtonLayout->addWidget(mResetBtn);
    
    mButtonLayout->addStretch();
    
    mCloseBtn = new QPushButton("close");
    mButtonLayout->addWidget(mCloseBtn);
}

void RiskEventPlannerDialog::connectSignals() {

    connect(mFlightZoneBrowseBtn, &QPushButton::clicked, this, &RiskEventPlannerDialog::browseFlightZoneFile);
    connect(mRiskEventBrowseBtn, &QPushButton::clicked, this, &RiskEventPlannerDialog::browseRiskEventFile);
    connect(mOutputBrowseBtn, &QPushButton::clicked, this, &RiskEventPlannerDialog::browseOutputDirectory);
    

    connect(mFlightZoneEdit, &QLineEdit::textChanged, this, &RiskEventPlannerDialog::validateInput);
    connect(mRiskEventEdit, &QLineEdit::textChanged, this, &RiskEventPlannerDialog::validateInput);
    connect(mOutputEdit, &QLineEdit::textChanged, this, &RiskEventPlannerDialog::validateInput);
    

    connect(mExecuteBtn, &QPushButton::clicked, this, &RiskEventPlannerDialog::executePlanning);
    connect(mExportBtn, &QPushButton::clicked, this, &RiskEventPlannerDialog::exportResults);
    connect(mPreviewBtn, &QPushButton::clicked, this, &RiskEventPlannerDialog::previewResults);
    connect(mResetBtn, &QPushButton::clicked, this, &RiskEventPlannerDialog::resetForm);
    connect(mCloseBtn, &QPushButton::clicked, this, &QDialog::accept);
    

    connect(mPlanner, &RiskEventPlanner::progressUpdated, this, &RiskEventPlannerDialog::onProgressUpdated);
    connect(mPlanner, &RiskEventPlanner::planningCompleted, this, &RiskEventPlannerDialog::onPlanningCompleted);
    connect(mPlanner, &RiskEventPlanner::planningFailed, this, &RiskEventPlannerDialog::onPlanningFailed);
}

void RiskEventPlannerDialog::setInitialValues() {

    QString defaultOutput = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/RiskEventPlanning";
    mOutputEdit->setText(defaultOutput);
    

    QString appDir = QCoreApplication::applicationDirPath();
    QString resourcesDir = QDir(appDir).absoluteFilePath("../resources/map");
    

    QString flightZoneFile = QDir(resourcesDir).absoluteFilePath("溺水风险区.shp");
    if (QFileInfo::exists(flightZoneFile)) {
        mFlightZoneEdit->setText(flightZoneFile);
    }
    

    QString riskEventFile = QDir(resourcesDir).absoluteFilePath("人群密集点（上课时段）.shp");
    if (QFileInfo::exists(riskEventFile)) {
        mRiskEventEdit->setText(riskEventFile);
    }
    

    mStartXSpin->setValue(558856.516); 
    mStartYSpin->setValue(3371566.848); 
    mStartZSpin->setValue(9.0);        
    

    logMessage(QString("default flight zone file: %1").arg(mFlightZoneEdit->text()), Qgis::MessageLevel::Info);
    logMessage(QString("default risk event file: %1").arg(mRiskEventEdit->text()), Qgis::MessageLevel::Info);
    logMessage(QString("start point set to data center: (%1, %2)").arg(mStartXSpin->value()).arg(mStartYSpin->value()), Qgis::MessageLevel::Info);
}

void RiskEventPlannerDialog::updateUIState(bool enabled) {
    mFlightZoneEdit->setEnabled(enabled);
    mFlightZoneBrowseBtn->setEnabled(enabled);
    mRiskEventEdit->setEnabled(enabled);
    mRiskEventBrowseBtn->setEnabled(enabled);
    mOutputEdit->setEnabled(enabled);
    mOutputBrowseBtn->setEnabled(enabled);
    
    mStartXSpin->setEnabled(enabled);
    mStartYSpin->setEnabled(enabled);
    mStartZSpin->setEnabled(enabled);
    mSpacingSpin->setEnabled(enabled);
    mAlgorithmCombo->setEnabled(enabled);
    
    if (enabled) {
            validateInput(); 
    } else {
        mExecuteBtn->setEnabled(false);
    }
    
    mResetBtn->setEnabled(enabled);
}

bool RiskEventPlannerDialog::validateFilePath(const QString& filePath) {
    return !filePath.isEmpty() && QFileInfo::exists(filePath);
}

void RiskEventPlannerDialog::showError(const QString& message) {
    QMessageBox::critical(this, "err", message);
    logMessage(message, Qgis::MessageLevel::Critical);
}

void RiskEventPlannerDialog::showSuccess(const QString& message) {
    QMessageBox::information(this, "success", message);
    logMessage(message, Qgis::MessageLevel::Success);
} 
