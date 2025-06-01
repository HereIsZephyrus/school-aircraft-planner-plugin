#include "MainWindow.h"
#include "core/WorkspaceState.h"
#include "gui/StyleManager.h"
#include "log/QgisDebug.h"
#include <QAction>
#include <QApplication>
#include <QFile>
#include <QScreen>
#include <QTextStream>
#include <memory>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  logMessage("Application started", Qgis::MessageLevel::Info);
  ws::initializeWorkspaceState();
  initWindowStatus();

  mpCanvas = new Canvas(this);
  QMainWindow::setCentralWidget(mpCanvas);
  if (!mpCanvas) {
    logMessage("Failed to create canvas", Qgis::MessageLevel::Error);
    return;
  }
  
  mpMenuBar = new MenuBar(this);
  setMenuBar(mpMenuBar);
  if (!mpMenuBar) {
    logMessage("Failed to create menu bar", Qgis::MessageLevel::Error);
    return;
  }

  mpLeftDockWidget = new LeftDockWidget(this);
  addDockWidget(Qt::LeftDockWidgetArea, mpLeftDockWidget);
  if (!mpLeftDockWidget) {
    logMessage("Failed to create left dock widget", Qgis::MessageLevel::Error);
    return;
  }

  mpRightDockWidget = new RightDockWidget(this);
  addDockWidget(Qt::RightDockWidgetArea, mpRightDockWidget);
  if (!mpRightDockWidget) {
    logMessage("Failed to create right dock widget", Qgis::MessageLevel::Error);
    return;
  }

  createSlots();
  logMessage("create main window", Qgis::MessageLevel::Success);
}

MainWindow::~MainWindow() {
  logMessage("MainWindow destroyed", Qgis::MessageLevel::Success);
}

void MainWindow::createSlots() {
  using namespace ws;
  connect(mpLeftDockWidget, &LeftDockWidget::switchTo3D, mpCanvas, &Canvas::switchTo3D);
  connect(mpLeftDockWidget, &LeftDockWidget::switchTo2D, mpCanvas, &Canvas::switchTo2D);
  connect(mpLeftDockWidget, &LeftDockWidget::viewReset, mpCanvas, &Canvas::viewReset);
  connect(mpLeftDockWidget, &LeftDockWidget::createRoute, mpCanvas, &RoutePlanner::createRoute);
  connect(mpLeftDockWidget, &LeftDockWidget::editRoute, mpCanvas, &RoutePlanner::editRoute);
  connect(mpLeftDockWidget, &LeftDockWidget::simulationStart, mpCanvas, &AnimationManager::startSimulation);
  connect(mpLeftDockWidget, &LeftDockWidget::simulationPause, mpCanvas, &AnimationManager::pauseSimulation);
  connect(mpLeftDockWidget, &LeftDockWidget::simulationResume, mpCanvas, &AnimationManager::resumeSimulation);
  connect(mpLeftDockWidget, &LeftDockWidget::simulationReturnHome, mpCanvas, &AnimationManager::returnToHome);
  connect(mpLeftDockWidget, &LeftDockWidget::simulationStop, mpCanvas, &AnimationManager::stopSimulation);
  connect(mpLeftDockWidget, &LeftDockWidget::queryFlightParams, mpCanvas, &FlightManager::queryFlightParameters);
  connect(mpLeftDockWidget, &LeftDockWidget::queryEnvParams, mpCanvas, &EnvManager::generateRandomWeather);
  connect(mpRightDockWidget, &RightDockWidget::createRoute, mpCanvas, &RoutePlanner::createRoute);
  connect(mpRightDockWidget, &RightDockWidget::editRoute, mpCanvas, &RoutePlanner::editRoute);
  connect(mpRightDockWidget, &RightDockWidget::simulationStart, mpCanvas, &AnimationManager::startSimulation);
  connect(mpRightDockWidget, &RightDockWidget::simulationPause, mpCanvas, &AnimationManager::pauseSimulation);
  connect(mpRightDockWidget, &RightDockWidget::simulationResume, mpCanvas, &AnimationManager::resumeSimulation);
  connect(mpRightDockWidget, &RightDockWidget::simulationReturnHome, mpCanvas, &AnimationManager::returnToHome);
  connect(mpRightDockWidget, &RightDockWidget::simulationStop, mpCanvas, &AnimationManager::stopSimulation);
  connect(mpRightDockWidget, &RightDockWidget::queryFlightParams, mpCanvas, &FlightManager::queryFlightParameters);
  connect(mpRightDockWidget, &RightDockWidget::queryEnvParams, mpCanvas, &EnvManager::generateRandomWeather);
  connect(mpMenuBar, &MenuBar::showUserManual, this, &MainWindow::showUserManual);
  connect(mpMenuBar, &MenuBar::projectMenuTriggered, mpCanvas, &Canvas::loadModel);
  connect(mpMenuBar, &MenuBar::viewMenuTriggered, mpCanvas, &Canvas::switchTo3D);
  connect(mpMenuBar, &MenuBar::switchTo2D, mpCanvas, &Canvas::switchTo2D);
  connect(mpMenuBar, &MenuBar::viewReset, mpCanvas, &Canvas::viewReset);
  connect(mpMenuBar, &MenuBar::simulationStart, mpCanvas, &AnimationManager::startSimulation);
  connect(mpMenuBar, &MenuBar::simulationPause, mpCanvas, &AnimationManager::pauseSimulation);
  connect(mpMenuBar, &MenuBar::simulationResume, mpCanvas, &AnimationManager::resumeSimulation);
  connect(mpMenuBar, &MenuBar::simulationReturnHome, mpCanvas, &AnimationManager::returnToHome);
  connect(mpMenuBar, &MenuBar::simulationStop, mpCanvas, &AnimationManager::stopSimulation);
  connect(mpMenuBar, &MenuBar::createRoute, mpCanvas, &RoutePlanner::createRoute);
  connect(mpMenuBar, &MenuBar::refreshFlightParams, mpCanvas, &FlightManager::queryFlightParameters);
  connect(mpMenuBar, &MenuBar::refreshEnvironmentalParams, mpCanvas, &EnvManager::generateRandomWeather);
}

QSize MainWindow::setWindowSize(QRect screenGeometry, int maxWidth, int maxHeight, int minWidth, int minHeight) {
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
    content = tr("无法加载用户手册文件。");
    logMessage("Failed to load user manual file", Qgis::MessageLevel::Warning);
  }

  QFile copyrightFile("resources/copyright.txt");
  if (copyrightFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&copyrightFile);
    in.setCodec("UTF-8");
    content += "\n\n" + in.readAll();
    copyrightFile.close();
  } else {
    logMessage("无法加载开发者信息", Qgis::MessageLevel::Warning);
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