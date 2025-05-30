#include "Menu.h"
#include "../log/QgisDebug.h"
#include "Canvas.h"
#include "DockWidget.h"
#include "../MainWindow.h"
#include <QFileDialog>
#include "../core/WorkspaceState.h"
using pMenu = std::shared_ptr<QMenu>;

pMenu MenuBar::createProjectMenu(QWidget *parent) {
  pMenu pProjectMenu = std::make_shared<QMenu>(tr("Project"), parent);
  QAction *loadAction = pProjectMenu->addAction(tr("load 3D file"));
  connect(loadAction, &QAction::triggered, this, [this]() {
    QString filePath = QFileDialog::getOpenFileName(
        this, tr("Open OBJ File"), "", tr("OBJ Files (*.obj)"));
    // if (filePath.isEmpty())
    //   logMessage("no file selected", Qgis::MessageLevel::Critical);
    // else
    //   loadModel(filePath);
  });
  logMessage("create project menu", Qgis::MessageLevel::Success);

  return pProjectMenu;
}

pMenu MenuBar::createViewMenu(QWidget *parent) {
  pMenu pViewMenu = std::make_shared<QMenu>(tr("View"), parent);
  pViewMenu->addAction(tr("3D View"), this, &Canvas::switchTo3D);
  pViewMenu->addAction(tr("2D View"), this, &Canvas::switchTo2D);
  pViewMenu->addAction(tr("Reset View"), this, &Canvas::resetView);
  logMessage("create view menu", Qgis::MessageLevel::Success);

  return pViewMenu;
}

pMenu MenuBar::createSimulationMenu(QWidget *parent) {
  pMenu pSimulationMenu = std::make_shared<QMenu>(tr("Simulation"), parent);
  if (ws::WindowManager::getInstance().getCurrentCanvas() == ws::CanvasType::ThreeD) {
    QAction *startAction = pSimulationMenu->addAction(tr("Start Simulation"));
    QAction *pauseAction = pSimulationMenu->addAction(tr("Pause Simulation"));
    QAction *resumeAction = pSimulationMenu->addAction(tr("Resume Simulation"));
    QAction *returnAction = pSimulationMenu->addAction(tr("Return Home"));
    QAction *stopAction = pSimulationMenu->addAction(tr("Stop Simulation"));
    /*
    connect(startAction, &QAction::triggered, mpOpenGLWidget.get(),
            &OpenGLCanvas::startSimulation);
    connect(pauseAction, &QAction::triggered, mpOpenGLWidget.get(),
            &OpenGLCanvas::pauseSimulation);
    connect(resumeAction, &QAction::triggered, mpOpenGLWidget.get(),
            &OpenGLCanvas::resumeSimulation);
    connect(returnAction, &QAction::triggered, mpOpenGLWidget.get(),
            &OpenGLCanvas::returnToHome);
    connect(stopAction, &QAction::triggered, mpOpenGLWidget.get(),
            &OpenGLCanvas::stopSimulation);
*/
  } else {
    logMessage("OpenGLWidget not initialized", Qgis::MessageLevel::Critical);
  }
  logMessage("create simulation menu", Qgis::MessageLevel::Success);

  return pSimulationMenu;
}

pMenu MenuBar::createRouteMenu(QWidget *parent) {
  pMenu pRouteMenu = std::make_shared<QMenu>(tr("Route Planning"), parent);
  QAction *pCreateRouteAction = pRouteMenu->addAction(tr("Create route"));

/*
  if (mpRoutePlanner) {
    connect(pCreateRouteAction, &QAction::triggered, mpRoutePlanner.get(),
            &RoutePlanner::enterRoutePlanningMode);
  } else {
    logMessage("RoutePlanner not initialized", Qgis::MessageLevel::Critical);
  }
*/
  logMessage("create route planning menu", Qgis::MessageLevel::Success);

  return pRouteMenu;
}

pMenu MenuBar::createSettingMenu(QWidget *parent) {
  pMenu pSettingMenu = std::make_shared<QMenu>(tr("Setting"), parent);
  // parameter setting directly associated with dialog control
  QAction *pFlightParamsAction = pSettingMenu->addAction(tr("Flight Parameters")); // aircraft parameters
  QAction *pEnvironmentalParamsAction = pSettingMenu->addAction(tr("Environmental parameters")); // environmental parameters

  MainWindow& mainWindow = MainWindow::getInstance();
  connect(pFlightParamsAction, &QAction::triggered, &mainWindow,
          &MainWindow::showFlightParamsDialog);
  logMessage(
      "connect flight parameters action to show flight parameters dialog",
      Qgis::MessageLevel::Info);
  connect(pEnvironmentalParamsAction, &QAction::triggered, &mainWindow,
          &MainWindow::showEnvironmentalParamsDialog);
  logMessage("connect environmental parameters action to show environmental "
             "parameters dialog",
             Qgis::MessageLevel::Info);
  logMessage("create setting menu", Qgis::MessageLevel::Success);

  return pSettingMenu;
}

pMenu MenuBar::createHelpMenu(QWidget *parent) {
  pMenu pHelpMenu = std::make_shared<QMenu>(tr("Help"), parent);

  MainWindow& mainWindow = MainWindow::getInstance();
  pHelpMenu->addAction(tr("User Manual"), &mainWindow,&MainWindow::showUserManual);

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