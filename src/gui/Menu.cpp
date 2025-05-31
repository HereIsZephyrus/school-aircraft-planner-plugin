#include "Menu.h"
#include "../MainWindow.h"
#include "../log/QgisDebug.h"
#include <QFileDialog>
using pMenu = std::shared_ptr<QMenu>;

void MenuBar::onProjectMenuTriggered() {
  QString filePath = QFileDialog::getOpenFileName(this, tr("Open OBJ File"), "",
                                                  tr("OBJ Files (*.obj)"));
  if (!filePath.isEmpty()) {
    logMessage("project menu triggered", Qgis::MessageLevel::Info);
    emit projectMenuTriggered(filePath);
  }
}

pMenu MenuBar::createProjectMenu(QWidget *parent) {
  pMenu pProjectMenu = std::make_shared<QMenu>(tr("Project"), parent);
  QAction *loadAction = pProjectMenu->addAction(tr("load 3D file"));
  connect(loadAction, &QAction::triggered, this, &MenuBar::onProjectMenuTriggered);
  logMessage("create project menu", Qgis::MessageLevel::Success);

  return pProjectMenu;
}

pMenu MenuBar::createViewMenu(QWidget *parent) {
  pMenu pViewMenu = std::make_shared<QMenu>(tr("View"), parent);
  QAction *p3DViewAction = pViewMenu->addAction(tr("3D View"));
  QAction *p2DViewAction = pViewMenu->addAction(tr("2D View"));
  QAction *pResetViewAction = pViewMenu->addAction(tr("Reset View"));
  connect(p3DViewAction, &QAction::triggered, this,
          [this]() { emit viewChanged(ws::CanvasType::ThreeD); });
  connect(p2DViewAction, &QAction::triggered, this,
          [this]() { emit viewChanged(ws::CanvasType::TwoD); });
  connect(pResetViewAction, &QAction::triggered, this,
          [this]() { emit viewReset(); });
  logMessage("create view menu", Qgis::MessageLevel::Success);

  return pViewMenu;
}

pMenu MenuBar::createSimulationMenu(QWidget *parent) {
  pMenu pSimulationMenu = std::make_shared<QMenu>(tr("Simulation"), parent);
  QAction *startAction = pSimulationMenu->addAction(tr("Start Simulation"));
  QAction *pauseAction = pSimulationMenu->addAction(tr("Pause Simulation"));
  QAction *resumeAction = pSimulationMenu->addAction(tr("Resume Simulation"));
  QAction *returnAction = pSimulationMenu->addAction(tr("Return Home"));
  QAction *stopAction = pSimulationMenu->addAction(tr("Stop Simulation"));
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

  return pSimulationMenu;
}

pMenu MenuBar::createRouteMenu(QWidget *parent) {
  pMenu pRouteMenu = std::make_shared<QMenu>(tr("Route Planning"), parent);
  QAction *pCreateRouteAction = pRouteMenu->addAction(tr("Create route"));
  connect(pCreateRouteAction, &QAction::triggered, this,
          [this]() { emit createRoute(); });

  logMessage("create route planning menu", Qgis::MessageLevel::Success);

  return pRouteMenu;
}

pMenu MenuBar::createSettingMenu(QWidget *parent) {
  pMenu pSettingMenu = std::make_shared<QMenu>(tr("Setting"), parent);
  // parameter setting directly associated with dialog control
  QAction *pFlightParamsAction = pSettingMenu->addAction(tr("Flight Parameters")); // aircraft parameters
  QAction *pEnvironmentalParamsAction = pSettingMenu->addAction(tr("Environmental parameters")); // environmental parameters

  MainWindow &mainWindow = MainWindow::getInstance();
  connect(pFlightParamsAction, &QAction::triggered, this,
          [this]() { emit showFlightParamsDialog(); });
  connect(pEnvironmentalParamsAction, &QAction::triggered, this,
          [this]() { emit showEnvironmentalParamsDialog(); });
  logMessage("create setting menu", Qgis::MessageLevel::Success);

  return pSettingMenu;
}

pMenu MenuBar::createHelpMenu(QWidget *parent) {
  pMenu pHelpMenu = std::make_shared<QMenu>(tr("Help"), parent);
  MainWindow &mainWindow = MainWindow::getInstance();
  QAction *pUserManualAction = pHelpMenu->addAction(tr("User Manual"));
  connect(pUserManualAction, &QAction::triggered, this,
          [this]() { emit showUserManual(); });

  logMessage("create help menu", Qgis::MessageLevel::Success);
  return pHelpMenu;
}

MenuBar::MenuBar(QWidget *parent) : QMenuBar(parent) {
  logMessage("create menu bar", Qgis::MessageLevel::Info);

  mpProjectMenu = createProjectMenu(this);
  mpViewMenu = createViewMenu(this);
  mpSimulationMenu = createSimulationMenu(this);
  mpRouteMenu = createRouteMenu(this);
  mpSettingMenu = createSettingMenu(this);
  mpHelpMenu = createHelpMenu(this);
}