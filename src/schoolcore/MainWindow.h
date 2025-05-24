/****************************************************************************
File:MainWindow.h
Author:w
Date:2025.1.6
****************************************************************************/
#pragma once
#include "MyOpenGLWidget.h"
#include "RoutePlanner.h"
#include "qgisinterface.h"
#include "qgisplugin.h"
#include "qgsmapcanvas.h"
#include "qgsmessagelog.h"
#include "qgsvectorlayer.h"
#include "qopengl.h"
#include "ui_MainWindow.h"
#include "qgis_debug.h"
#include <QAction>
#include <QApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QMainWindow>
#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QUrl>
#include <QVBoxLayout>
#include <QWebEngineView>
#include <QWheelEvent>
#include <QtCore>
#include <QtMath>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <cmath>
#include <qdebug.h>
#include <qdockwidget.h>
#include <qfiledialog.h>
#include <qmenu.h>
#include <qrandom.h>
#include <qtreewidget.h>
#include <random>

//#include <JoystickWidget.h>
//#include <qgamepad.h>

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  void setTianDiTuMap(double lat, double lon, int zoom);
  QLabel *mpImageLabel;
private slots:
  void Unrealized();
  void switchTo3D();
  void switchTo2D();
  void open3D();
  void showUserManual();

private:
  void createMenu();
  void createMainWindow();
  void createDockWidgets();
  void createLeftDockWidget();
  void createRightDockWidget();
  void createCanvas();
  QMenuBar *mpMenuBar;
  QStackedWidget *mpStackedWidget;
  QWebEngineView *mpWebView;
  QTreeWidget *mpFileTreeWidget;
  QPushButton *mpBtnReset;
  QPushButton *mpBtnSwitchTo3D;
  QPushButton *mpBtnSwitchTo2D;
  QString mPath3D;
  QString mPathTexture;
  QList<QString> mObjPaths;
  QList<QString> mTexturePaths;
  void onSelectDirectoryClicked();
  void loadDirectoryFiles(const QString &path);
  std::unique_ptr<RoutePlanner> mpRoutePlanner;
  std::unique_ptr<MyOpenGLWidget> mpOpenGLWidget;
  void resetView();
  void initWindowStatus();

private slots:
  void queryFlightParameters();
  void refreshBasicData();

private:
  QLabel *m_pFlightParamsDisplay;
  QLabel *m_pWeatherLabel;
  QLabel *m_pTemperatureLabel;
  QLabel *m_pPressureLabel;
private slots:
  void showFlightParamsDialog();
  void showEnvironmentalParamsDialog();

private:
  struct FlightParams {
    double speed = 10.0;
    double altitude = 100.0;
    double battery = 100.0;
  };

  struct EnvironmentalParams {
    QString weather = "Sunny";
    double temperature = 25.0;
    double pressure = 1013.25;
  };

  FlightParams m_flightParams;
  EnvironmentalParams m_envParams;

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