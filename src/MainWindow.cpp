#include "MainWindow.h"
#include "core/WorkspaceState.h"
#include "gui/StyleManager.h"
#include "log/QgisDebug.h"
#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QFile>
#include <QScreen>
#include <QTextEdit>
#include <QTextStream>
#include <memory>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  logMessage("Application started", Qgis::MessageLevel::Info);
  ws::initializeWorkspaceState();
  initWindowStatus();

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

  mpMenuBar = new MenuBar(this);
  setMenuBar(mpMenuBar);
  logMessage("set menu bar", Qgis::MessageLevel::Info);

  if (!mpMenuBar) {
    logMessage("Failed to create menu bar", Qgis::MessageLevel::Critical);
    return;
  }

  createSlots();
  logMessage("create main window", Qgis::MessageLevel::Success);
}

void MainWindow::release() {
  disconnect();
  delete mpCanvas;
  delete mpLeftDockWidget;
  delete mpRightDockWidget;
  delete mpMenuBar;
  logMessage("MainWindow released", Qgis::MessageLevel::Success);
}

MainWindow::~MainWindow() {
  logMessage("MainWindow destroyed", Qgis::MessageLevel::Success);
}

void MainWindow::createSlots() {
  using namespace ws;
  connect(mpLeftDockWidget->getViewGroup(), &ViewGroup::switchTo3D, mpCanvas,
          &Canvas::switchTo3D);
  connect(mpLeftDockWidget->getViewGroup(), &ViewGroup::switchTo2D, mpCanvas,
          &Canvas::switchTo2D);
  connect(mpLeftDockWidget->getViewGroup(), &ViewGroup::viewReset, mpCanvas,
          &Canvas::viewReset);
  connect(mpLeftDockWidget->getRouteGroup(), &RouteGroup::createRoute,
          &RoutePlanner::getInstance(), &RoutePlanner::createRoute);
  connect(mpLeftDockWidget->getRouteGroup(), &RouteGroup::editRoute,
          &RoutePlanner::getInstance(), &RoutePlanner::editRoute);
  connect(mpLeftDockWidget->getFlightSimGroup(),
          &FlightSimGroup::simulationStart, &AnimationManager::getInstance(),
          &AnimationManager::startSimulation);
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
  connect(mpLeftDockWidget->getFlightQueryGroup(),
          &FlightQueryGroup::queryFlightParams, &FlightManager::getInstance(),
          &FlightManager::queryFlightParameters);
  connect(mpLeftDockWidget->getEnvQueryGroup(), &EnvQueryGroup::queryEnvParams,
          &EnvManager::getInstance(), &EnvManager::generateRandomWeather);
  connect(mpRightDockWidget->getToolTreeWidget(), &ToolTreeWidget::createRoute,
          &RoutePlanner::getInstance(), &RoutePlanner::createRoute);
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
          &ToolTreeWidget::queryFlightParams, &FlightManager::getInstance(),
          &FlightManager::queryFlightParameters);
  connect(mpRightDockWidget->getToolTreeWidget(),
          &ToolTreeWidget::queryEnvParams, &EnvManager::getInstance(),
          &EnvManager::generateRandomWeather);
  connect(mpMenuBar, &MenuBar::showUserManual, this,
          &MainWindow::showUserManual);
  connect(mpMenuBar, &MenuBar::projectMenuTriggered, mpCanvas,
          &Canvas::loadModel);
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
          &RoutePlanner::createRoute);
  connect(mpMenuBar, &MenuBar::refreshFlightParams,
          &FlightManager::getInstance(), &FlightManager::queryFlightParameters);
  connect(mpMenuBar, &MenuBar::refreshEnvironmentalParams,
          &EnvManager::getInstance(), &EnvManager::generateRandomWeather);
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