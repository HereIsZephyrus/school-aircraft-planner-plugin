#ifndef RIGHTDOCKWIDGET_H
#define RIGHTDOCKWIDGET_H

#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QToolButton>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QWidget>
#include <qwidget.h>
// #include <JoyDockWidget.h>
// #include <qgamepad.h>

class JoyDockWidget;
class FileTreeWidget;
class ToolTreeWidget;
class RoutePlanningToolbox;
class SimulationToolbox;
class ParameterToolbox;

class RightDockWidget : public QDockWidget {
  Q_OBJECT

public:
  RightDockWidget(QWidget *parent = nullptr);
  ~RightDockWidget();
  ToolTreeWidget* getToolTreeWidget() const { return mpToolTreeWidget; }
  FileTreeWidget* getFileTreeWidget() const { return mpFileTreeWidget; }
  JoyDockWidget* getJoystickWidget() const { return mJoystickWidget; }

private:
  QWidget *mpMainContainer;
  QVBoxLayout *mpMainLayout;
  JoyDockWidget *mJoystickWidget;
  FileTreeWidget *mpFileTreeWidget;
  ToolTreeWidget *mpToolTreeWidget;
  QScrollArea *mpScrollArea;
  QWidget *mpDockContent;
  void createScrollArea(QWidget *parent);
  void createDockContent(QWidget *parent);
};

class JoyDockWidget : public QWidget {
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
  QPushButton *mpBtnManualMode;
  QPushButton *mpBtnAutoMode;
signals:
  void joystickMove(float dx, float dy);
  void manualMode();
  void autoMode();
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

class FileTreeWidget : public QWidget {
  Q_OBJECT

public:
  FileTreeWidget(QWidget *parent = nullptr);
  ~FileTreeWidget() = default;

private:
  void createSelectDirectoryButton();
  void createTreeWidget();
  void createSlots();
  QToolButton *mpSelectDirectoryButton;
  void loadDirectoryFiles(const QString &path);
  void loadDirectoryLevel(QTreeWidgetItem *parentItem, const QString &path,
                          int level, int maxLevel);
  void onTreeItemExpanded(QTreeWidgetItem *item);
  QString getItemFullPath(QTreeWidgetItem *item);
  QVBoxLayout *mpMainLayout;
  QTreeWidgetItem *mpRootItem;
  QTreeWidget *mpTreeWidget;
private slots:
  void onSelectDirectoryClicked();
  void onTreeItemDoubleClicked(QTreeWidgetItem *item, int column);

signals:
  void loadModel(const QString &filePath);
};

class ToolTreeWidget : public QTreeWidget {
  Q_OBJECT

public:
  ToolTreeWidget(QWidget *parent = nullptr);
  ~ToolTreeWidget() = default;

private:
  QVBoxLayout *mpMainLayout;
  RoutePlanningToolbox *mpRoutePlanningToolbox;
  SimulationToolbox *mpSimulationToolbox;
  ParameterToolbox *mpParameterToolbox;
  void createSlots();

private slots:
  void onTreeItemClicked(QTreeWidgetItem *item, int column);

signals:
  void createRoute();
  void editRoute();
  void viewReset();
  void simulationStart();
  void simulationPause();
  void simulationResume();
  void simulationReturnHome();
  void simulationStop();
  void queryFlightParams();
  void queryEnvParams();
};

class RoutePlanningToolbox : public QTreeWidgetItem {
public:
  RoutePlanningToolbox(QTreeWidget *parent = nullptr);
  ~RoutePlanningToolbox() = default;
  bool isCreateRoute(const QTreeWidgetItem *item) const {
    return item == mpCreateRoute;
  }
  bool isEditRoute(const QTreeWidgetItem *item) const {
    return item == mpEditRoute;
  }

private:
  QTreeWidgetItem *mpCreateRoute;
  QTreeWidgetItem *mpEditRoute;
};

class SimulationToolbox : public QTreeWidgetItem {
public:
  SimulationToolbox(QTreeWidget *parent = nullptr);
  ~SimulationToolbox() = default;
  bool isStart(const QTreeWidgetItem *item) const { return item == mpStart; }
  bool isPause(const QTreeWidgetItem *item) const { return item == mpPause; }
  bool isResume(const QTreeWidgetItem *item) const { return item == mpResume; }
  bool isReturn(const QTreeWidgetItem *item) const { return item == mpReturn; }
  bool isStop(const QTreeWidgetItem *item) const { return item == mpStop; }

private:
  QTreeWidgetItem *mpStart;
  QTreeWidgetItem *mpPause;
  QTreeWidgetItem *mpResume;
  QTreeWidgetItem *mpReturn;
  QTreeWidgetItem *mpStop;
};

class ParameterToolbox : public QTreeWidgetItem {
public:
  ParameterToolbox(QTreeWidget *parent = nullptr);
  ~ParameterToolbox() = default;
  bool isFlightParams(const QTreeWidgetItem *item) const {
    return item == mpFlightParams;
  }
  bool isEnvironmentParams(const QTreeWidgetItem *item) const {
    return item == mpEnvironmentParams;
  }

private:
  QTreeWidgetItem *mpFlightParams;
  QTreeWidgetItem *mpEnvironmentParams;
};
#endif // RIGHTDOCKWIDGET_H