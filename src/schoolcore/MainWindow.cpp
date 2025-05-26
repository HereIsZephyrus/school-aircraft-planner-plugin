#include "MainWindow.h"
#include "MyOpenGLWidget.h"
#include "StyleManager.h"
#include "WorkspaceState.h"
#include "camera.h"
#include "qgsmessagelog.h"
#include <memory>
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDirIterator>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QQueue>
#include <QRect>
#include <QScreen>
#include <QTextEdit>
#include <QToolBar>
#include <QVBoxLayout>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <QFile>
#include <QDateTime>
#include <QDir>
#include <QScrollArea>

void MainWindow::init3DWidget(){
    mpRoutePlanner = std::make_unique<RoutePlanner>(this);
    mpOpenGLWidget = std::make_unique<MyOpenGLWidget>(this);

    //connect(mpOpenGLWidget.get(), &MyOpenGLWidget::glInitialized, this, &MainWindow::switchTo3D);

    logMessage("canvas initialized", Qgis::MessageLevel::Success);

    // pass RoutePlanner instance to OpenGLWidget
    //mpOpenGLWidget->setRoutePlanner(mpRoutePlanner.get());
}
void MainWindow::init2DWidget(){
    // create QLabel to display local image
    mpImageLabel = new QLabel(this);
    QPixmap mapImage(":/schoolcore/map/capture.png"); // use resource path to load image
    if (mapImage.isNull()) {
        logMessage("failed to load local map image", Qgis::MessageLevel::Critical);
        return;
    }
    mpImageLabel->setPixmap(mapImage);
    mpImageLabel->setScaledContents(true); // let image adapt to label size, keep ratio
    mpImageLabel->setSizePolicy(QSizePolicy::Ignored,
                                    QSizePolicy::Ignored); // set size policy
    logMessage("create QLabel to display local map image", Qgis::MessageLevel::Success);
}
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    logMessage("Application started", Qgis::MessageLevel::Info);
    initWindowStatus();
    ws::initializeWorkspaceState();
    ws::initializeWorkspaceState();
    logMessage("MainWindow constructor called", Qgis::MessageLevel::Success);

    init3DWidget();
    init2DWidget();
    
    init3DWidget();
    init2DWidget();
    
    logMessage("function class initialized", Qgis::MessageLevel::Success);
    
    createMenu();
    createMainWindow();
    connect(mpRoutePlanner.get(), &RoutePlanner::dataUpdated, mpOpenGLWidget.get(),
            QOverload<>::of(&QOpenGLWidget::update));
    logMessage("connect route planner signal to update mapcanvas", Qgis::MessageLevel::Success);
    createJoyDockWidgets();
    createJoyDockWidgets();
    logMessage("create dock widgets", Qgis::MessageLevel::Success);
}

MainWindow::~MainWindow() {
    logMessage("MainWindow destroyed", Qgis::MessageLevel::Info);
}

static QSize setWindowSize(QRect screenGeometry, int maxWidth, int maxHeight, int minWidth, int minHeight){
    // calc current screen size
    double width_d = screenGeometry.width() * 0.8;
    double height_d = screenGeometry.height() * 0.8;

    int width = qMin(maxWidth, static_cast<int>(width_d));
    int height = qMin(maxHeight, static_cast<int>(height_d));
    width = qMax(minWidth, width);
    height = qMax(minHeight, height);
    return QSize(width, height);
}
void MainWindow::initWindowStatus(){
    StyleManager::initializeStyle(); // initialize global style
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    QSize windowSize = setWindowSize(screenGeometry, 2700, 1500, 800, 600);
    setMinimumSize(windowSize);
    int x = (screenGeometry.width() - windowSize.width()) / 2, y = (screenGeometry.height() - windowSize.height()) / 2;
    move(x, y);

    setWindowFlags(Qt::Window);
    setWindowTitle("3D Flight Simulation");
    setWindowTitle("3D Flight Simulation");
}

void MainWindow::Unrealized() {}

void MainWindow::createMenu() {
    logMessage("create menu bar", Qgis::MessageLevel::Info);
    mpMenuBar = new QMenuBar(this);

    // ================ Project menu ================
    QMenu *pProjectMenu = mpMenuBar->addMenu(tr("Project"));
    QAction* loadAction = pProjectMenu->addAction(tr("load 3D file"));
    connect(loadAction, &QAction::triggered, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(this, tr("Open OBJ File"), "", tr("OBJ Files (*.obj)"));
        if (filePath.isEmpty())
            logMessage("no file selected", Qgis::MessageLevel::Critical);
        else
            loadModel(filePath);
    });
    logMessage("create project menu", Qgis::MessageLevel::Success);

    // ================ View menu ================
    QMenu *pViewMenu = mpMenuBar->addMenu(tr("View"));
    pViewMenu->addAction(tr("3D View"), this, &MainWindow::switchTo3D);
    pViewMenu->addAction(tr("2D View"), this, &MainWindow::switchTo2D);
    pViewMenu->addAction(tr("Reset View"), this, &MainWindow::resetView);
    logMessage("create view menu", Qgis::MessageLevel::Success);

    // ================ Setting menu ================
    QMenu *pSettingMenu = mpMenuBar->addMenu(tr("Setting"));
    // parameter setting directly associated with dialog control
    QAction *pFlightParamsAction =
        pSettingMenu->addAction(tr("Flight Parameters")); // aircraft parameters
    QAction *pEnvironmentalParamsAction =
        pSettingMenu->addAction(tr("Environmental parameters")); // environmental parameters

    connect(pFlightParamsAction, &QAction::triggered, this,
            &MainWindow::showFlightParamsDialog);
    logMessage("connect flight parameters action to show flight parameters dialog", Qgis::MessageLevel::Info);
    connect(pEnvironmentalParamsAction, &QAction::triggered, this,
            &MainWindow::showEnvironmentalParamsDialog);
    logMessage("connect environmental parameters action to show environmental parameters dialog", Qgis::MessageLevel::Info);
    logMessage("create setting menu", Qgis::MessageLevel::Success);

    // ================ Route Planning menu ================
    QMenu *pRouteMenu = mpMenuBar->addMenu(tr("Route Planning"));
    QAction *pCreateRouteAction = pRouteMenu->addAction(tr("Create route"));
    if (mpRoutePlanner) {
        connect(pCreateRouteAction, &QAction::triggered, mpRoutePlanner.get(),
                &RoutePlanner::enterRoutePlanningMode);
    } else {
        logMessage("RoutePlanner not initialized", Qgis::MessageLevel::Critical);
    }
    logMessage("create route planning menu", Qgis::MessageLevel::Success);

    //  ================ Simulation menu ================
    QMenu *pSimulationMenu = mpMenuBar->addMenu(tr("Simulation"));
    if (mpOpenGLWidget) {
        QAction* startAction = pSimulationMenu->addAction(tr("Start Simulation"));
        QAction* pauseAction = pSimulationMenu->addAction(tr("Pause Simulation"));
        QAction* resumeAction = pSimulationMenu->addAction(tr("Resume Simulation"));
        QAction* returnAction = pSimulationMenu->addAction(tr("Return Home"));
        QAction* stopAction = pSimulationMenu->addAction(tr("Stop Simulation"));
/*
        connect(startAction, &QAction::triggered, mpOpenGLWidget.get(),
                &MyOpenGLWidget::startSimulation);
        connect(pauseAction, &QAction::triggered, mpOpenGLWidget.get(),
                &MyOpenGLWidget::pauseSimulation);
        connect(resumeAction, &QAction::triggered, mpOpenGLWidget.get(),
                &MyOpenGLWidget::resumeSimulation);
        connect(returnAction, &QAction::triggered, mpOpenGLWidget.get(),
                &MyOpenGLWidget::returnToHome);
        connect(stopAction, &QAction::triggered, mpOpenGLWidget.get(),
                &MyOpenGLWidget::stopSimulation);
*/
    } else {
        logMessage("OpenGLWidget not initialized", Qgis::MessageLevel::Critical);
    }
    logMessage("create simulation menu", Qgis::MessageLevel::Success);

    //  ================ Help menu ================
    QMenu *pHelpMenu = mpMenuBar->addMenu(tr("Help"));
    pHelpMenu->addAction(tr("User Manual"), this, &MainWindow::showUserManual);

    setMenuBar(mpMenuBar);
    logMessage("create menu bar", Qgis::MessageLevel::Success);
}

void MainWindow::createLeftDockWidget() {
    // initialize left dock widget
    QDockWidget *pLeftDockWidget = new QDockWidget(tr("Control Panel"), this);
    pLeftDockWidget->setObjectName("pLeftDockWidget");
    pLeftDockWidget->setObjectName("pLeftDockWidget");
    pLeftDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea);
    pLeftDockWidget->setFeatures(QDockWidget::DockWidgetMovable |
                                QDockWidget::DockWidgetFloatable);
    pLeftDockWidget->setMinimumWidth(200);

    QScrollArea *scrollArea = new QScrollArea(pLeftDockWidget);
    scrollArea->setObjectName("scrollArea");
    scrollArea->setObjectName("scrollArea");
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    QWidget *pDockContent = new QWidget(pLeftDockWidget);
    pDockContent->setObjectName("pDockContent");
    pDockContent->setObjectName("pDockContent");
    pDockContent->setMinimumWidth(175);
    QVBoxLayout *pMainLayout = new QVBoxLayout(pDockContent);
    pMainLayout->setContentsMargins(10, 10, 10, 10);
    pMainLayout->setSpacing(10);

    // ===== view switch group =====
    QGroupBox *pViewGroup = new QGroupBox("View Switch", pDockContent);
    pViewGroup->setObjectName("pViewGroup");
    pViewGroup->setObjectName("pViewGroup");
    QVBoxLayout *pViewLayout = new QVBoxLayout(pViewGroup);
    pViewLayout->setObjectName("pViewLayout");
    pViewLayout->setObjectName("pViewLayout");
    pViewLayout->addWidget(mpBtnReset = new QPushButton("Reset", pViewGroup));
    pViewLayout->addWidget(mpBtnSwitchTo3D = new QPushButton("3D View", pViewGroup));
    pViewLayout->addWidget(mpBtnSwitchTo2D = new QPushButton("2D Map", pViewGroup));
    pMainLayout->addWidget(pViewGroup);
    logMessage("create view switch group", Qgis::MessageLevel::Success);

    // ===== route planning group =====
    QGroupBox *pRouteGroup = new QGroupBox("Route Planning", pDockContent);
    pRouteGroup->setObjectName("pRouteGroup");
    pRouteGroup->setObjectName("pRouteGroup");
    QFormLayout *pRouteLayout = new QFormLayout(pRouteGroup);
    pRouteLayout->setObjectName("pRouteLayout");
    pRouteLayout->setObjectName("pRouteLayout");
    logMessage("create route planning group", Qgis::MessageLevel::Success);

    // add base height control widget to route planning group
    QDoubleSpinBox *pBaseHeightSpin = new QDoubleSpinBox(pRouteGroup);
    pBaseHeightSpin->setObjectName("pBaseHeightSpin");
    pBaseHeightSpin->setObjectName("pBaseHeightSpin");
    pBaseHeightSpin->setRange(-1000.0, 1000.0);
    pBaseHeightSpin->setValue(0.0);
    pBaseHeightSpin->setSuffix(" m");
    pRouteLayout->addRow("Base Height:", pBaseHeightSpin);

    // set height selection box
    QDoubleSpinBox *pHeightSpin = new QDoubleSpinBox(pRouteGroup);
    pHeightSpin->setObjectName("pHeightSpin");
    pHeightSpin->setObjectName("pHeightSpin");
    pHeightSpin->setRange(1.0, 300.0);
    pHeightSpin->setValue(75.0);
    pHeightSpin->setSuffix(" m");
    pRouteLayout->addRow("Set Altitude:", pHeightSpin);

    // set height selection box
    QDoubleSpinBox *pWidthSpin = new QDoubleSpinBox(pRouteGroup);
    pWidthSpin->setObjectName("pWidthSpin");
    pWidthSpin->setObjectName("pWidthSpin");
    pWidthSpin->setRange(1.0, 300.0);
    pWidthSpin->setValue(10.0);
    pWidthSpin->setSuffix(" m");
    pRouteLayout->addRow("Flight Path Width:", pWidthSpin);

    // set operation buttons: create route, add, move, clear control points, generate route
    QPushButton *pBtnCreateRoute =
        new QPushButton("Create Route", pRouteGroup); // start creating route
    pBtnCreateRoute->setObjectName("pBtnCreateRoute");
    pBtnCreateRoute->setObjectName("pBtnCreateRoute");
    QPushButton *pBtnSetHome = new QPushButton("Set Home Point", pRouteGroup);
    pBtnSetHome->setObjectName("pBtnSetHome");
    pBtnSetHome->setObjectName("pBtnSetHome");
    QPushButton *pBtnAddControlPoint =
        new QPushButton("Add Control Point", pRouteGroup); // add control point button
    pBtnAddControlPoint->setObjectName("pBtnAddControlPoint");
    pBtnAddControlPoint->setObjectName("pBtnAddControlPoint");
    QPushButton *pBtnEditPoint =
        new QPushButton("Edit Points", pRouteGroup); // bring up the toolbar, select the point, and move, delete point function
    pBtnEditPoint->setObjectName("pBtnEditPoint");
    pBtnEditPoint->setObjectName("pBtnEditPoint");
    QPushButton *pBtnGenerate = new QPushButton("Generate Route", pRouteGroup); // generate route
    pBtnGenerate->setObjectName("pBtnGenerate");
    pBtnGenerate->setObjectName("pBtnGenerate");

    QVBoxLayout *pBtnColumn =
        new QVBoxLayout(pRouteGroup); // use QVBoxLayout instead of QHBoxLayout
    pBtnColumn->setObjectName("pBtnColumn");
    pBtnColumn->setContentsMargins(0, 0, 0, 0);
    pBtnColumn->setObjectName("pBtnColumn");
    pBtnColumn->setContentsMargins(0, 0, 0, 0);
    pBtnColumn->addWidget(pBtnCreateRoute);
    pBtnColumn->addWidget(pBtnSetHome);
    pBtnColumn->addWidget(pBtnAddControlPoint); // add "add control point" button
    pBtnColumn->addWidget(pBtnEditPoint);
    pBtnColumn->addWidget(pBtnGenerate);
    pRouteLayout->addRow(pBtnColumn);
    pMainLayout->addWidget(pRouteGroup);

    logMessage("create route planning group", Qgis::MessageLevel::Success);

    // ===== flight simulation group =====
    QGroupBox *pSimGroup = new QGroupBox("Flight Simulation", pDockContent);
    pSimGroup->setObjectName("pSimGroup");
    pSimGroup->setObjectName("pSimGroup");
    QFormLayout *pSimLayout = new QFormLayout(pSimGroup);
    pSimLayout->setObjectName("pSimLayout");
    pSimLayout->setObjectName("pSimLayout");
    logMessage("create flight simulation group", Qgis::MessageLevel::Success);

    QDoubleSpinBox *pSpeedSpin = new QDoubleSpinBox(pSimGroup);
    pSpeedSpin->setObjectName("pSpeedSpin");
    pSpeedSpin->setRange(ws::FlightManager::minFlightSpeed, ws::FlightManager::maxFlightSpeed);
    pSpeedSpin->setValue(ws::FlightManager::getInstance().getFlightSpeed());
    pSpeedSpin->setObjectName("pSpeedSpin");
    pSpeedSpin->setRange(ws::FlightManager::minFlightSpeed, ws::FlightManager::maxFlightSpeed);
    pSpeedSpin->setValue(ws::FlightManager::getInstance().getFlightSpeed());
    pSpeedSpin->setSuffix(" m/s");
    pSimLayout->addRow("Flight Speed:", pSpeedSpin);
    logMessage("create flight speed spin box", Qgis::MessageLevel::Success);

    QPushButton *pBtnStart = new QPushButton("Start", pSimGroup);
    pBtnStart->setObjectName("pBtnStart");
    pBtnStart->setObjectName("pBtnStart");
    QPushButton *pBtnPause = new QPushButton("Pause", pSimGroup);
    pBtnPause->setObjectName("pBtnPause");
    pBtnPause->setObjectName("pBtnPause");
    QPushButton *pBtnResume = new QPushButton("Resume", pSimGroup);
    pBtnResume->setObjectName("pBtnResume");
    pBtnResume->setObjectName("pBtnResume");
    QPushButton *pBtnReturn = new QPushButton("Return Home", pSimGroup);
    pBtnReturn->setObjectName("pBtnReturn");
    pBtnReturn->setObjectName("pBtnReturn");
    QPushButton *pBtnStop = new QPushButton("Stop Simulation", pSimGroup);
    pBtnStop->setObjectName("pBtnStop");
    pBtnStop->setObjectName("pBtnStop");
    QHBoxLayout *pControlRow1 = new QHBoxLayout(pSimGroup);
    pControlRow1->setObjectName("pControlRow1");
    pControlRow1->setObjectName("pControlRow1");
    pControlRow1->addWidget(pBtnStart);
    pControlRow1->addWidget(pBtnPause);
    QHBoxLayout *pControlRow2 = new QHBoxLayout(pSimGroup);
    pControlRow2->setObjectName("pControlRow2");
    pControlRow2->setObjectName("pControlRow2");
    pControlRow2->addWidget(pBtnResume);
    pControlRow2->addWidget(pBtnReturn);
    QHBoxLayout *pControlRow3 = new QHBoxLayout(pSimGroup);
    pControlRow3->setObjectName("pControlRow3");
    pControlRow3->setObjectName("pControlRow3");
    pControlRow3->addWidget(pBtnStop);
    logMessage("create flight simulation group", Qgis::MessageLevel::Success);

    pSimLayout->addRow(pControlRow1);
    pSimLayout->addRow(pControlRow2);
    pSimLayout->addRow(pControlRow3);
    pMainLayout->addWidget(pSimGroup);

    pMainLayout->addStretch();

    scrollArea->setWidget(pDockContent);
    pLeftDockWidget->setWidget(scrollArea);
    addDockWidget(Qt::LeftDockWidgetArea, pLeftDockWidget);

    // ===== flight parameters query group =====
    QGroupBox *pFlightParamsGroup =
        new QGroupBox("Flight Parameters", pDockContent);
    pFlightParamsGroup->setObjectName("pFlightParamsGroup");
    pFlightParamsGroup->setObjectName("pFlightParamsGroup");
    QVBoxLayout *pFlightParamsLayout = new QVBoxLayout(pFlightParamsGroup);
    pFlightParamsLayout->setObjectName("pFlightParamsLayout");
    pFlightParamsLayout->setObjectName("pFlightParamsLayout");
    QPushButton *pBtnQueryParams = new QPushButton("Query Parameters", pFlightParamsGroup);
    pBtnQueryParams->setObjectName("pBtnQueryParams");
    pBtnQueryParams->setObjectName("pBtnQueryParams");
    m_pFlightParamsDisplay = new QLabel("No flight data available", pFlightParamsGroup);
    m_pFlightParamsDisplay->setWordWrap(true);
    m_pFlightParamsDisplay->setFrameStyle(QFrame::Box);
    pFlightParamsLayout->addWidget(pBtnQueryParams);
    pFlightParamsLayout->addWidget(m_pFlightParamsDisplay);
    pMainLayout->addWidget(pFlightParamsGroup);
    logMessage("create flight parameters query group", Qgis::MessageLevel::Success);

    // ===== basic data group =====
    QGroupBox *pBasicDataGroup =
        new QGroupBox("Environmental Data", pDockContent);
    pBasicDataGroup->setObjectName("pBasicDataGroup");
    pBasicDataGroup->setObjectName("pBasicDataGroup");
    QFormLayout *pBasicDataLayout = new QFormLayout(pBasicDataGroup);
    pBasicDataLayout->setObjectName("pBasicDataLayout");
    pBasicDataLayout->setObjectName("pBasicDataLayout");
    m_pWeatherLabel = new QLabel("Weather: -", pBasicDataGroup);
    m_pTemperatureLabel = new QLabel("Temperature: -", pBasicDataGroup);
    m_pPressureLabel = new QLabel("Pressure: -", pBasicDataGroup);
    QPushButton *pBtnRefreshData = new QPushButton("Refresh Data", pBasicDataGroup);
    pBtnRefreshData->setObjectName("pBtnRefreshData");
    pBtnRefreshData->setObjectName("pBtnRefreshData");
    pBasicDataLayout->addRow("Weather:", m_pWeatherLabel);
    pBasicDataLayout->addRow("Temperature:", m_pTemperatureLabel);
    pBasicDataLayout->addRow("Pressure:", m_pPressureLabel);
    pBasicDataLayout->addRow(pBtnRefreshData);
    pMainLayout->addWidget(pBasicDataGroup);
    logMessage("create basic data group", Qgis::MessageLevel::Success);
}
void MainWindow::createRightDockWidget() {
    // initialize right dock widget
    QDockWidget *pRightDockWidget = new QDockWidget(tr("Property Panel"), this);
    pRightDockWidget->setObjectName("pRightDockWidget");
    pRightDockWidget->setObjectName("pRightDockWidget");
    pRightDockWidget->setAllowedAreas(Qt::RightDockWidgetArea);
    pRightDockWidget->setFeatures(QDockWidget::DockWidgetMovable |
                                QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, pRightDockWidget);
    logMessage("create right dock widget", Qgis::MessageLevel::Success);
    // create file tree
    mpFileTreeWidget = new QTreeWidget(pRightDockWidget);
    mpFileTreeWidget->setObjectName("mpFileTreeWidget");
    mpFileTreeWidget->setObjectName("mpFileTreeWidget");
    mpFileTreeWidget->setHeaderLabel(tr("File List"));
    logMessage("create file tree", Qgis::MessageLevel::Success);
    // set right-click menu
    mpFileTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mpFileTreeWidget, &QTreeWidget::customContextMenuRequested, this,
            &MainWindow::Unrealized);
    logMessage("connect file tree to unrealized", Qgis::MessageLevel::Info);
    // add double click event processing
    connect(mpFileTreeWidget, &QTreeWidget::itemDoubleClicked, this,
            &MainWindow::onTreeItemDoubleClicked);

    // create select directory button
    QToolButton *selectDirectoryButton = new QToolButton(pRightDockWidget);
    selectDirectoryButton->setObjectName("selectDirectoryButton");
    selectDirectoryButton->setObjectName("selectDirectoryButton");
    selectDirectoryButton->setText(tr("Select Directory"));
    connect(selectDirectoryButton, &QToolButton::clicked, this,
            &MainWindow::onSelectDirectoryClicked);
    logMessage("connect select directory button to onSelectDirectoryClicked", Qgis::MessageLevel::Info);

    // add button and file tree widget to vertical layout
    QVBoxLayout *layout = new QVBoxLayout(pRightDockWidget);
    layout->setObjectName("layout");
    layout->setObjectName("layout");
    layout->addWidget(selectDirectoryButton);
    layout->addWidget(mpFileTreeWidget);

    QWidget *container = new QWidget(pRightDockWidget);
    container->setLayout(layout);
    container->setObjectName("container");
    container->setObjectName("container");
    pRightDockWidget->setWidget(container);
    // load file list of specified directory to tree widget
    QString dirPath = ws::PathManager::getInstance().getRootDir();
    QString dirPath = ws::PathManager::getInstance().getRootDir();
    loadDirectoryFiles(dirPath);
    logMessage("load file list of specified directory to tree widget", Qgis::MessageLevel::Success);

    ///create right bottom tool box sidebar------------------------------------------------------------------------------------
    QDockWidget *mpRightDock = new QDockWidget(tr("tool box"), pRightDockWidget);
    mpRightDock->setObjectName("mpRightDock");
    mpRightDock->setObjectName("mpRightDock");
    mpRightDock->setAllowedAreas(Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, mpRightDock);
    QVBoxLayout *mpToolLayout = new QVBoxLayout(pRightDockWidget);
    mpToolLayout->setObjectName("mpToolLayout");
    mpToolLayout->setObjectName("mpToolLayout");
    QWidget *toolWidget = new QWidget(pRightDockWidget);
    toolWidget->setObjectName("toolWidget");
    toolWidget->setObjectName("toolWidget");
    QTreeWidget *mpToolTree = new QTreeWidget(toolWidget);
    mpToolTree->setHeaderHidden(true); // hide header
    logMessage("create tool tree", Qgis::MessageLevel::Success);

    // tool tree
    // ===== route planning group =====
    QTreeWidgetItem *pRoutePlanningItem = new QTreeWidgetItem(mpToolTree);
    pRoutePlanningItem->setText(0, tr("RoutePlanning"));
    QTreeWidgetItem *pPaintItem = new QTreeWidgetItem(pRoutePlanningItem);
    pPaintItem->setText(0, tr("paint"));
    QTreeWidgetItem *pCreateRouteItem = new QTreeWidgetItem(pRoutePlanningItem);
    pCreateRouteItem->setText(0, tr("Create Route"));
    QTreeWidgetItem *pSetHomeItem = new QTreeWidgetItem(pRoutePlanningItem);
    pSetHomeItem->setText(0, tr("Set Home"));
    QTreeWidgetItem *pAddControlPointItem = new QTreeWidgetItem(pRoutePlanningItem);
    pAddControlPointItem->setText(0, tr("Add Control Point"));
    QTreeWidgetItem *pEditPointsItem = new QTreeWidgetItem(pRoutePlanningItem);
    pEditPointsItem->setText(0, tr("Edit Points"));
    QTreeWidgetItem *pGenerateRouteItem = new QTreeWidgetItem(pRoutePlanningItem);
    pGenerateRouteItem->setText(0, tr("Generate Route"));
    logMessage("create route planning item", Qgis::MessageLevel::Success);

    // ===== simulation control group =====
    QTreeWidgetItem *pSimulationItem = new QTreeWidgetItem(mpToolTree);
    pSimulationItem->setText(0, tr("Simulation"));
    QTreeWidgetItem *pStartItem = new QTreeWidgetItem(pSimulationItem);
    pStartItem->setText(0, tr("Start"));
    QTreeWidgetItem *pPauseItem = new QTreeWidgetItem(pSimulationItem);
    pPauseItem->setText(0, tr("Pause"));
    QTreeWidgetItem *pResumeItem = new QTreeWidgetItem(pSimulationItem);
    pResumeItem->setText(0, tr("Resume"));
    QTreeWidgetItem *pReturnHomeItem = new QTreeWidgetItem(pSimulationItem);
    pReturnHomeItem->setText(0, tr("Return Home"));
    QTreeWidgetItem *pStopSimulationItem = new QTreeWidgetItem(pSimulationItem);
    pStopSimulationItem->setText(0, tr("Stop Simulation"));
    logMessage("create simulation control item", Qgis::MessageLevel::Success);

    // ===== view switch group =====
    QTreeWidgetItem *pSwitchViewItem = new QTreeWidgetItem(mpToolTree);
    pSwitchViewItem->setText(0, tr("SwitchView"));
    QTreeWidgetItem *pResetItem = new QTreeWidgetItem(pSwitchViewItem);
    pResetItem->setText(0, tr("Reset"));
    QTreeWidgetItem *p3DItem = new QTreeWidgetItem(pSwitchViewItem);
    p3DItem->setText(0, tr("SwitchTo3D"));
    QTreeWidgetItem *p2DItem = new QTreeWidgetItem(pSwitchViewItem);
    p2DItem->setText(0, tr("SwitchTo2D"));
    logMessage("create view switch item", Qgis::MessageLevel::Success);

    // ===== new flight parameters group =====
    QTreeWidgetItem *pFlightParamsItem = new QTreeWidgetItem(mpToolTree);
    pFlightParamsItem->setText(0, tr("Flight Parameters"));
    QTreeWidgetItem *pQueryParamsItem = new QTreeWidgetItem(pFlightParamsItem);
    pQueryParamsItem->setText(0, tr("Query Parameters"));
    logMessage("create flight parameters item", Qgis::MessageLevel::Success);

    // ===== new environmental data group =====
    QTreeWidgetItem *pEnvDataItem = new QTreeWidgetItem(mpToolTree);
    pEnvDataItem->setText(0, tr("Environmental Data"));
    QTreeWidgetItem *pRefreshDataItem = new QTreeWidgetItem(pEnvDataItem);
    pRefreshDataItem->setText(0, tr("Refresh Data"));

    mpToolLayout->addWidget(mpToolTree);
    toolWidget->setLayout(mpToolLayout);
    mpRightDock->setWidget(toolWidget);
    logMessage("create tool box", Qgis::MessageLevel::Success);

    QDoubleSpinBox *pBaseHeightSpin = this->safeFindChild<QDoubleSpinBox*>("pBaseHeightSpin");
    QDoubleSpinBox *pHeightSpin = this->safeFindChild<QDoubleSpinBox*>("pHeightSpin");
    QDoubleSpinBox *pSpeedSpin = this->safeFindChild<QDoubleSpinBox*>("pSpeedSpin");
    QDoubleSpinBox *pWidthSpin = this->safeFindChild<QDoubleSpinBox*>("pWidthSpin");
    QPushButton *pBtnAddControlPoint = this->safeFindChild<QPushButton*>("pBtnAddControlPoint");
    QPushButton *pBtnCreateRoute = this->safeFindChild<QPushButton*>("pBtnCreateRoute");
    QPushButton *pBtnEditPoint = this->safeFindChild<QPushButton*>("pBtnEditPoint");
    QPushButton *pBtnGenerate = this->safeFindChild<QPushButton*>("pBtnGenerate");
    QPushButton *pBtnPause = this->safeFindChild<QPushButton*>("pBtnPause");
    QPushButton *pBtnQueryParams = this->safeFindChild<QPushButton*>("pBtnQueryParams");
    QPushButton *pBtnRefreshData = this->safeFindChild<QPushButton*>("pBtnRefreshData");
    QPushButton *pBtnResume = this->safeFindChild<QPushButton*>("pBtnResume");
    QPushButton *pBtnReturn = this->safeFindChild<QPushButton*>("pBtnReturn");
    QPushButton *pBtnSetHome = this->safeFindChild<QPushButton*>("pBtnSetHome");
    QPushButton *pBtnStart = this->safeFindChild<QPushButton*>("pBtnStart");
    QPushButton *pBtnStop = this->safeFindChild<QPushButton*>("pBtnStop");
    QDoubleSpinBox *pBaseHeightSpin = this->safeFindChild<QDoubleSpinBox*>("pBaseHeightSpin");
    QDoubleSpinBox *pHeightSpin = this->safeFindChild<QDoubleSpinBox*>("pHeightSpin");
    QDoubleSpinBox *pSpeedSpin = this->safeFindChild<QDoubleSpinBox*>("pSpeedSpin");
    QDoubleSpinBox *pWidthSpin = this->safeFindChild<QDoubleSpinBox*>("pWidthSpin");
    QPushButton *pBtnAddControlPoint = this->safeFindChild<QPushButton*>("pBtnAddControlPoint");
    QPushButton *pBtnCreateRoute = this->safeFindChild<QPushButton*>("pBtnCreateRoute");
    QPushButton *pBtnEditPoint = this->safeFindChild<QPushButton*>("pBtnEditPoint");
    QPushButton *pBtnGenerate = this->safeFindChild<QPushButton*>("pBtnGenerate");
    QPushButton *pBtnPause = this->safeFindChild<QPushButton*>("pBtnPause");
    QPushButton *pBtnQueryParams = this->safeFindChild<QPushButton*>("pBtnQueryParams");
    QPushButton *pBtnRefreshData = this->safeFindChild<QPushButton*>("pBtnRefreshData");
    QPushButton *pBtnResume = this->safeFindChild<QPushButton*>("pBtnResume");
    QPushButton *pBtnReturn = this->safeFindChild<QPushButton*>("pBtnReturn");
    QPushButton *pBtnSetHome = this->safeFindChild<QPushButton*>("pBtnSetHome");
    QPushButton *pBtnStart = this->safeFindChild<QPushButton*>("pBtnStart");
    QPushButton *pBtnStop = this->safeFindChild<QPushButton*>("pBtnStop");
    connect(mpToolTree, &QTreeWidget::itemClicked, this,
        [=](QTreeWidgetItem *item, int) {
            if (item == pPaintItem) {   Unrealized();}
            else if (item == p3DItem) { switchTo3D();}
            else if (item == p2DItem) { switchTo2D();}
            // bind route planning group functions
            else if (item == pCreateRouteItem) {pBtnCreateRoute->click();}
            else if (item == pSetHomeItem) {pBtnSetHome->click();}
            else if (item == pAddControlPointItem) {pBtnAddControlPoint->click();}
            else if (item == pEditPointsItem) {pBtnEditPoint->click();}
            else if (item == pGenerateRouteItem) {pBtnGenerate->click();}
            // bind simulation control group functions
            else if (item == pStartItem) {pBtnStart->click();}
            else if (item == pPauseItem) {pBtnPause->click();}
            else if (item == pResumeItem) {pBtnResume->click();}
            else if (item == pReturnHomeItem) {pBtnReturn->click();}
            else if (item == pStopSimulationItem) {pBtnStop->click();}
            // bind parameter query function
            else if (item == pQueryParamsItem) {pBtnQueryParams->click();}
            // bind environmental data refresh function
            else if (item == pRefreshDataItem) {pBtnRefreshData->click();}
        }
    );
    logMessage("connect right widget to slots", Qgis::MessageLevel::Success);
    logMessage("connect right widget to slots", Qgis::MessageLevel::Success);
}
void MainWindow::createCanvas() {
    // ================= middle area =================
    mpStackedWidget = new QStackedWidget(this);
    setCentralWidget(mpStackedWidget);
    mpStackedWidget->setObjectName("mpStackedWidget");
    mpStackedWidget->addWidget(mpOpenGLWidget.get()); // add 3D view
    mpStackedWidget->addWidget(mpImageLabel); // add 2D view
    mpStackedWidget->setCurrentWidget(mpOpenGLWidget.get()); // set 3D view as default

    logMessage("create stacked widget", Qgis::MessageLevel::Info);
}
void MainWindow::createSlots() {
    logMessage("create slots", Qgis::MessageLevel::Info);
    QDoubleSpinBox *pBaseHeightSpin = this->safeFindChild<QDoubleSpinBox*>("pBaseHeightSpin");
    QDoubleSpinBox *pHeightSpin = this->safeFindChild<QDoubleSpinBox*>("pHeightSpin");
    QDoubleSpinBox *pSpeedSpin = this->safeFindChild<QDoubleSpinBox*>("pSpeedSpin");
    QDoubleSpinBox *pWidthSpin = this->safeFindChild<QDoubleSpinBox*>("pWidthSpin");
    QPushButton *pBtnAddControlPoint = this->safeFindChild<QPushButton*>("pBtnAddControlPoint");
    QPushButton *pBtnCreateRoute = this->safeFindChild<QPushButton*>("pBtnCreateRoute");
    QPushButton *pBtnEditPoint = this->safeFindChild<QPushButton*>("pBtnEditPoint");
    QPushButton *pBtnGenerate = this->safeFindChild<QPushButton*>("pBtnGenerate");
    QPushButton *pBtnPause = this->safeFindChild<QPushButton*>("pBtnPause");
    QPushButton *pBtnQueryParams = this->safeFindChild<QPushButton*>("pBtnQueryParams");
    QPushButton *pBtnRefreshData = this->safeFindChild<QPushButton*>("pBtnRefreshData");
    QPushButton *pBtnResume = this->safeFindChild<QPushButton*>("pBtnResume");
    QPushButton *pBtnReturn = this->safeFindChild<QPushButton*>("pBtnReturn");
    QPushButton *pBtnSetHome = this->safeFindChild<QPushButton*>("pBtnSetHome");
    QPushButton *pBtnStart = this->safeFindChild<QPushButton*>("pBtnStart");
    QPushButton *pBtnStop = this->safeFindChild<QPushButton*>("pBtnStop");
    
    connect(pBtnSetHome, &QPushButton::clicked, [this]() {
    if (!mpRoutePlanner->m_settingHomePointMode) {
        // enter setting home point mode
        mpRoutePlanner->setSettingHomePointMode(true);
        logMessage("enter setting home point mode", Qgis::MessageLevel::Success);
    }
    });
    connect(pBtnQueryParams, &QPushButton::clicked, this,
            &MainWindow::queryFlightParameters);
    connect(pBtnRefreshData, &QPushButton::clicked, this,
            &MainWindow::refreshBasicData);
    logMessage("connect left widget to slots", Qgis::MessageLevel::Success);

    mpStackedWidget->setObjectName("mpStackedWidget");
    mpStackedWidget->addWidget(mpOpenGLWidget.get()); // add 3D view
    mpStackedWidget->addWidget(mpImageLabel); // add 2D view
    mpStackedWidget->setCurrentWidget(mpOpenGLWidget.get()); // set 3D view as default

    logMessage("create stacked widget", Qgis::MessageLevel::Info);
}
void MainWindow::createSlots() {
    logMessage("create slots", Qgis::MessageLevel::Info);
    QDoubleSpinBox *pBaseHeightSpin = this->safeFindChild<QDoubleSpinBox*>("pBaseHeightSpin");
    QDoubleSpinBox *pHeightSpin = this->safeFindChild<QDoubleSpinBox*>("pHeightSpin");
    QDoubleSpinBox *pSpeedSpin = this->safeFindChild<QDoubleSpinBox*>("pSpeedSpin");
    QDoubleSpinBox *pWidthSpin = this->safeFindChild<QDoubleSpinBox*>("pWidthSpin");
    QPushButton *pBtnAddControlPoint = this->safeFindChild<QPushButton*>("pBtnAddControlPoint");
    QPushButton *pBtnCreateRoute = this->safeFindChild<QPushButton*>("pBtnCreateRoute");
    QPushButton *pBtnEditPoint = this->safeFindChild<QPushButton*>("pBtnEditPoint");
    QPushButton *pBtnGenerate = this->safeFindChild<QPushButton*>("pBtnGenerate");
    QPushButton *pBtnPause = this->safeFindChild<QPushButton*>("pBtnPause");
    QPushButton *pBtnQueryParams = this->safeFindChild<QPushButton*>("pBtnQueryParams");
    QPushButton *pBtnRefreshData = this->safeFindChild<QPushButton*>("pBtnRefreshData");
    QPushButton *pBtnResume = this->safeFindChild<QPushButton*>("pBtnResume");
    QPushButton *pBtnReturn = this->safeFindChild<QPushButton*>("pBtnReturn");
    QPushButton *pBtnSetHome = this->safeFindChild<QPushButton*>("pBtnSetHome");
    QPushButton *pBtnStart = this->safeFindChild<QPushButton*>("pBtnStart");
    QPushButton *pBtnStop = this->safeFindChild<QPushButton*>("pBtnStop");
    
    connect(pBtnSetHome, &QPushButton::clicked, [this]() {
    if (!mpRoutePlanner->m_settingHomePointMode) {
        // enter setting home point mode
        mpRoutePlanner->setSettingHomePointMode(true);
        logMessage("enter setting home point mode", Qgis::MessageLevel::Success);
    }
    });
    connect(pBtnQueryParams, &QPushButton::clicked, this,
            &MainWindow::queryFlightParameters);
    connect(pBtnRefreshData, &QPushButton::clicked, this,
            &MainWindow::refreshBasicData);
    logMessage("connect left widget to slots", Qgis::MessageLevel::Success);

    // connect signal in main window constructor
    connect(mpStackedWidget, &QStackedWidget::currentChanged, this,
            [=](int index) {
            if (index == 0 && ws::WindowManager::getInstance().getCurrentCanvas() == ws::CanvasType::ThreeD) { // assume MyOpenGLWidget is the first page (index 0)
            if (index == 0 && ws::WindowManager::getInstance().getCurrentCanvas() == ws::CanvasType::ThreeD) { // assume MyOpenGLWidget is the first page (index 0)
                mpOpenGLWidget->setFocus(); // switch back to force focus
            }
            });
    logMessage("connect signal in main window constructor", Qgis::MessageLevel::Info);
    // ================= signal and slot connection =================

    connect(mpBtnReset, &QPushButton::clicked, this, &MainWindow::resetView);
    logMessage("connect reset view button to reset view", Qgis::MessageLevel::Info);
    connect(mpBtnSwitchTo3D, &QPushButton::clicked, this,
            &MainWindow::switchTo3D);
    logMessage("connect switch to 3D button to switch to 3D", Qgis::MessageLevel::Info);
    connect(mpBtnSwitchTo2D, &QPushButton::clicked, this,
            &MainWindow::switchTo2D);
    logMessage("connect switch to 2D button to switch to 2D", Qgis::MessageLevel::Info);
    
    connect(pBtnCreateRoute, &QPushButton::clicked, mpRoutePlanner.get(),
            &RoutePlanner::enterRoutePlanningMode); // start route planning
    logMessage("connect create route button to enter route planning mode", Qgis::MessageLevel::Info);
    connect(pBtnAddControlPoint, &QPushButton::clicked, [this]() {
    if (!mpRoutePlanner->m_isAddingControlPoint) {
        mpRoutePlanner->m_isAddingControlPoint = true; // enter "add control point" mode
    } else
        mpRoutePlanner->m_isAddingControlPoint = false; // exit
    });
    logMessage("connect add control point button to enter add control point mode", Qgis::MessageLevel::Info);
    connect(pBtnEditPoint, &QPushButton::clicked, [this]() {
        if (!mpRoutePlanner->m_editingMode) {
        mpRoutePlanner->m_editingMode = true; // enter "edit point" mode
        } else
        mpRoutePlanner->m_editingMode = false; // exit
    });
    logMessage("connect edit point button to enter edit point mode", Qgis::MessageLevel::Info);
    connect(pBtnGenerate, &QPushButton::clicked, [=]() {
    //mpOpenGLWidget->generateFlightRoute(pHeightSpin->value());
    });
    logMessage("connect generate flight route button to generate flight route", Qgis::MessageLevel::Info);

    connect(pHeightSpin, SIGNAL(valueChanged(double)), mpOpenGLWidget.get(),
            SLOT(updateFlightHeight(double))); //传递行高到MyOpenGLWidget类中
    logMessage("connect height spin box to update flight height", Qgis::MessageLevel::Info);
    connect(pWidthSpin, SIGNAL(valueChanged(double)), mpRoutePlanner.get(),
            SLOT(setScanSpacing(double))); //传递航带宽度到RoutePlanner类中
    logMessage("connect width spin box to set scan spacing", Qgis::MessageLevel::Info);

    /*
    // flight simulation related connections
    connect(pBtnStart, &QPushButton::clicked,
            [=]() { mpOpenGLWidget->startSimulation(pSpeedSpin->value()); });
    connect(pBtnPause, &QPushButton::clicked, mpOpenGLWidget.get(),
            &MyOpenGLWidget::pauseSimulation);
    connect(pBtnResume, &QPushButton::clicked, mpOpenGLWidget.get(),
            &MyOpenGLWidget::resumeSimulation);
    connect(pBtnReturn, &QPushButton::clicked, mpOpenGLWidget.get(),
            &MyOpenGLWidget::returnToHome);
    connect(pBtnStop, &QPushButton::clicked, mpOpenGLWidget.get(),
            &MyOpenGLWidget::stopSimulation);
    connect(pBaseHeightSpin, SIGNAL(valueChanged(double)), mpOpenGLWidget.get(),
            SLOT(ws::FlightManager::getInstance().setBaseHeight(double)));
    logMessage("connected all slots on main window", Qgis::MessageLevel::Success);
    */
}
void MainWindow::createMainWindow() {
    logMessage("create main window", Qgis::MessageLevel::Info);
    createLeftDockWidget();
    logMessage("create left dock widget", Qgis::MessageLevel::Success);
    logMessage("create left dock widget", Qgis::MessageLevel::Success);
    createRightDockWidget();
    logMessage("create right dock widget", Qgis::MessageLevel::Success);
    logMessage("create right dock widget", Qgis::MessageLevel::Success);
    createCanvas();
    logMessage("create canvas", Qgis::MessageLevel::Success);
    createSlots();
    logMessage("create canvas", Qgis::MessageLevel::Success);
    createSlots();
    logMessage("create main window", Qgis::MessageLevel::Success);
}
// select file list directory

void MainWindow::onSelectDirectoryClicked() {
    // open folder selection dialog
    QString currentDir = ws::PathManager::getInstance().getRootDir();
    QString currentDir = ws::PathManager::getInstance().getRootDir();
    QString dirPath = QFileDialog::getExistingDirectory(
        this, tr("Select Directory"), currentDir);
        this, tr("Select Directory"), currentDir);
    if (!dirPath.isEmpty()) {
        loadDirectoryFiles(dirPath); // call loadDirectoryFiles to load selected directory
        loadDirectoryFiles(dirPath); // call loadDirectoryFiles to load selected directory
    }
    logMessage("select file list directory", Qgis::MessageLevel::Success);
}
// load file list of specified directory to QTreeWidget
void MainWindow::loadDirectoryFiles(const QString &path) {
    QDir dir(path);
    if (!dir.exists()) return;
    if (!dir.exists()) return;

    mpFileTreeWidget->clear();
    mpFileTreeWidget->clear();

    QTreeWidgetItem *rootItem = new QTreeWidgetItem(mpFileTreeWidget);
    rootItem->setText(0, dir.dirName());
    loadDirectoryLevel(rootItem, path, 1, 3);

    connect(mpFileTreeWidget, &QTreeWidget::itemExpanded, this, &MainWindow::onTreeItemExpanded);
}

void MainWindow::loadDirectoryLevel(QTreeWidgetItem *parentItem, const QString &path, int level, int maxLevel) {
    if (level > maxLevel) return;

    QDir dir(path);
    QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    rootItem->setText(0, dir.dirName());
    loadDirectoryLevel(rootItem, path, 1, 3);

    connect(mpFileTreeWidget, &QTreeWidget::itemExpanded, this, &MainWindow::onTreeItemExpanded);
}

void MainWindow::loadDirectoryLevel(QTreeWidgetItem *parentItem, const QString &path, int level, int maxLevel) {
    if (level > maxLevel) return;

    QDir dir(path);
    QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    foreach (const QFileInfo &fileInfo, files) {
        QTreeWidgetItem *item = new QTreeWidgetItem(parentItem);
        item->setText(0, fileInfo.fileName());
    foreach (const QFileInfo &fileInfo, files) {
        QTreeWidgetItem *item = new QTreeWidgetItem(parentItem);
        item->setText(0, fileInfo.fileName());

        if (fileInfo.isDir()) {
            if (level < maxLevel) {
                loadDirectoryLevel(item, fileInfo.absoluteFilePath(), level + 1, maxLevel);
            } else if (level == maxLevel) {
                new QTreeWidgetItem(item);
            }
        }
    }
        if (fileInfo.isDir()) {
            if (level < maxLevel) {
                loadDirectoryLevel(item, fileInfo.absoluteFilePath(), level + 1, maxLevel);
            } else if (level == maxLevel) {
                new QTreeWidgetItem(item);
            }
        }
    }
}

// switch to 3D
void MainWindow::switchTo3D() {
    if (!ws::WindowManager::getInstance().get3DMapInited()) {
        logMessage("3D map not initialized", Qgis::MessageLevel::Info);
        init3DWidget();
        logMessage("3D map initialized", Qgis::MessageLevel::Success);
    }
    mpStackedWidget->setCurrentWidget(mpOpenGLWidget.get()); // switch to 3D model view
    ws::WindowManager::getInstance().setCurrentCanvas(ws::CanvasType::ThreeD);
    ws::WindowManager::getInstance().setCurrentCanvas(ws::CanvasType::ThreeD);
    logMessage("switch to 3D model view", Qgis::MessageLevel::Success);
}
// switch to 2D
void MainWindow::switchTo2D() {
    if (!ws::WindowManager::getInstance().get2DMapInited()) {
        logMessage("2D map not initialized", Qgis::MessageLevel::Info);
        init2DWidget();
        logMessage("2D map initialized", Qgis::MessageLevel::Success);
    }
    logMessage("switch to 2D map view", Qgis::MessageLevel::Info);
    mpStackedWidget->setCurrentWidget(mpImageLabel); // switch to 2D map view
    ws::WindowManager::getInstance().setCurrentCanvas(ws::CanvasType::TwoD);
    ws::WindowManager::getInstance().setCurrentCanvas(ws::CanvasType::TwoD);
    logMessage("switch to 2D map view", Qgis::MessageLevel::Success);
}

void MainWindow::resetView() {
    Camera::getInstance().resetView();
    logMessage("reset view", Qgis::MessageLevel::Success);
}

// add new slot function at the end of the file
void MainWindow::queryFlightParameters() {
    logMessage("generate random flight parameters", Qgis::MessageLevel::Info);
    double speed = ws::FlightManager::getInstance().getFlightSpeed();
    double altitude = ws::FlightManager::getInstance().getFlightAltitude();
    double battery = ws::FlightManager::getInstance().getFlightBattery();
    double speed = ws::FlightManager::getInstance().getFlightSpeed();
    double altitude = ws::FlightManager::getInstance().getFlightAltitude();
    double battery = ws::FlightManager::getInstance().getFlightBattery();
    double latitude = QRandomGenerator::global()->bounded(-90, 90);
    double longitude = QRandomGenerator::global()->bounded(-180, 180);

    QString params = QString("Current Flight Parameters:\n"
                            "Speed: %1 m/s\n"
                            "Altitude: %2 m\n"
                            "Battery: %3%\n"
                            "Position: (%4, %5)")
                        .arg(speed, 0, 'f', 1)
                        .arg(altitude, 0, 'f', 1)
                        .arg(battery, 0, 'f', 1)
                        .arg(latitude, 0, 'f', 6)
                        .arg(longitude, 0, 'f', 6);

    m_pFlightParamsDisplay->setText(params);
    logMessage("generate random flight parameters", Qgis::MessageLevel::Success);
}

void MainWindow::refreshBasicData() {
    logMessage("generate random weather data", Qgis::MessageLevel::Info);
    QStringList weatherTypes = {"Sunny", "Cloudy", "Rainy", "Snowy", "Foggy"};
    QString weather = weatherTypes[QRandomGenerator::global()->bounded(5)];

    ws::EnvManager& envManager = ws::EnvManager::getInstance();
    double temperature = QRandomGenerator::global()->bounded(envManager.minTemperature * 10, envManager.maxTemperature * 10) / 10.0;
    ws::EnvManager& envManager = ws::EnvManager::getInstance();
    double temperature = QRandomGenerator::global()->bounded(envManager.minTemperature * 10, envManager.maxTemperature * 10) / 10.0;

    double pressure = QRandomGenerator::global()->bounded(envManager.minPressure * 10, envManager.maxPressure * 10) / 10.0;
    double pressure = QRandomGenerator::global()->bounded(envManager.minPressure * 10, envManager.maxPressure * 10) / 10.0;

    m_pWeatherLabel->setText(weather);
    m_pTemperatureLabel->setText(QString("%1 C").arg(temperature, 0, 'f', 1));
    m_pPressureLabel->setText(
        QString("%1 hPa").arg(pressure, 0, 'f', 1)); // 注意这里有两个闭合括号
}

void MainWindow::setTianDiTuMap(double lat, double lon, int zoom) {
    QString key = "3e6c3b63b9529d502fc08c5850dfa5d5";

    QString htmlContent =
      QString("<!DOCTYPE html>"
              "<html>"
              "<head>"
              "<title>TianDiTu Map</title>"
              "<link rel='stylesheet' "
              "href='https://unpkg.com/leaflet@1.7.1/dist/leaflet.css'/>"
              "<script "
              "src='https://unpkg.com/leaflet@1.7.1/dist/leaflet.js'></script>"
              "<style>"
              "#map { height: 100%%; width: 100%%; }"
              "body { margin: 0; padding: 0; }"
              "</style>"
              "</head>"
              "<body>"
              "<div id='map'></div>"
              "<script>"
              "var map = L.map('map', {"
              "   crs: L.CRS.EPSG4326," // use EPSG4326 coordinate system
              "   center: [%4, %3],"    // latitude, longitude
              "   zoom: %2"
              "});"
              ""
              "L.tileLayer("
              "   "
              "'http://t0.tianditu.gov.cn/img_c/"
              "wmts?tk=%1&layer=img&style=default&tilematrixset=wgs84&Service="
              "WMTS&Request=GetTile&Version=1.0.0&Format=image/"
              "jpeg&TileMatrix=%2&TileRow=%3&TileCol=%4"
              "    "
              "Request=GetTile&Version=1.0.0&Format=image/"
              "jpeg&TileMatrix={z}&TileRow={y}&TileCol={x}',"
              "   {"
              "       tms: true,"              // enable TMS coordinate system
              "       maxZoom: 18,"            // maximum zoom level
              "       tileSize: 256,"          // tile size
              "       attribution: 'TianDiTu'" // attribution
              "   }"
              ").addTo(map);"
              "</script>"
              "</body>"
              "</html>")
          .arg(key)
          .arg(lat)
          .arg(lon)
          .arg(zoom);

  // mpWebView->setHtml(htmlContent);
  logMessage("set TianDiTu map", Qgis::MessageLevel::Success);
}
void MainWindow::showUserManual() {
    logMessage("show user manual", Qgis::MessageLevel::Info);
    QDialog *manualDialog = new QDialog(this);
    manualDialog->setWindowTitle(tr("User Manual"));

    manualDialog->setWindowFlag(Qt::WindowContextHelpButtonHint, true);

    QTextEdit *textEdit = new QTextEdit(manualDialog);
    textEdit->setText(
        "======================== 用户手册 ========================\n"
        "欢迎使用3D航线规划仿真系统\n\n"

        "【程序概述】\n"
        "本程序提供三维场景下的无人机航线规划与飞行仿真功能，支持：\n"
        "- OBJ格式3D模型加载与展示\n"
        "- 二维地图联动显示\n"
        "- 交互式航线规划\n"
        "- 三维飞行仿真\n"
        "- 多视角观察\n\n"

        "【界面布局】\n"
        "1. 左侧控制面板\n"
        "   - 视图切换：3D视图/2D地图/重置视角\n"
        "   - 航线规划：设置基准高度/飞行高度/航带宽度\n"
        "   - 仿真控制：启动/暂停/继续/返航\n"
        "2. 右侧属性面板\n"
        "   - 文件树：显示加载的3D模型目录结构\n"
        "   - 工具箱：快速访问常用工具\n"
        "3. 中央视图区\n"
        "   - 3D视图：支持鼠标拖拽旋转/滚轮缩放\n"
        "   - 2D地图：集成OpenStreetMap在线地图\n\n"

        "【核心功能指南】\n"
        "▶ 加载3D模型：\n"
        "   1. 点击菜单栏 Project -> load 3D file\n"
        "   2. 选择包含.obj和.jpg的文件夹\n"
        "   3. 模型将自动加载到3D视图\n\n"

        "▶ 创建航线：\n"
        "   1. 在控制面板设置基准高度（地面高度）\n"
        "   2. 点击'Create Route'进入规划模式\n"
        "   3. 操作步骤：\n"
        "      a) 点击'Set Home Point'设置起飞点\n"
        "      b) 点击'Add Control Point'添加控制点\n"
        "      c) 调整航带宽度（相邻航线间距）\n"
        "      d) 点击'Generate Route'生成最终航线\n"
        "   4. 右键点击控制点可进行删除操作\n\n"

        "▶ 飞行仿真：\n"
        "   1. 设置飞行速度（建议5-15 m/s）\n"
        "   2. 操作按钮：\n"
        "      - Start：从Home点开始仿真\n"
        "      - Pause：暂停仿真\n"
        "      - Resume：继续飞行\n"
        "      - Return Home：立即返航\n"
        "      - Stop：终止仿真\n"
        "   3. 按空格键切换摄像机跟随模式\n\n"

        "▶ 视角操作：\n"
        "   - 鼠标左键拖拽：旋转视角\n"
        "   - 鼠标滚轮：缩放视图\n"
        "   - 方向键：平移视图\n"
        "   - Reset按钮：恢复默认视角\n\n"

        "【参数说明】\n"
        "▶ 飞行参数：\n"
        "   - 基准高度：模型所在平面高度（单位：米）\n"
        "   - 飞行高度：相对基准面的垂直高度（±1000m）\n"
        "   - 航带宽度：相邻航线水平间距（1-300m）\n"
        "   - 飞行速度：仿真速度（1-50m/s）\n\n"

        "▶ 环境参数：\n"
        "   - 温度：模拟环境温度（-20°C~45°C）\n"
        "     影响电池效率（低温降低续航）\n"
        "   - 气压：大气压强（950-1050hPa）\n"
        "     用于高度计校准参考\n"
        "   - 天气状况：模拟气象条件\n"
        "     ■ 晴天：正常能见度\n"
        "     ■ 雨天：降低最大飞行速度20%\n"
        "     ■ 雪天：缩短有效控制距离30%\n"
        "     ■ 雾天：开启防撞雷达\n\n"

        "▶ 系统参数：\n"
        "   - 模型缩放：默认1:1比例尺\n"
        "   - 纹理精度：2048x2048像素\n"
        "   - 刷新频率：60Hz（垂直同步）\n\n"

        "【常见问题】\n"
        "Q1: 控制点添加不成功？\n"
        "A: 请确保：1) 已进入规划模式 2) 鼠标点击位置在模型表面\n\n"
        "Q2: 生成的航线不符合预期？\n"
        "A: 检查控制点是否形成有效凸包，建议至少设置3个控制点\n\n"
        "Q3: 3D模型显示异常？\n"
        "A: 确认模型文件包含配套的.jpg纹理文件，且尺寸为2的幂次方\n\n"

        "【技术支持】\n"
        "联系开发者：18372124178\n"
        "电子邮箱：wankj@cug.edu.cn\n"
        "更新日期：2024-03-04");
    textEdit->setReadOnly(true);
    textEdit->setFixedSize(800, 600);

    textEdit->setWhatsThis(tr("If you have any questions or issues, please "
                            "contact the developer at: 18372124178."));

    logMessage("show user manual text set", Qgis::MessageLevel::Success);

    QVBoxLayout *dialogLayout = new QVBoxLayout(manualDialog);
    dialogLayout->addWidget(textEdit);
    manualDialog->setLayout(dialogLayout);

    manualDialog->exec();
    logMessage("show user manual", Qgis::MessageLevel::Success);
}
void MainWindow::showFlightParamsDialog() {
    logMessage("show flight parameters dialog", Qgis::MessageLevel::Info);
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(tr("Flight Parameters Settings"));

    QFormLayout *form = new QFormLayout(dialog);

    ws::FlightManager& flightManager = ws::FlightManager::getInstance();
    ws::FlightManager& flightManager = ws::FlightManager::getInstance();
    QDoubleSpinBox *speedSpin = new QDoubleSpinBox(dialog);
    speedSpin->setRange(ws::FlightManager::minFlightSpeed, ws::FlightManager::maxFlightSpeed);
    speedSpin->setValue(ws::FlightManager::getInstance().getFlightSpeed());
    speedSpin->setRange(ws::FlightManager::minFlightSpeed, ws::FlightManager::maxFlightSpeed);
    speedSpin->setValue(ws::FlightManager::getInstance().getFlightSpeed());
    form->addRow(tr("Flight Speed (m/s):"), speedSpin);

    QDoubleSpinBox *altitudeSpin = new QDoubleSpinBox(dialog);
    altitudeSpin->setRange(ws::FlightManager::minFlightAltitude, ws::FlightManager::maxFlightAltitude);
    altitudeSpin->setValue(ws::FlightManager::getInstance().getFlightAltitude());
    altitudeSpin->setRange(ws::FlightManager::minFlightAltitude, ws::FlightManager::maxFlightAltitude);
    altitudeSpin->setValue(ws::FlightManager::getInstance().getFlightAltitude());
    form->addRow(tr("Max Altitude (m):"), altitudeSpin);

    QDoubleSpinBox *batterySpin = new QDoubleSpinBox(dialog);
    batterySpin->setRange(ws::FlightManager::minFlightBattery, ws::FlightManager::maxFlightBattery);
    batterySpin->setValue(ws::FlightManager::getInstance().getFlightBattery());
    batterySpin->setRange(ws::FlightManager::minFlightBattery, ws::FlightManager::maxFlightBattery);
    batterySpin->setValue(ws::FlightManager::getInstance().getFlightBattery());
    form->addRow(tr("Battery Capacity (%):"), batterySpin);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, dialog);
    form->addRow(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    logMessage("connect flight parameters dialog button box accepted", Qgis::MessageLevel::Success);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    logMessage("connect flight parameters dialog button box rejected", Qgis::MessageLevel::Success);

    if (dialog->exec() == QDialog::Accepted) {
        flightManager.setFlightSpeed(speedSpin->value());
        flightManager.setFlightAltitude(altitudeSpin->value());
        flightManager.setFlightBattery(batterySpin->value());

        queryFlightParameters(); // 刷新飞行参数显示
    }
    logMessage("show flight parameters dialog", Qgis::MessageLevel::Success);
}
void MainWindow::showEnvironmentalParamsDialog() {
    logMessage("show environmental parameters dialog", Qgis::MessageLevel::Info);
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(tr("Environmental Parameters Settings"));

    QFormLayout *form = new QFormLayout(dialog);

    ws::EnvManager& envManager = ws::EnvManager::getInstance();
    ws::EnvManager& envManager = ws::EnvManager::getInstance();
    QComboBox *weatherCombo = new QComboBox(dialog);
    weatherCombo->addItems(envManager.weatherList);
    weatherCombo->setCurrentText(envManager.getWeatherString());
    weatherCombo->addItems(envManager.weatherList);
    weatherCombo->setCurrentText(envManager.getWeatherString());
    form->addRow(tr("Weather Condition:"), weatherCombo);

    QDoubleSpinBox *tempSpin = new QDoubleSpinBox(dialog);
    tempSpin->setRange(ws::EnvManager::minTemperature, ws::EnvManager::maxTemperature);
    tempSpin->setValue(envManager.getTemperature());
    tempSpin->setRange(ws::EnvManager::minTemperature, ws::EnvManager::maxTemperature);
    tempSpin->setValue(envManager.getTemperature());
    form->addRow(tr("Temperature (°C):"), tempSpin);

    QDoubleSpinBox *pressureSpin = new QDoubleSpinBox(dialog);
    pressureSpin->setRange(ws::EnvManager::minPressure, ws::EnvManager::maxPressure);
    pressureSpin->setValue(envManager.getPressure());
    pressureSpin->setRange(ws::EnvManager::minPressure, ws::EnvManager::maxPressure);
    pressureSpin->setValue(envManager.getPressure());
    form->addRow(tr("Pressure (hPa):"), pressureSpin);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, dialog);
    form->addRow(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    logMessage("connect environmental parameters dialog button box accepted", Qgis::MessageLevel::Success);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    logMessage("connect environmental parameters dialog button box rejected", Qgis::MessageLevel::Success);

    if (dialog->exec() == QDialog::Accepted) {
        envManager.setWeather(static_cast<ws::WeatherType>(weatherCombo->currentIndex()));
        envManager.setTemperature(tempSpin->value());
        envManager.setPressure(pressureSpin->value());
        envManager.setWeather(static_cast<ws::WeatherType>(weatherCombo->currentIndex()));
        envManager.setTemperature(tempSpin->value());
        envManager.setPressure(pressureSpin->value());

        refreshBasicData(); // refresh environment data display
    }
    logMessage("show environmental parameters dialog", Qgis::MessageLevel::Success);
}

void MainWindow::createJoyDockWidgets() {
void MainWindow::createJoyDockWidgets() {
    //create flight control dock widget
    QDockWidget *controlDock = new QDockWidget(tr("Flight Control"), this);
    QWidget *controlPanel = new QWidget(this);

    //main vertical layout
    QVBoxLayout *mainLayout = new QVBoxLayout(controlPanel);
    mainLayout->setContentsMargins(20, 20, 20, 20); // 增加边距
    mainLayout->setSpacing(30);                     // 摇杆与按钮间距

    //joystick row (horizontal layout)=====================================
    QHBoxLayout *joystickLayout = new QHBoxLayout(controlPanel);
    joystickLayout->setSpacing(40); // 增大摇杆间距

    // m_leftJoystick = new JoystickWidget();
    // m_rightJoystick = new JoystickWidget();

    // 设置大尺寸摇杆
    const int joystickSize = 180; // 摇杆尺寸
    // m_leftJoystick->setFixedSize(joystickSize, joystickSize);
    // m_rightJoystick->setFixedSize(joystickSize, joystickSize);

    QString joystickStyle = "background-color: #333333;"
                            "border-radius: " +
                            QString::number(joystickSize / 2) +
                            "px;"; // 动态计算圆角
    // m_leftJoystick->setStyleSheet(joystickStyle);
    // m_rightJoystick->setStyleSheet(joystickStyle);

    // joystickLayout->addWidget(m_leftJoystick);
    // joystickLayout->addWidget(m_rightJoystick);
    mainLayout->addLayout(joystickLayout);

    // 按钮行（保持大尺寸）===================================
    QHBoxLayout *buttonLayout = new QHBoxLayout(controlPanel);
    buttonLayout->setAlignment(Qt::AlignHCenter);
    buttonLayout->setSpacing(30); // 按钮间距

    m_btnManualMode = new QPushButton("手动模式", controlPanel);
    m_btnAutoMode = new QPushButton("自动模式", controlPanel);

    // 按钮样式（保持大尺寸但调整比例）
    QString buttonStyle = "QPushButton {"
                        "  background-color: #4A4A4A;"
                        "  border: 2px solid #5A5A5A;"
                        "  border-radius: 8px;"
                        "  padding: 15px 30px;"         // 增大内边距
                        "  min-width: 140px;"           // 保持大宽度
                        "  min-height: 45px;"           // 保持大高度
                        "  font: bold 16px '微软雅黑';" // 字体稍增大
                        "}"
                        "QPushButton:hover {"
                        "  background-color: #5A5A5A;"
                        "}";
    m_btnManualMode->setStyleSheet(buttonStyle);
    m_btnAutoMode->setStyleSheet(buttonStyle);

    buttonLayout->addWidget(m_btnManualMode);
    buttonLayout->addWidget(m_btnAutoMode);
    mainLayout->addLayout(buttonLayout);

    controlDock->setWidget(controlPanel);
    addDockWidget(Qt::RightDockWidgetArea, controlDock);
}

void MainWindow::onTreeItemExpanded(QTreeWidgetItem *item) {
    if (item->childCount() == 1 && item->child(0)->text(0).isEmpty()) {
        QString path = getItemFullPath(item);
        item->removeChild(item->child(0));
        loadDirectoryLevel(item, path, 1, 1);
        
        for (int i = 0; i < item->childCount(); ++i) {
            QTreeWidgetItem *child = item->child(i);
            QFileInfo fileInfo(getItemFullPath(child));

            if (fileInfo.isDir())
                child->setHidden(false);
            else
                child->setHidden(!child->text(0).endsWith(".obj")); // only show obj file
        }
    }
}

QString MainWindow::getItemFullPath(QTreeWidgetItem *item) {
    QStringList pathParts;
    while (item && item->parent()) {
        pathParts.prepend(item->text(0));
        item = item->parent();
    }
    if (item) pathParts.prepend(item->text(0));
    QString rootPath = ws::PathManager::getInstance().getRootDir();
    return QDir(rootPath).filePath(pathParts.join("/"));
}

void MainWindow::onTreeItemDoubleClicked(QTreeWidgetItem *item, int column) {
    if (!item) return;
    
    QString filePath = getItemFullPath(item);
    QFileInfo fileInfo(filePath);
    
    if (fileInfo.isFile() && fileInfo.suffix().toLower() == "obj") {
        mpOpenGLWidget->loadModel(filePath);
    }
}

void MainWindow::loadModel(const QString& objFilePath){
    mpOpenGLWidget->loadModel(objFilePath);
}