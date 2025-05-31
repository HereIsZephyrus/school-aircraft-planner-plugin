#include "Menu.h"
#include "../MainWindow.h"
#include "../log/QgisDebug.h"
#include <QFileDialog>

void MenuBar::onProjectMenuTriggered() {
  QString filePath = QFileDialog::getOpenFileName(this, tr("Open OBJ File"), "",
                                                  tr("OBJ Files (*.obj)"));
  if (!filePath.isEmpty()) {
    logMessage("project menu triggered", Qgis::MessageLevel::Info);
    emit projectMenuTriggered(filePath);
  }
}

QMenu* MenuBar::createProjectMenu(QWidget *parent) {
  QMenu* projectMenu = new QMenu(tr("Project"), parent);
  QAction *loadAction = projectMenu->addAction(tr("load 3D file"));
  connect(loadAction, &QAction::triggered, this, &MenuBar::onProjectMenuTriggered);
  logMessage("create project menu", Qgis::MessageLevel::Success);

  return projectMenu;
}

QMenu* MenuBar::createViewMenu(QWidget *parent) {
  QMenu* viewMenu = new QMenu(tr("View"), parent);
  QAction *p3DViewAction = viewMenu->addAction(tr("3D View"));
  QAction *p2DViewAction = viewMenu->addAction(tr("2D View"));
  QAction *pResetViewAction = viewMenu->addAction(tr("Reset View"));
  connect(p3DViewAction, &QAction::triggered, this,
          [this]() { emit viewChanged(ws::CanvasType::ThreeD); });
  connect(p2DViewAction, &QAction::triggered, this,
          [this]() { emit viewChanged(ws::CanvasType::TwoD); });
  connect(pResetViewAction, &QAction::triggered, this,
          [this]() { emit viewReset(); });
  logMessage("create view menu", Qgis::MessageLevel::Success);

  return viewMenu;
}

QMenu* MenuBar::createSimulationMenu(QWidget *parent) {
  QMenu* simulationMenu = new QMenu(tr("Simulation"), parent);
  QAction *startAction = simulationMenu->addAction(tr("Start Simulation"));
  QAction *pauseAction = simulationMenu->addAction(tr("Pause Simulation"));
  QAction *resumeAction = simulationMenu->addAction(tr("Resume Simulation"));
  QAction *returnAction = simulationMenu->addAction(tr("Return Home"));
  QAction *stopAction = simulationMenu->addAction(tr("Stop Simulation"));
  connect(startAction, &QAction::triggered, this,
          [this]() { emit simulationStart(); });
  connect(pauseAction, &QAction::triggered, this,
          [this]() { emit simulationPause(); });
  connect(resumeAction, &QAction::triggered, this,
          [this]() { emit simulationResume(); });
  connect(returnAction, &QAction::triggered, this,
          [this]() { emit simulationReturnHome(); });
  connect(stopAction, &QAction::triggered, this,
          [this]() { emit simulationStop(); });
  logMessage("create simulation menu", Qgis::MessageLevel::Success);

  return simulationMenu;
}

QMenu* MenuBar::createRouteMenu(QWidget *parent) {
  QMenu* routeMenu = new QMenu(tr("Route Planning"), parent);
  QAction *createRouteAction = routeMenu->addAction(tr("Create route"));
  connect(createRouteAction, &QAction::triggered, this,
          [this]() { emit createRoute(); });

  logMessage("create route planning menu", Qgis::MessageLevel::Success);

  return routeMenu;
}

QMenu* MenuBar::createSettingMenu(QWidget *parent) {
  QMenu* settingMenu = new QMenu(tr("Setting"), parent);
  // parameter setting directly associated with dialog control
  QAction *flightParamsAction = settingMenu->addAction(tr("Flight Parameters")); // aircraft parameters
  QAction *environmentalParamsAction = settingMenu->addAction(tr("Environmental parameters")); // environmental parameters

  MainWindow &mainWindow = MainWindow::getInstance();
  connect(flightParamsAction, &QAction::triggered, this,
          [this]() { emit showFlightParamsDialog(); });
  connect(environmentalParamsAction, &QAction::triggered, this,
          [this]() { emit showEnvironmentalParamsDialog(); });
  logMessage("create setting menu", Qgis::MessageLevel::Success);

  return settingMenu;
}

QMenu* MenuBar::createHelpMenu(QWidget *parent) {
  QMenu* helpMenu = new QMenu(tr("Help"), parent);
  MainWindow &mainWindow = MainWindow::getInstance();
  QAction *userManualAction = helpMenu->addAction(tr("User Manual"));
  connect(userManualAction, &QAction::triggered, this,
          [this]() { emit showUserManual(); });

  logMessage("create help menu", Qgis::MessageLevel::Success);
  return helpMenu;
}

MenuBar::MenuBar(QWidget *parent) : QMenuBar(parent) {
  mpProjectMenu = createProjectMenu(this);
  mpViewMenu = createViewMenu(this);
  mpSimulationMenu = createSimulationMenu(this);
  mpRouteMenu = createRouteMenu(this);
  mpSettingMenu = createSettingMenu(this);
  mpHelpMenu = createHelpMenu(this);
  logMessage("create menu bar", Qgis::MessageLevel::Success);
}