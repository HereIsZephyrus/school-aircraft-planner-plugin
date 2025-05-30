#ifndef MENU_H
#define MENU_H

#include <qmenubar.h>
#include <memory>

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
};
#endif