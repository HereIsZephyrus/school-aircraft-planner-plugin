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
#include <QAction>
#include <QApplication>
#include <QKeyEvent>
#include <QMainWindow>

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  static MainWindow& getInstance(QWidget *parent = nullptr) {
    static MainWindow instance(parent);
    return instance;
  }
  ~MainWindow();
  MainWindow(const MainWindow&) = delete;
  MainWindow& operator=(const MainWindow&) = delete;

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