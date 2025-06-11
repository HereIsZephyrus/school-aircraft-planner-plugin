#ifndef LEFTDOCKWIDGET_H
#define LEFTDOCKWIDGET_H

#include <QComboBox>
#include <QDialogButtonBox>
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
#include <qobjectdefs.h>

class ViewGroup;
class RouteGroup;
class FlightSimGroup;
class FlightQueryGroup;
class EnvQueryGroup;

class LeftDockWidget : public QDockWidget {
  Q_OBJECT

public:
  LeftDockWidget(QWidget *parent = nullptr);
  ~LeftDockWidget();

  RouteGroup *getRouteGroup() const { return mpRouteGroup; }
  ViewGroup *getViewGroup() const { return mpViewGroup; }
  FlightSimGroup *getFlightSimGroup() const { return mpFlightSimGroup; }
  FlightQueryGroup *getFlightQueryGroup() const { return mpFlightQueryGroup; }
  EnvQueryGroup *getEnvQueryGroup() const { return mpEnvQueryGroup; }

private:
  QVBoxLayout *mpMainLayout;
  QScrollArea *mpScrollArea;
  QWidget *mpDockContent;
  ViewGroup *mpViewGroup;
  RouteGroup *mpRouteGroup;
  FlightSimGroup *mpFlightSimGroup;
  FlightQueryGroup *mpFlightQueryGroup;
  EnvQueryGroup *mpEnvQueryGroup;
  void createScrollArea(QWidget *parent);
  void createDockContent(QWidget *parent);
};

class FunctionGroup : public QGroupBox {
  Q_OBJECT

public:
  FunctionGroup(const QString &title, const QString &objectName, QWidget *parent = nullptr);
  ~FunctionGroup() = default;

protected:
  QVBoxLayout *mpGroupLayout;
  virtual void createSlots() {}
  virtual void createButtons() {}
};

class ViewGroup : public FunctionGroup {
  Q_OBJECT

public:
  ViewGroup(QWidget *parent = nullptr);
  void createSlots() override;
  ~ViewGroup() = default;

private:
  QPushButton *mpBtnReset;
  QPushButton *mpBtnSwitchTo3D;
  QPushButton *mpBtnSwitchTo2D;

signals:
  void switchTo3D();
  void switchTo2D();
  void viewReset();
};

class RouteGroup : public FunctionGroup {
  Q_OBJECT

public:
  RouteGroup(QWidget *parent = nullptr);
  ~RouteGroup() = default;

private:
  void createSpins();
  void createSlots() override;
  void createButtons() override;
  QVBoxLayout *mpButtonLayout;
  QWidget *mpButtonContainer;
  QDoubleSpinBox *mpBaseHeightSpin;
  QDoubleSpinBox *mpHeightSpin;
  QDoubleSpinBox *mpWidthSpin;
  QPushButton *mpBtnCreateRoute;
  QPushButton *mpBtnEditRoute;

signals:
  void createRoute();
  void editRoute();
};

class FlightSimGroup : public FunctionGroup {
  Q_OBJECT

public:
  FlightSimGroup(QWidget *parent = nullptr);
  ~FlightSimGroup() = default;

private:
  void createSpins();
  void createSlots() override;
  void createButtons() override;
  QHBoxLayout *mpControlRow1;
  QHBoxLayout *mpControlRow2;
  QHBoxLayout *mpControlRow3;
  QFormLayout *mpSpeedLayout;
  QDoubleSpinBox *mpSpeedSpin;
  QPushButton *mpBtnStart;
  QPushButton *mpBtnPause;
  QPushButton *mpBtnResume;
  QPushButton *mpBtnReturn;
  QPushButton *mpBtnStop;

signals:
  void simulationStart();
  void simulationPause();
  void simulationResume();
  void simulationReturnHome();
  void simulationStop();
};

class FlightQueryGroup : public FunctionGroup {
  Q_OBJECT

public:
  FlightQueryGroup(QWidget *parent = nullptr);
  ~FlightQueryGroup() = default;

private:
  void createSlots() override;
  void createButtons() override;
  void createDialog();
  QPushButton *mpBtnQueryParams;
  QLabel *mpFlightParamsDisplay;
  QDialog *mpFlightParamsDialog;
  QFormLayout *mpFlightParamsForm;
  QDoubleSpinBox *mpSpeedSpin;
  QDoubleSpinBox *mpAltitudeSpin;
  QDoubleSpinBox *mpBatterySpin;
  QDialogButtonBox *mpFlightParamsButtonBox;

public slots:
  void refreshFlightParams();

signals:
  void queryFlightParams();
};

class EnvQueryGroup : public FunctionGroup {
  Q_OBJECT

public:
  EnvQueryGroup(QWidget *parent = nullptr);
  ~EnvQueryGroup() = default;

private:
  void createButtons() override;
  void createSlots() override;
  void createDialog();
  void refreshEnvParams();
  QPushButton *mpBtnRefreshData;
  QLabel *mpWeatherLabel;
  QLabel *mpTemperatureLabel;
  QLabel *mpPressureLabel;
  QDialog *mpEnvParamsDialog;
  QFormLayout *mpEnvParamsForm;
  QDialogButtonBox *mpEnvParamsButtonBox;
  QComboBox *mpWeatherCombo;
  QDoubleSpinBox *mpTemperatureSpin;
  QDoubleSpinBox *mpPressureSpin;

signals:
  void queryEnvParams();
};

#endif // LEFTDOCKWIDGET_H