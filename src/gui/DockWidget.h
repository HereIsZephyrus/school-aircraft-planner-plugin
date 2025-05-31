#ifndef DOCKWIDGET_H
#define DOCKWIDGET_H

#include <QDockWidget>
#include <QTreeWidgetItem>
#include <QLabel>
//#include <JoystickWidget.h>
//#include <qgamepad.h>

class LeftDockWidget : public QDockWidget {
  Q_OBJECT

public:
  LeftDockWidget(QWidget *parent = nullptr);
  ~LeftDockWidget();

private:
  QLabel *mpFlightParamsDisplay;
  QLabel *mpWeatherLabel;
  QLabel *mpTemperatureLabel;
  QLabel *mpPressureLabel;
};

class JoystickWidget : public QWidget {
  Q_OBJECT

public:
  JoystickWidget(QWidget *parent = nullptr);
  ~JoystickWidget();

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

class RightDockWidget : public QDockWidget {
  Q_OBJECT

public:
  RightDockWidget(QWidget *parent = nullptr);
  ~RightDockWidget();

private:
  JoystickWidget *mJoystickWidget;
  QTreeWidget *mpFileTreeWidget;
  void onSelectDirectoryClicked();
  void loadDirectoryFiles(const QString &path);
  void loadDirectoryLevel(QTreeWidgetItem *parentItem, const QString &path,
                          int level, int maxLevel);
  void onTreeItemExpanded(QTreeWidgetItem *item);
  void onTreeItemDoubleClicked(QTreeWidgetItem *item, int column);
  QString getItemFullPath(QTreeWidgetItem *item);
};

#endif // DOCKWIDGET_H