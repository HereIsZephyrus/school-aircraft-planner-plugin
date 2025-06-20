#include "MainWindow.h"
#include "core/WorkspaceState.h"
#include "core/VideoManager.h"
#include "gui/Canvas.h"
#include "gui/LeftDockWidget.h"
#include "gui/StyleManager.h"
#include "log/QgisDebug.h"
#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QScreen>
#include <QTextEdit>
#include <QTextStream>
#include <memory>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  logMessage("Application started", Qgis::MessageLevel::Info);
  wsp::initializeWorkspaceState();
  initWindowStatus();
  
  mpMenuBar = new MenuBar(this);
  setMenuBar(mpMenuBar);
  logMessage("set menu bar", Qgis::MessageLevel::Info);
  if (!mpMenuBar) {
    logMessage("Failed to create menu bar", Qgis::MessageLevel::Critical);
    return;
  }

  mpCanvas = new Canvas(this);
  QMainWindow::setCentralWidget(mpCanvas);
  logMessage("set central widget", Qgis::MessageLevel::Info);
  if (!mpCanvas) {
    logMessage("Failed to create canvas", Qgis::MessageLevel::Critical);
    return;
  }

  mpLeftDockWidget = new LeftDockWidget(this);
  addDockWidget(Qt::LeftDockWidgetArea, mpLeftDockWidget);
  logMessage("add left dock widget", Qgis::MessageLevel::Info);
  if (!mpLeftDockWidget) {
    logMessage("Failed to create left dock widget",
               Qgis::MessageLevel::Critical);
    return;
  }

  mpRightDockWidget = new RightDockWidget(this);
  addDockWidget(Qt::RightDockWidgetArea, mpRightDockWidget);
  logMessage("add right dock widget", Qgis::MessageLevel::Info);
  if (!mpRightDockWidget) {
    logMessage("Failed to create right dock widget",
               Qgis::MessageLevel::Critical);
    return;
  }

  // 初始化风险事件路径规划对话框
  mpRiskEventPlannerDialog = nullptr; // 延迟创建
  mpGridPathPlannerDialog = nullptr; // 延迟创建
  
  // 初始化路径可视化
  mpRouteVisualization = new RouteVisualization(this);
  logMessage("create route visualization", Qgis::MessageLevel::Info);

  createSlots();
  
  // 初始化视频管理器
  VideoManager::getInstance().initialize();
  VideoManager::getInstance().setVideoDisplayWidget(mpRightDockWidget->getVideoDisplayWidget());
  

  QString appDir = QApplication::applicationDirPath();
  qDebug() << "MainWindow appDir:" << appDir;
  

  QDir projectDir(appDir);
  projectDir.cdUp(); 
  projectDir.cdUp(); 
  projectDir.cdUp(); 
  
  QString videoPath = projectDir.filePath("resources/video/VID_20250617094821_analyzed2.wmv");
  qDebug() << "MainWindow videoPath:" << videoPath;
  VideoManager::getInstance().setVideoSource(VideoSourceType::FILE, videoPath);
  
  logMessage("create main window", Qgis::MessageLevel::Success);
}

void MainWindow::release() {
  disconnect();
  delete mpCanvas;
  delete mpLeftDockWidget;
  delete mpRightDockWidget;
  delete mpMenuBar;
  delete mpRiskEventPlannerDialog;
  delete mpGridPathPlannerDialog;
  delete mpRouteVisualization;
  logMessage("MainWindow released", Qgis::MessageLevel::Success);
}

MainWindow::~MainWindow() {
  logMessage("MainWindow destroyed", Qgis::MessageLevel::Success);
}

void MainWindow::createSlots() {
  using namespace wsp;
  connect(mpCanvas, &Canvas::refreashParms, mpLeftDockWidget->getFlightQueryGroup(), &FlightQueryGroup::refreshFlightParams);
  connect(mpLeftDockWidget->getViewGroup(), &ViewGroup::switchTo3D, mpCanvas,
          &Canvas::switchTo3D);
  connect(mpLeftDockWidget->getViewGroup(), &ViewGroup::switchTo2D, mpCanvas,
          &Canvas::switchTo2D);
  connect(mpLeftDockWidget->getViewGroup(), &ViewGroup::viewReset, mpCanvas,
          &Canvas::viewReset);
  connect(mpLeftDockWidget->getRouteGroup(), &RouteGroup::createRoute,
          &RoutePlanner::getInstance(), &RoutePlanner::createControlPoint);
  connect(mpLeftDockWidget->getRouteGroup(), &RouteGroup::editRoute,
          &RoutePlanner::getInstance(), &RoutePlanner::editRoute);
  connect(mpLeftDockWidget->getFlightSimGroup(),
          &FlightSimGroup::simulationStart, &AnimationManager::getInstance(),
          &AnimationManager::startSimulation);
  connect(mpLeftDockWidget->getFlightSimGroup(),
          &FlightSimGroup::simulationStart, this, [this](){
      VideoManager::getInstance().startVideoStream();
  });
  connect(mpLeftDockWidget->getFlightSimGroup(),
          &FlightSimGroup::simulationPause, &AnimationManager::getInstance(),
          &AnimationManager::pauseSimulation);
  connect(mpLeftDockWidget->getFlightSimGroup(),
          &FlightSimGroup::simulationResume, &AnimationManager::getInstance(),
          &AnimationManager::resumeSimulation);
  connect(mpLeftDockWidget->getFlightSimGroup(),
          &FlightSimGroup::simulationReturnHome,
          &AnimationManager::getInstance(), &AnimationManager::returnToHome);
  connect(mpLeftDockWidget->getFlightSimGroup(),
          &FlightSimGroup::simulationStop, &AnimationManager::getInstance(),
          &AnimationManager::stopSimulation);
  connect(mpLeftDockWidget->getFlightSimGroup(),
          &FlightSimGroup::simulationStop, this, [this](){
      VideoManager::getInstance().stopVideoStream();
  });
  connect(mpLeftDockWidget->getEnvQueryGroup(), &EnvQueryGroup::queryEnvParams,
          &EnvManager::getInstance(), &EnvManager::generateRandomWeather);
  connect(mpRightDockWidget->getToolTreeWidget(), &ToolTreeWidget::createRoute,
          &RoutePlanner::getInstance(), &RoutePlanner::createControlPoint);
  connect(mpRightDockWidget->getToolTreeWidget(), &ToolTreeWidget::editRoute,
          &RoutePlanner::getInstance(), &RoutePlanner::editRoute);
  connect(mpRightDockWidget->getToolTreeWidget(),
          &ToolTreeWidget::simulationStart, &AnimationManager::getInstance(),
          &AnimationManager::startSimulation);
  connect(mpRightDockWidget->getToolTreeWidget(),
          &ToolTreeWidget::simulationPause, &AnimationManager::getInstance(),
          &AnimationManager::pauseSimulation);
  connect(mpRightDockWidget->getToolTreeWidget(),
          &ToolTreeWidget::simulationResume, &AnimationManager::getInstance(),
          &AnimationManager::resumeSimulation);
  connect(mpRightDockWidget->getToolTreeWidget(),
          &ToolTreeWidget::simulationReturnHome,
          &AnimationManager::getInstance(), &AnimationManager::returnToHome);
  connect(mpRightDockWidget->getToolTreeWidget(),
          &ToolTreeWidget::simulationStop, &AnimationManager::getInstance(),
          &AnimationManager::stopSimulation);
  connect(mpRightDockWidget->getToolTreeWidget(),
          &ToolTreeWidget::setFlightParams, mpLeftDockWidget->getFlightQueryGroup(),
          &FlightQueryGroup::setFlightParams);
  connect(mpRightDockWidget->getToolTreeWidget(),
          &ToolTreeWidget::queryEnvParams, &EnvManager::getInstance(),
          &EnvManager::generateRandomWeather);
  connect(mpMenuBar, &MenuBar::showUserManual, this,
          &MainWindow::showUserManual);
  connect(mpMenuBar, &MenuBar::loadModelTriggered, mpCanvas->getOpenGLWidget(),
          &OpenGLCanvas::loadModel);
  connect(mpMenuBar, &MenuBar::loadRiskTriggered, mpCanvas->getOpenGLWidget(),
          &OpenGLCanvas::loadRisk);
  connect(mpMenuBar, &MenuBar::viewMenuTriggered, mpCanvas,
          &Canvas::switchTo3D);
  connect(mpMenuBar, &MenuBar::switchTo2D, mpCanvas, &Canvas::switchTo2D);
  connect(mpMenuBar, &MenuBar::viewReset, mpCanvas, &Canvas::viewReset);
  connect(mpMenuBar, &MenuBar::simulationStart,
          &AnimationManager::getInstance(), &AnimationManager::startSimulation);
  connect(mpMenuBar, &MenuBar::simulationPause,
          &AnimationManager::getInstance(), &AnimationManager::pauseSimulation);
  connect(mpMenuBar, &MenuBar::simulationResume,
          &AnimationManager::getInstance(),
          &AnimationManager::resumeSimulation);
  connect(mpMenuBar, &MenuBar::simulationReturnHome,
          &AnimationManager::getInstance(), &AnimationManager::returnToHome);
  connect(mpMenuBar, &MenuBar::simulationStop, &AnimationManager::getInstance(),
          &AnimationManager::stopSimulation);
  connect(mpMenuBar, &MenuBar::createRoute, &RoutePlanner::getInstance(),
          &RoutePlanner::createControlPoint);
  connect(mpMenuBar, &MenuBar::setFlightParams,
          mpLeftDockWidget->getFlightQueryGroup(), &FlightQueryGroup::setFlightParams);
  connect(mpMenuBar, &MenuBar::refreshEnvironmentalParams,
          &EnvManager::getInstance(), &EnvManager::generateRandomWeather);
  connect(mpCanvas->getOpenGLWidget(), &OpenGLCanvas::submitEdit,
          &RoutePlanner::getInstance(), &RoutePlanner::createRoute);
  connect(mpCanvas->getOpenGLWidget(), &OpenGLCanvas::submitPoint,
          &RoutePlanner::getInstance(), &RoutePlanner::addControlPoint);
  connect(mpCanvas->getOpenGLWidget(), &OpenGLCanvas::setLayerContext,
          LayerTreeWidget::getInstance(), &LayerTreeWidget::setContext);
  connect(mpCanvas->getOpenGLWidget(), &OpenGLCanvas::setLayerContext,
          &RoutePlanner::getInstance(), &RoutePlanner::setContext);
  //connect(mpRightDockWidget->getJoystickWidget(), &JoyDockWidget::joystickConnected, mpCanvas->getOpenGLWidget(),&OpenGLCanvas::onJoystickConnected);
  
  // 连接风险事件路径规划信号
  connect(mpMenuBar, &MenuBar::riskEventPlannerDialogTriggered, this,
          &MainWindow::showRiskEventPlannerDialog);
  
  // 连接网格路径规划信号
  connect(mpMenuBar, &MenuBar::gridPathPlannerDialogTriggered, this,
          &MainWindow::showGridPathPlannerDialog);
}

QSize MainWindow::setWindowSize(QRect screenGeometry, int maxWidth,
                                int maxHeight, int minWidth, int minHeight) {
  // calc current screen size
  double width_d = screenGeometry.width() * 0.8;
  double height_d = screenGeometry.height() * 0.8;

  int width = qMin(maxWidth, static_cast<int>(width_d));
  int height = qMin(maxHeight, static_cast<int>(height_d));
  width = qMax(minWidth, width);
  height = qMax(minHeight, height);
  return QSize(width, height);
}

void MainWindow::initWindowStatus() {
  StyleManager::initializeStyle(); // initialize global style
  QScreen *screen = QApplication::primaryScreen();
  QRect screenGeometry = screen->geometry();
  QSize windowSize = setWindowSize(screenGeometry, 2700, 1500, 800, 600);
  setMinimumSize(windowSize);
  int x = (screenGeometry.width() - windowSize.width()) / 2,
      y = (screenGeometry.height() - windowSize.height()) / 2;
  move(x, y);

  setWindowFlags(Qt::Window);
  setWindowTitle("3D Flight Simulation");
}

void MainWindow::showUserManual() {
  logMessage("show user manual", Qgis::MessageLevel::Info);
  QDialog *manualDialog = new QDialog(this);
  manualDialog->setWindowTitle(tr("User Manual"));
  manualDialog->setWindowFlag(Qt::WindowContextHelpButtonHint, true);

  QTextEdit *textEdit = new QTextEdit(manualDialog);

  QFile manualFile("resources/user_manual.txt");
  QString content;
  if (manualFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&manualFile);
    in.setCodec("UTF-8");
    content = in.readAll();
    manualFile.close();
  } else {
    content = tr("cannot add user note");
    logMessage("Failed to load user manual file", Qgis::MessageLevel::Warning);
  }

  QFile copyrightFile("resources/copyright.txt");
  if (copyrightFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&copyrightFile);
    in.setCodec("UTF-8");
    content += "\n\n" + in.readAll();
    copyrightFile.close();
  } else {
    logMessage("cannot add developer note", Qgis::MessageLevel::Warning);
  }

  textEdit->setText(content);
  textEdit->setReadOnly(true);
  textEdit->setFixedSize(800, 600);
  textEdit->setWhatsThis(tr("If you have any questions or issues, please "
                            "contact the developer at: 18372124178."));

  QVBoxLayout *dialogLayout = new QVBoxLayout(manualDialog);
  dialogLayout->addWidget(textEdit);
  manualDialog->setLayout(dialogLayout);

  manualDialog->exec();
  logMessage("show user manual", Qgis::MessageLevel::Success);
}

void MainWindow::showRiskEventPlannerDialog() {
  logMessage("show risk event path planner dialog", Qgis::MessageLevel::Info);
  

  if (!mpRiskEventPlannerDialog) {
    mpRiskEventPlannerDialog = new RiskEventPlannerDialog(this);
    

    connect(mpRiskEventPlannerDialog, &RiskEventPlannerDialog::showResults,
            this, &MainWindow::onRiskEventPlanningResults);
  }
  

  mpRiskEventPlannerDialog->show();
  mpRiskEventPlannerDialog->raise();
  mpRiskEventPlannerDialog->activateWindow();
}

void MainWindow::onRiskEventPlanningResults(const PlanningResult& result) {
  logMessage("received risk event path planning results", Qgis::MessageLevel::Info);
  
  if (!result.success) {
    logMessage(QString("planning failed: %1").arg(result.errorMessage), 
               Qgis::MessageLevel::Critical);
    return;
  }
  

  if (mpRouteVisualization) {
    mpRouteVisualization->setPlanningResult(result);
    

    logMessage(QString("path planning results updated - total length: %1 meters, risk points count: %2")
               .arg(result.totalPathLength, 0, 'f', 2)
               .arg(result.riskEventCount), 
               Qgis::MessageLevel::Success);
  }
}

void MainWindow::showGridPathPlannerDialog() {
  logMessage("show grid path planner dialog", Qgis::MessageLevel::Info);
  

  if (!mpGridPathPlannerDialog) {
    mpGridPathPlannerDialog = new GridPathPlannerDialog(this);
    

    connect(mpGridPathPlannerDialog, &GridPathPlannerDialog::showResults,
            this, &MainWindow::onGridPathPlanningResults);
    connect(mpGridPathPlannerDialog, &GridPathPlannerDialog::showAreaResults,
            this, &MainWindow::onAreaPlanningResults);
  }
  

  mpGridPathPlannerDialog->show();
  mpGridPathPlannerDialog->raise();
  mpGridPathPlannerDialog->activateWindow();
}

void MainWindow::onGridPathPlanningResults(const GridPlanningResult& result) {
  logMessage("received grid path planning results", Qgis::MessageLevel::Info);
  
  if (!result.success) {
    logMessage(QString("grid planning failed: %1").arg(result.errorMessage), 
               Qgis::MessageLevel::Critical);
    return;
  }
  

  logMessage(QString("grid path planning results updated - total length: %1 meters, visited risk points: %2/%3")
             .arg(result.totalPathLength, 0, 'f', 2)
             .arg(result.visitedRiskPoints)
             .arg(result.totalRiskPoints), 
             Qgis::MessageLevel::Success);
             

}

void MainWindow::onAreaPlanningResults(const AreaPlanningResult& result) {
  logMessage("received area path planning results", Qgis::MessageLevel::Info);
  
  if (!result.success) {
    logMessage(QString("area planning failed: %1").arg(result.errorMessage), 
               Qgis::MessageLevel::Critical);
    return;
  }
  

  logMessage(QString("area path planning results updated - total length: %1 meters, coverage: %2% (%3/%4 nodes)")
             .arg(result.totalPathLength, 0, 'f', 2)
             .arg(result.coverageRate * 100.0, 0, 'f', 1)
             .arg(result.coveredGridCells)
             .arg(result.totalGridCells), 
             Qgis::MessageLevel::Success);
             

  logMessage(QString("area planning grid nodes: %1 total nodes").arg(result.gridNodes.size()), 
             Qgis::MessageLevel::Info);
             

  for (int i = 0; i < qMin(5, result.gridNodes.size()); ++i) {
    const GridNode& node = result.gridNodes[i];
    logMessage(QString("node %1: position(%2, %3, %4), neighbors: %5")
               .arg(node.id)
               .arg(node.position.x(), 0, 'f', 2)
               .arg(node.position.y(), 0, 'f', 2)
               .arg(node.position.z(), 0, 'f', 2)
               .arg(node.neighbors.size()), 
               Qgis::MessageLevel::Info);
  }
}
