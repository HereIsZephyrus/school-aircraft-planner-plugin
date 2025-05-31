/****************************************************************************
File:MainWindow.h
Author:w
Date:2025.1.6
****************************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "core/WorkspaceState.h"
#include "gui/Canvas.h"
#include "gui/Menu.h"
#include "gui/DockWidget.h"
#include "log/QgisDebug.h"
#include "ui_MainWindow.h"
#include <QAction>
#include <QApplication>
#include <QKeyEvent>

//#include <JoystickWidget.h>
//#include <qgamepad.h>

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  static MainWindow& getInstance(QWidget *parent = nullptr) {
    static MainWindow instance(parent);
    return instance;
  }
  ~MainWindow();

private:
  MainWindow(QWidget *parent = nullptr);
  MainWindow(const MainWindow&) = delete;
  MainWindow& operator=(const MainWindow&) = delete;

  // void setTianDiTuMap(double lat, double lon, int zoom);
  QLabel *mpImageLabel;
private slots:

private:
  void createMenu();
  void createJoyDockWidgets();
  void createLeftDockWidget();
  void createRightDockWidget();
  void createSlots();
  Canvas *mpCanvas;
  MenuBar *mpMenuBar;
  LeftDockWidget *mpLeftDockWidget;
  RightDockWidget *mpRightDockWidget;
  QTreeWidget *mpFileTreeWidget;
  QPushButton *mpBtnReset;
  QPushButton *mpBtnSwitchTo3D;
  QPushButton *mpBtnSwitchTo2D;
  void onSelectDirectoryClicked();
  void loadDirectoryFiles(const QString &path);
  void loadDirectoryLevel(QTreeWidgetItem *parentItem, const QString &path,
                          int level, int maxLevel);
  void onTreeItemExpanded(QTreeWidgetItem *item);
  void onTreeItemDoubleClicked(QTreeWidgetItem *item, int column);
  QString getItemFullPath(QTreeWidgetItem *item);
  void initWindowStatus();
  template <typename Tp> // Tp is the pointer type
  Tp safeFindChild(const QString &name) {
    Tp pWidget = this->findChild<Tp>(name);
    if (pWidget == nullptr) {
      logMessage("findChild: " + name + " not found",
                 Qgis::MessageLevel::Critical);
      return dynamic_cast<Tp>(
          ws::WindowManager::getInstance().getDefaultObject());
    }
    return pWidget;
  }

private slots:
  void queryFlightParameters();
  void refreshBasicData();
  void loadModel(const QString &objFilePath);

private:
  QLabel *m_pFlightParamsDisplay;
  QLabel *m_pWeatherLabel;
  QLabel *m_pTemperatureLabel;
  QLabel *m_pPressureLabel;
  
private slots:
  void showFlightParamsDialog();
  void showEnvironmentalParamsDialog();
  void showUserManual();
  void Unrealized();

private:
  QPushButton *m_btnManualMode;
  QPushButton *m_btnAutoMode;

  // private slots:
  //   void handleJoystickMove(float dx, float dy);
  //   void switchToManualMode();
  //   void switchToAutoMode();
  //
  // private:
  //   QGamepad *m_gamepad = nullptr;
  //   JoystickWidget *m_leftJoystick = nullptr;
  //   JoystickWidget *m_rightJoystick = nullptr;
};

#endif // MAINWINDOW_H