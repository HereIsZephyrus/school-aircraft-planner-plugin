#ifndef MENU_H
#define MENU_H

#include "../core/WorkspaceState.h"
#include <qmenubar.h>

class MenuBar : public QMenuBar {
  Q_OBJECT

public:
  MenuBar(QWidget *parent = nullptr);

private:
  QMenu *createProjectMenu(QWidget *parent);
  QMenu *createViewMenu(QWidget *parent);
  QMenu *createSimulationMenu(QWidget *parent);
  QMenu *createRouteMenu(QWidget *parent);
  QMenu *createSettingMenu(QWidget *parent);
  QMenu *createHelpMenu(QWidget *parent);
  QMenu *mpProjectMenu;
  QMenu *mpViewMenu;
  QMenu *mpSimulationMenu;
  QMenu *mpRouteMenu;
  QMenu *mpSettingMenu;
  QMenu *mpHelpMenu;

private slots:
  void onProjectMenuTriggered();

signals:
  void projectMenuTriggered(const QString &filePath);
  void viewMenuTriggered();
  void simulationMenuTriggered();
  void routeMenuTriggered();
  void settingMenuTriggered();
  void helpMenuTriggered();
  void switchTo3D();
  void switchTo2D();
  void viewReset();
  void simulationStart();
  void simulationPause();
  void simulationResume();
  void simulationReturnHome();
  void simulationStop();
  void createRoute();
  void refreshFlightParams();
  void refreshEnvironmentalParams();
  void showUserManual();
};
#endif