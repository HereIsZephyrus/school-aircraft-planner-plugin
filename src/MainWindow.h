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
#include "gui/LeftDockWidget.h"
#include "gui/RightDockWidget.h"
#include "log/QgisDebug.h"
#include <QAction>
#include <QApplication>
#include <QKeyEvent>
#include <QMainWindow>

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  static MainWindow& getInstance(QWidget *parent = nullptr) {
    if (!QApplication::instance()) {
      logMessage("QApplication must be created before MainWindow", Qgis::MessageLevel::Critical);
      throw std::runtime_error("QApplication must be created before MainWindow");
    }
    static MainWindow instance(parent);
    return instance;
  }
  ~MainWindow();
  MainWindow(const MainWindow&) = delete;
  MainWindow& operator=(const MainWindow&) = delete;
  void release();

private:
  MainWindow(QWidget *parent = nullptr);
  Canvas *mpCanvas;
  MenuBar *mpMenuBar;
  LeftDockWidget *mpLeftDockWidget;
  RightDockWidget *mpRightDockWidget;
  QSize setWindowSize(QRect screenGeometry, int maxWidth, int maxHeight, int minWidth, int minHeight);
  void initWindowStatus();
  void showUserManual();
  void createSlots();
};

#endif // MAINWINDOW_H