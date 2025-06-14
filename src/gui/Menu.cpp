#include "Menu.h"
#include "../MainWindow.h"
#include "../log/QgisDebug.h"
#include <QFileDialog>

void MenuBar::onProjectMenuTriggered() {
  QString filePath = QFileDialog::getExistingDirectory(this, tr("Open OBJ File"), "",
                                                  QFileDialog::ShowDirsOnly);
  if (!filePath.isEmpty()) {
    logMessage("project menu triggered", Qgis::MessageLevel::Info);
    emit projectMenuTriggered(filePath);
  }
}

QMenu *MenuBar::createProjectMenu(QWidget *parent) {
  QMenu *projectMenu = new QMenu(tr("Project"), parent);
  QAction *loadAction = projectMenu->addAction(tr("load 3D file"));
  connect(loadAction, &QAction::triggered, this,
          &MenuBar::onProjectMenuTriggered);
  logMessage("create project menu", Qgis::MessageLevel::Success);

  return projectMenu;
}

QMenu *MenuBar::createViewMenu(QWidget *parent) {
  QMenu *viewMenu = new QMenu(tr("View"), parent);
  QAction *p3DViewAction = viewMenu->addAction(tr("3D View"));
  QAction *p2DViewAction = viewMenu->addAction(tr("2D View"));
  QAction *pResetViewAction = viewMenu->addAction(tr("Reset View"));
  connect(p3DViewAction, &QAction::triggered, this, &MenuBar::switchTo3D);
  connect(p2DViewAction, &QAction::triggered, this, &MenuBar::switchTo2D);
  connect(pResetViewAction, &QAction::triggered, this, &MenuBar::viewReset);
  logMessage("create view menu", Qgis::MessageLevel::Success);

  return viewMenu;
}

QMenu *MenuBar::createSimulationMenu(QWidget *parent) {
  QMenu *simulationMenu = new QMenu(tr("Simulation"), parent);
  QAction *startAction = simulationMenu->addAction(tr("Start Simulation"));
  QAction *pauseAction = simulationMenu->addAction(tr("Pause Simulation"));
  QAction *resumeAction = simulationMenu->addAction(tr("Resume Simulation"));
  QAction *returnAction = simulationMenu->addAction(tr("Return Home"));
  QAction *stopAction = simulationMenu->addAction(tr("Stop Simulation"));
  connect(startAction, &QAction::triggered, this, &MenuBar::simulationStart);
  connect(pauseAction, &QAction::triggered, this, &MenuBar::simulationPause);
  connect(resumeAction, &QAction::triggered, this, &MenuBar::simulationResume);
  connect(returnAction, &QAction::triggered, this,
          &MenuBar::simulationReturnHome);
  connect(stopAction, &QAction::triggered, this, &MenuBar::simulationStop);
  logMessage("create simulation menu", Qgis::MessageLevel::Success);

  return simulationMenu;
}

QMenu *MenuBar::createRouteMenu(QWidget *parent) {
  QMenu *routeMenu = new QMenu(tr("Route Planning"), parent);
  QAction *createRouteAction = routeMenu->addAction(tr("Create route"));
  connect(createRouteAction, &QAction::triggered, this, &MenuBar::createRoute);

  logMessage("create route planning menu", Qgis::MessageLevel::Success);

  return routeMenu;
}

QMenu *MenuBar::createSettingMenu(QWidget *parent) {
  QMenu *settingMenu = new QMenu(tr("Setting"), parent);
  // parameter setting directly associated with dialog control
  QAction *flightParamsAction =
      settingMenu->addAction(tr("Flight Parameters")); // aircraft parameters
  QAction *environmentalParamsAction = settingMenu->addAction(
      tr("Environmental parameters")); // environmental parameters

  connect(flightParamsAction, &QAction::triggered, this,
          &MenuBar::setFlightParams);
  connect(environmentalParamsAction, &QAction::triggered, this,
          &MenuBar::refreshEnvironmentalParams);
  logMessage("create setting menu", Qgis::MessageLevel::Success);

  return settingMenu;
}

QMenu *MenuBar::createHelpMenu(QWidget *parent) {
  QMenu *helpMenu = new QMenu(tr("Help"), parent);
  QAction *userManualAction = helpMenu->addAction(tr("User Manual"));
  connect(userManualAction, &QAction::triggered, this,
          &MenuBar::showUserManual);

  logMessage("create help menu", Qgis::MessageLevel::Success);
  return helpMenu;
}

MenuBar::MenuBar(QWidget *parent) : QMenuBar(parent) {
  mpProjectMenu = createProjectMenu(this);
  addMenu(mpProjectMenu);
  mpViewMenu = createViewMenu(this);
  addMenu(mpViewMenu);
  mpSimulationMenu = createSimulationMenu(this);
  addMenu(mpSimulationMenu);
  mpRouteMenu = createRouteMenu(this);
  addMenu(mpRouteMenu);
  mpSettingMenu = createSettingMenu(this);
  addMenu(mpSettingMenu);
  mpHelpMenu = createHelpMenu(this);
  addMenu(mpHelpMenu);
  logMessage("create menu bar", Qgis::MessageLevel::Success);
}