#ifndef RIGHTDOCKWIDGET_H
#define RIGHTDOCKWIDGET_H

#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QWidget>
#include <QToolButton>
#include <qwidget.h>
// #include <JoyDockWidget.h>
// #include <qgamepad.h>

class JoyDockWidget;
class FileTreeWidget;
class RoutePlanningToolbox;
class SimulationToolbox;
class ParameterToolbox;

class RightDockWidget : public QDockWidget{
    Q_OBJECT

public:
    RightDockWidget(QWidget *parent = nullptr);
    ~RightDockWidget() = default;

private:
    QWidget *mpMainContainer;
    JoyDockWidget *mJoystickWidget;
    FileTreeWidget *mpFileTreeWidget;
    ToolTreeWidget *mpToolTreeWidget;
    void onTreeItemClicked(QTreeWidgetItem *item, int column);

signals:
    void createRouteClicked();
    void editRouteClicked();
    void startClicked();
    void pauseClicked();
    void resumeClicked();
    void returnClicked();
    void stopClicked();
    void flightParamsClicked();
    void environmentParamsClicked();
};

class JoyDockWidget : public QWidget{
    Q_OBJECT

public:
    JoyDockWidget(QWidget *parent = nullptr);
    ~JoyDockWidget() = default;

private:
    QVBoxLayout *mpMainLayout;
    QDockWidget *mpControlDock;
    QWidget *mpControlPanel;
    QHBoxLayout *mpJoystickLayout;
    QHBoxLayout *mpButtonLayout;
    QPushButton *mpManualBtn;
    QPushButton *mpAutoBtn;
private slots:
    void handleJoystickMove(float dx, float dy);
    void switchToManualMode();
    void switchToAutoMode();
    //
    // private:
    //   QGamepad *m_gamepad = nullptr;
    //   JoystickWidget *m_leftJoystick = nullptr;
    //   JoystickWidget *m_rightJoystick = nullptr;
};

class FileTreeWidget : public QTreeWidget{
    Q_OBJECT

public:
    FileTreeWidget(QWidget *parent = nullptr);
    ~FileTreeWidget() = default;

private:
    void createSelectDirectoryButton();
    void createSlots();
    QToolButton *mpSelectDirectoryButton;
    void loadDirectoryFiles(const QString &path);
    void loadDirectoryLevel(QTreeWidgetItem *parentItem, const QString &path,
                            int level, int maxLevel);
    void onTreeItemExpanded(QTreeWidgetItem *item);
    void onTreeItemDoubleClicked(QTreeWidgetItem *item, int column);
    QString getItemFullPath(QTreeWidgetItem *item);
    QVBoxLayout *mpMainLayout;
    QTreeWidgetItem *mpRootItem;
private slots:
    void onSelectDirectoryClicked();
    void onTreeItemDoubleClicked(QTreeWidgetItem *item, int column);
};

class ToolTreeWidget : public QTreeWidget{
    Q_OBJECT

public:
    ToolTreeWidget(QWidget *parent = nullptr);
    ~ToolTreeWidget() = default;

private:
    void createToolTree();
    QVBoxLayout *mpMainLayout;
    RoutePlanningToolbox *mpRoutePlanningToolbox;
    SimulationToolbox *mpSimulationToolbox;
    ParameterToolbox *mpParameterToolbox;
    void createSlots();
};

class RoutePlanningToolbox : public QTreeWidgetItem{
    Q_OBJECT

public:
    RoutePlanningToolbox(QTreeWidget *parent = nullptr);
    ~RoutePlanningToolbox() = default;
    bool isCreateRoute(const QTreeWidgetItem *item) const{return item == mpCreateRoute;}
    bool isEditRoute(const QTreeWidgetItem *item) const{return item == mpEditRoute;}

private:
    QTreeWidgetItem *mpCreateRoute;
    QTreeWidgetItem *mpEditRoute;
};

class SimulationToolbox : public QTreeWidgetItem{
    Q_OBJECT

public:
    SimulationToolbox(QTreeWidget *parent = nullptr);
    ~SimulationToolbox() = default;
    bool isStart(const QTreeWidgetItem *item) const{return item == mpStart;}
    bool isPause(const QTreeWidgetItem *item) const{return item == mpPause;}
    bool isResume(const QTreeWidgetItem *item) const{return item == mpResume;}
    bool isReturn(const QTreeWidgetItem *item) const{return item == mpReturn;}
    bool isStop(const QTreeWidgetItem *item) const{return item == mpStop;}

private:
    QTreeWidgetItem *mpStart;
    QTreeWidgetItem *mpPause;
    QTreeWidgetItem *mpResume;
    QTreeWidgetItem *mpReturn;
    QTreeWidgetItem *mpStop;
};

class ParameterToolbox : public QTreeWidgetItem{
    Q_OBJECT

public:
    ParameterToolbox(QTreeWidget *parent = nullptr);
    ~ParameterToolbox() = default;
    bool isFlightParams(const QTreeWidgetItem *item) const{return item == mpFlightParams;}
    bool isEnvironmentParams(const QTreeWidgetItem *item) const{return item == mpEnvironmentParams;}

private:
    QTreeWidgetItem *mpFlightParams;
    QTreeWidgetItem *mpEnvironmentParams;
};
#endif // RIGHTDOCKWIDGET_H