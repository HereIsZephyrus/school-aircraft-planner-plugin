
#ifndef GRID_PATH_PLANNER_DIALOG_H
#define GRID_PATH_PLANNER_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QTextEdit>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QVector3D>
#include "../core/GridPathPlanner.h"


class GridPathPlannerDialog : public QDialog {
    Q_OBJECT

public:
    explicit GridPathPlannerDialog(QWidget *parent = nullptr);
    ~GridPathPlannerDialog();


    GridPlanningParameters getPlanningParameters() const;


    LinePlanningParameters getLinePlanningParameters() const;


    AreaPlanningParameters getAreaPlanningParameters() const;


    void setPlanningParameters(const GridPlanningParameters& params);


    void resetToDefaults();

public slots:

    void onProgressUpdated(int percentage, const QString& message);


    void onPlanningCompleted(const GridPlanningResult& result);


    void onLinePlanningCompleted(const LinePlanningResult& result);


    void onAreaPlanningCompleted(const AreaPlanningResult& result);


    void onPlanningFailed(const QString& errorMessage);

signals:

    void startPlanning(const GridPlanningParameters& params);


    void exportPath(const GridPlanningResult& result, const QString& outputPath);

    void showResults(const GridPlanningResult& result);


    void showAreaResults(const AreaPlanningResult& result);

private slots:

    void browseFishnetFile();


    void browseRiskPointsFile();

    void browseRiskLinesFile();


    void browseOutputDirectory();

    void executePlanning();


    void exportResults();


    void previewResults();

    void resetForm();

    void validateInput();

    void onAlgorithmChanged();

 
    void onPlanningModeChanged();

private:

    QGroupBox* mInputGroup;            
    QGroupBox* mParametersGroup;       
    QGroupBox* mAlgorithmGroup;         
    QGroupBox* mOutputGroup;            
    QGroupBox* mProgressGroup;          
    QGroupBox* mResultsGroup;           


    QLineEdit* mFishnetEdit;            
    QPushButton* mFishnetBrowseBtn;     
    QLineEdit* mRiskPointsEdit;         
    QPushButton* mRiskPointsBrowseBtn;  
    QLineEdit* mRiskLinesEdit;          
    QPushButton* mRiskLinesBrowseBtn;   


    QDoubleSpinBox* mStartXSpin;        
    QDoubleSpinBox* mStartYSpin;        
    QDoubleSpinBox* mStartZSpin;        
    QDoubleSpinBox* mMaxDistanceSpin;   
    QDoubleSpinBox* mMaxRiskLineDistanceSpin; 
    QDoubleSpinBox* mCoverageThresholdSpin; 
    QSpinBox* mMaxIterationsSpin;       
    QDoubleSpinBox* mGridSpacingSpin;   

    QComboBox* mAlgorithmCombo;         

    QLineEdit* mOutputEdit;             
    QPushButton* mOutputBrowseBtn;      

    QProgressBar* mProgressBar;         
    QLabel* mProgressLabel;             

    QTextEdit* mResultsText;            

    QPushButton* mExecuteBtn;           
    QPushButton* mExportBtn;            
    QPushButton* mPreviewBtn;           
    QPushButton* mResetBtn;             
    QPushButton* mCloseBtn;             

    QVBoxLayout* mMainLayout;           
    QHBoxLayout* mButtonLayout;         

    GridPathPlanner* mPlanner;          
    bool mPlanningInProgress;           
    GridPlanningResult mCurrentResult;  
    LinePlanningResult mCurrentLineResult; 
    AreaPlanningResult mCurrentAreaResult; 

    void initializeUI();

    QGroupBox* createInputGroup();

    QGroupBox* createParametersGroup();

    QGroupBox* createAlgorithmGroup();

    QGroupBox* createOutputGroup();

    QGroupBox* createProgressGroup();

    QGroupBox* createResultsGroup();

    void createButtons();

    void connectSignals();

    void setInitialValues();

    void updateUIState(bool enabled);

    bool validateFilePath(const QString& filePath);

    void showError(const QString& message);

    void showSuccess(const QString& message);


    void updateAlgorithmDescription(const QString& algorithm);
};

#endif // GRID_PATH_PLANNER_DIALOG_H 