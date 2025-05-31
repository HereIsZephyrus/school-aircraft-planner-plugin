#ifndef MENU_H
#define MENU_H

#include <qmenubar.h>
#include <memory>
#include "../core/WorkspaceState.h"

class MenuBar : public QMenuBar{
    using pMenu = std::shared_ptr<QMenu>;
public:
    MenuBar(QWidget *parent = nullptr);
 
private:
    pMenu createProjectMenu(QWidget *parent);
    pMenu createViewMenu(QWidget *parent);
    pMenu createSimulationMenu(QWidget *parent);
    pMenu createRouteMenu(QWidget *parent);
    pMenu createSettingMenu(QWidget *parent);
    pMenu createHelpMenu(QWidget *parent);
    pMenu mpProjectMenu;
    pMenu mpViewMenu;
    pMenu mpSimulationMenu;
    pMenu mpRouteMenu;
    pMenu mpSettingMenu;
    pMenu mpHelpMenu;

private slots:
    void onProjectMenuTriggered();

signals:
    void projectMenuTriggered(const QString &filePath);
    void viewMenuTriggered();
    void simulationMenuTriggered();
    void routeMenuTriggered();
    void settingMenuTriggered();
    void helpMenuTriggered();
    void viewChanged(ws::CanvasType viewType);
    void viewReset();
    void simulationStart();
    void simulationPause();
    void simulationResume();
    void simulationReturnHome();
    void simulationStop();
    void createRoute();
    void showFlightParamsDialog();
    void showEnvironmentalParamsDialog();
    void showUserManual();
};
#endif