
#ifndef RISK_EVENT_PLANNER_DIALOG_H
#define RISK_EVENT_PLANNER_DIALOG_H

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
#include "../core/RiskEventPlanner.h"


class RiskEventPlannerDialog : public QDialog {
    Q_OBJECT

public:
    explicit RiskEventPlannerDialog(QWidget *parent = nullptr);
    ~RiskEventPlannerDialog();


    PlanningParameters getPlanningParameters() const;


    void setPlanningParameters(const PlanningParameters& params);

    void resetToDefaults();

public slots:

    void onProgressUpdated(int percentage, const QString& message);


    void onPlanningCompleted(const PlanningResult& result);


    void onPlanningFailed(const QString& errorMessage);

signals:

    void startPlanning(const PlanningParameters& params);


    void exportPath(const QString& outputPath);


    void showResults(const PlanningResult& result);

private slots:

    void browseFlightZoneFile();

    void browseRiskEventFile();


    void browseOutputDirectory();

    void executePlanning();


    void exportResults();


    void previewResults();


    void resetForm();

    void validateInput();

private:
   
    QGroupBox* mInputGroup;           
    QGroupBox* mParametersGroup;     
    QGroupBox* mOutputGroup;          
    QGroupBox* mProgressGroup;         
    QGroupBox* mResultsGroup;          

   
    QLineEdit* mFlightZoneEdit;        
    QPushButton* mFlightZoneBrowseBtn;  
    QLineEdit* mRiskEventEdit;          
    QPushButton* mRiskEventBrowseBtn;    /


    QDoubleSpinBox* mStartXSpin;      
    QDoubleSpinBox* mStartYSpin;       
    QDoubleSpinBox* mStartZSpin;     
    QDoubleSpinBox* mSpacingSpin;      
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


    RiskEventPlanner* mPlanner;         
    PlanningResult mCurrentResult;      
    bool mPlanningInProgress;            


    void initializeUI();


    QGroupBox* createInputGroup();


    QGroupBox* createParametersGroup();


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
};

#endif // RISK_EVENT_PLANNER_DIALOG_H 