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
  void createSlots();
  void initWindowStatus();
  void showUserManual();
};

#endif // MAINWINDOW_H