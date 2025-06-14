#include "LeftDockWidget.h"
#include "../core/WorkspaceState.h"
#include "../log/QgisDebug.h"
#include "../opengl/Camera.h"
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

LeftDockWidget::~LeftDockWidget() {
  logMessage("LeftDockWidget destroyed", Qgis::MessageLevel::Success);
}

void LeftDockWidget::createScrollArea(QWidget *parent) {
  mpScrollArea = new QScrollArea(parent);
  mpScrollArea->setObjectName("leftDockWidgetScrollArea");
  mpScrollArea->setWidgetResizable(true);
  mpScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  mpScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setWidget(mpScrollArea);
  logMessage("create scroll area", Qgis::MessageLevel::Info);
}

void LeftDockWidget::createDockContent(QWidget *parent) {
  mpDockContent = new QWidget(parent);
  mpDockContent->setObjectName("dockContent");
  mpDockContent->setMinimumWidth(175);
  mpDockContent->setStyleSheet("background-color: black;");
  mpScrollArea->setWidget(mpDockContent);
  logMessage("create dock content", Qgis::MessageLevel::Info);
}

FunctionGroup::FunctionGroup(const QString &title, const QString &objectName, QWidget *parent)
    : QGroupBox(title, parent) {
  setObjectName(objectName);
  mpGroupLayout = new QVBoxLayout(this);
  QString groupLayoutName = objectName + "Layout";
  mpGroupLayout->setObjectName(groupLayoutName);
}

void ViewGroup::createSlots() {
  connect(mpBtnSwitchTo3D, &QPushButton::clicked, this, &ViewGroup::switchTo3D);
  connect(mpBtnSwitchTo2D, &QPushButton::clicked, this, &ViewGroup::switchTo2D);
  connect(mpBtnReset, &QPushButton::clicked, this, &ViewGroup::viewReset);
  logMessage("create view group slots", Qgis::MessageLevel::Info);
}

ViewGroup::ViewGroup(QWidget *parent)
    : FunctionGroup(tr("View Switch"), "viewGroup", parent) {
  mpGroupLayout->setObjectName("viewLayout");
  mpGroupLayout->setContentsMargins(10, 10, 10, 10);
  mpGroupLayout->setSpacing(5);

  mpBtnReset = new QPushButton(tr("Reset"), this);
  mpBtnReset->setObjectName("resetButton");
  mpBtnSwitchTo3D = new QPushButton(tr("3D View"), this);
  mpBtnSwitchTo3D->setObjectName("switchTo3DButton");
  mpBtnSwitchTo2D = new QPushButton(tr("2D Map"), this);
  mpBtnSwitchTo2D->setObjectName("switchTo2DButton");
  mpGroupLayout->addWidget(mpBtnReset);
  mpGroupLayout->addWidget(mpBtnSwitchTo3D);
  mpGroupLayout->addWidget(mpBtnSwitchTo2D);
  createSlots();

  logMessage("View switch group created", Qgis::MessageLevel::Success);
}

void RouteGroup::createSpins() {
  mpBaseHeightSpin = new QDoubleSpinBox(this);
  mpBaseHeightSpin->setObjectName("baseHeightSpin");
  mpBaseHeightSpin->setRange(-1000.0, 1000.0);
  mpBaseHeightSpin->setValue(0.0);
  mpBaseHeightSpin->setSuffix(" m");

  mpHeightSpin = new QDoubleSpinBox(this);
  mpHeightSpin->setObjectName("heightSpin");
  mpHeightSpin->setRange(1.0, 300.0);
  mpHeightSpin->setValue(75.0);
  mpHeightSpin->setSuffix(" m");

  mpWidthSpin = new QDoubleSpinBox(this);
  mpWidthSpin->setObjectName("widthSpin");
  mpWidthSpin->setRange(1.0, 300.0);
  mpWidthSpin->setValue(10.0);
  mpWidthSpin->setSuffix(" m");
  logMessage("create route group spins", Qgis::MessageLevel::Info);
}

void RouteGroup::createButtons() {
  // create button container
  mpButtonContainer = new QWidget(this);
  mpButtonContainer->setObjectName("buttonContainer");

  // create button layout
  mpButtonLayout = new QVBoxLayout(mpButtonContainer);
  mpButtonLayout->setContentsMargins(0, 0, 0, 0);
  mpButtonLayout->setSpacing(5); // set button spacing

  // create button
  mpBtnCreateRoute = new QPushButton(tr("Create Route"), mpButtonContainer);
  mpBtnCreateRoute->setObjectName("createRouteButton");
  mpBtnEditRoute = new QPushButton(tr("Edit Route"), mpButtonContainer);
  mpBtnEditRoute->setObjectName("editRouteButton");

  mpButtonLayout->addWidget(mpBtnCreateRoute);
  mpButtonLayout->addWidget(mpBtnEditRoute);
  logMessage("create button layout", Qgis::MessageLevel::Info);
}

void RouteGroup::createSlots() {
  connect(mpBtnCreateRoute, &QPushButton::clicked, this,
          &RouteGroup::createRoute);
  connect(mpBtnEditRoute, &QPushButton::clicked, this, &RouteGroup::editRoute);
  logMessage("create route group slots", Qgis::MessageLevel::Info);
}

RouteGroup::RouteGroup(QWidget *parent)
    : FunctionGroup(tr("Route Planning"), "routeGroup", parent) {
  createSpins();
  mpGroupLayout->addWidget(mpBaseHeightSpin);
  mpGroupLayout->addWidget(mpHeightSpin);
  mpGroupLayout->addWidget(mpWidthSpin);
  createButtons();
  mpGroupLayout->addWidget(mpButtonContainer);
  createSlots();

  logMessage("route planning group created", Qgis::MessageLevel::Success);
}

void FlightSimGroup::createSpins() {
  using namespace wsp;
  FlightManager &flightManager = FlightManager::getInstance();
  mpSpeedSpin = new QDoubleSpinBox(this);
  mpSpeedSpin->setObjectName("pSpeedSpin");
  mpSpeedSpin->setRange(FlightManager::minFlightSpeed,
                        FlightManager::maxFlightSpeed);
  mpSpeedSpin->setValue(flightManager.getFlightSpeed());
  mpSpeedSpin->setSuffix(" m/s");
  logMessage("create flight sim group spins", Qgis::MessageLevel::Info);
}

void FlightSimGroup::createSlots() {
  connect(mpBtnStart, &QPushButton::clicked, this,
          &FlightSimGroup::simulationStart);
  connect(mpBtnPause, &QPushButton::clicked, this,
          &FlightSimGroup::simulationPause);
  connect(mpBtnResume, &QPushButton::clicked, this,
          &FlightSimGroup::simulationResume);
  connect(mpBtnReturn, &QPushButton::clicked, this,
          &FlightSimGroup::simulationReturnHome);
  connect(mpBtnStop, &QPushButton::clicked, this,
          &FlightSimGroup::simulationStop);
  logMessage("create flight sim group slots", Qgis::MessageLevel::Info);
}

void FlightSimGroup::createButtons() {
  mpControlRow1 = new QHBoxLayout();
  mpControlRow1->setObjectName("controlRow1");
  mpBtnStart = new QPushButton("Start", this);
  mpBtnStart->setObjectName("startButton");
  mpBtnPause = new QPushButton("Pause", this);
  mpBtnPause->setObjectName("pauseButton");
  mpControlRow1->addWidget(mpBtnStart);
  mpControlRow1->addWidget(mpBtnPause);

  mpControlRow2 = new QHBoxLayout();
  mpControlRow2->setObjectName("controlRow2");
  mpBtnResume = new QPushButton("Resume", this);
  mpBtnResume->setObjectName("resumeButton");
  mpBtnStop = new QPushButton("Stop", this);
  mpBtnStop->setObjectName("stopButton");
  mpControlRow2->addWidget(mpBtnResume);
  mpControlRow2->addWidget(mpBtnStop);

  mpControlRow3 = new QHBoxLayout();
  mpControlRow3->setObjectName("controlRow3");
  mpBtnReturn = new QPushButton("Return Home", this);
  mpBtnReturn->setObjectName("returnButton");
  mpControlRow3->addWidget(mpBtnReturn);
  logMessage("create flight sim group buttons", Qgis::MessageLevel::Info);
}

FlightSimGroup::FlightSimGroup(QWidget *parent)
    : FunctionGroup(tr("Flight Simulation"), "flightSimGroup", parent) {
  createSpins();
  createButtons();

  mpSpeedLayout = new QFormLayout();
  mpSpeedLayout->addRow("Flight Speed:", mpSpeedSpin);
  mpGroupLayout->addLayout(mpSpeedLayout);
  mpGroupLayout->addLayout(mpControlRow1);
  mpGroupLayout->addLayout(mpControlRow2);
  mpGroupLayout->addLayout(mpControlRow3);
  createSlots();

  logMessage("flight simulation group created", Qgis::MessageLevel::Success);
}

void FlightQueryGroup::createDialog() {
  using namespace wsp;
  FlightManager &flightManager = FlightManager::getInstance();
  mpFlightParamsDisplay =
      new QLabel(flightManager.displayFlightParams(), this);
  mpFlightParamsDisplay->setWordWrap(true);
  mpFlightParamsDisplay->setFrameStyle(QFrame::Box);

  mpFlightParamsDialog = new QDialog(this);
  mpFlightParamsDialog->setObjectName("flightParamsDialog");
  mpFlightParamsDialog->setModal(true);
  mpFlightParamsDialog->setWindowTitle(tr("Flight Parameters Settings"));

  mpFlightParamsForm = new QFormLayout(mpFlightParamsDialog);

  mpLatSpin = new QDoubleSpinBox(mpFlightParamsDialog);
  mpLatSpin->setRange(0, 100000000);
  mpLatSpin->setDecimals(5);
  mpFlightParamsForm->addRow(tr("Latitude:"), mpLatSpin);

  mpLonSpin = new QDoubleSpinBox(mpFlightParamsDialog);
  mpLonSpin->setRange(0, 100000000);
  mpLonSpin->setDecimals(5);
  mpFlightParamsForm->addRow(tr("Longtitude:"), mpLonSpin);

  mpAltitudeSpin = new QDoubleSpinBox(mpFlightParamsDialog);
  mpAltitudeSpin->setRange(FlightManager::minFlightAltitude,
                           FlightManager::maxFlightAltitude);
  mpAltitudeSpin->setDecimals(5);
  mpFlightParamsForm->addRow(tr("Max Altitude (m):"), mpAltitudeSpin);

  mpSpeedSpin = new QDoubleSpinBox(mpFlightParamsDialog);
  mpSpeedSpin->setRange(FlightManager::minFlightSpeed,
                        FlightManager::maxFlightSpeed);
  mpFlightParamsForm->addRow(tr("Flight Speed (m/s):"), mpSpeedSpin);

  mpBatterySpin = new QDoubleSpinBox(mpFlightParamsDialog);
  mpBatterySpin->setRange(FlightManager::minFlightBattery,
                          FlightManager::maxFlightBattery);

  mpFlightParamsForm->addRow(tr("Battery Capacity (%):"), mpBatterySpin);

  mpFlightParamsButtonBox =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                           Qt::Horizontal, mpFlightParamsDialog);
  mpFlightParamsForm->addRow(mpFlightParamsButtonBox);

  mpLatSpin->setValue(flightManager.getPosition().x());
  mpLonSpin->setValue(flightManager.getPosition().y());
  mpAltitudeSpin->setValue(flightManager.getMaxAltitude());
  mpSpeedSpin->setValue(flightManager.getFlightSpeed());
  mpBatterySpin->setValue(flightManager.getFlightBattery());

  logMessage("show flight parameters dialog", Qgis::MessageLevel::Success);
}

void FlightQueryGroup::refreshFlightParams() {
  using namespace wsp;
  FlightManager &flightManager = FlightManager::getInstance();
  mpFlightParamsDisplay->setText(flightManager.displayFlightParams());
}
void FlightQueryGroup::setFlightParams(){
  wsp::FlightManager& flightManager = wsp::FlightManager::getInstance();
  mpLatSpin->setValue(flightManager.getPosition().x());
  mpLonSpin->setValue(flightManager.getPosition().y());
  mpAltitudeSpin->setValue(flightManager.getMaxAltitude());
  mpSpeedSpin->setValue(flightManager.getFlightSpeed());
  mpBatterySpin->setValue(flightManager.getFlightBattery());

  mpFlightParamsDialog->exec();
  flightManager.setFlightSpeed(mpSpeedSpin->value());
  flightManager.setFlightBattery(mpBatterySpin->value());
  QVector3D newModelPosition = wsp::WindowManager::getInstance().getModelTransform(QVector3D(mpLatSpin->value(), mpLonSpin->value(), flightManager.getPosition().z()));
  Camera& camera  = Camera::getInstance();
  if (flightManager.isManualMode())
    camera.setPosition(newModelPosition - camera.mFront * camera.mDis2Camera);
  else
    camera.setPosition(newModelPosition);
}

void FlightQueryGroup::createSlots() {
  connect(mpBtnSetParams, &QPushButton::clicked, this,
          &FlightQueryGroup::setFlightParams);
  connect(mpFlightParamsButtonBox, &QDialogButtonBox::accepted,
          mpFlightParamsDialog, &QDialog::accept);
  connect(mpFlightParamsButtonBox, &QDialogButtonBox::rejected,
          mpFlightParamsDialog, &QDialog::reject);
  connect(mpFlightParamsDialog, &QDialog::accepted, this,
          &FlightQueryGroup::refreshFlightParams);
}

void FlightQueryGroup::createButtons() {
  mpBtnSetParams = new QPushButton("Set Parameters", this);
  mpBtnSetParams->setObjectName("pBtnSetParams");
}

FlightQueryGroup::FlightQueryGroup(QWidget *parent)
    : FunctionGroup(tr("Flight Query"), "flightQueryGroup", parent) {

  createDialog();
  createButtons();
  mpGroupLayout->addWidget(mpFlightParamsDisplay);
  mpGroupLayout->addWidget(mpBtnSetParams);

  createSlots();

  logMessage("flight parameters query group created",
             Qgis::MessageLevel::Success);
}

void EnvQueryGroup::createButtons() {
  mpBtnRefreshData = new QPushButton("Refresh Data", this);
  mpBtnRefreshData->setObjectName("pBtnRefreshData");
}

void EnvQueryGroup::createDialog() {
  mpEnvParamsDialog = new QDialog(this);
  mpEnvParamsDialog->setWindowTitle(tr("Environmental Parameters Settings"));

  mpEnvParamsForm = new QFormLayout(mpEnvParamsDialog);

  wsp::EnvManager &envManager = wsp::EnvManager::getInstance();
  mpWeatherCombo = new QComboBox(mpEnvParamsDialog);
  mpWeatherCombo->addItems(envManager.weatherList);
  mpWeatherCombo->setCurrentText(envManager.getWeatherString());
  mpEnvParamsForm->addRow(tr("Weather Condition:"), mpWeatherCombo);

  mpTemperatureSpin = new QDoubleSpinBox(mpEnvParamsDialog);
  mpTemperatureSpin->setRange(wsp::EnvManager::minTemperature,
                              wsp::EnvManager::maxTemperature);
  mpTemperatureSpin->setValue(envManager.getTemperature());
  mpEnvParamsForm->addRow(tr("Temperature (°C):"), mpTemperatureSpin);

  mpPressureSpin = new QDoubleSpinBox(mpEnvParamsDialog);
  mpPressureSpin->setRange(wsp::EnvManager::minPressure,
                           wsp::EnvManager::maxPressure);
  mpPressureSpin->setValue(envManager.getPressure());
  mpEnvParamsForm->addRow(tr("Pressure (hPa):"), mpPressureSpin);

  mpEnvParamsButtonBox =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                           Qt::Horizontal, mpEnvParamsDialog);
  mpEnvParamsForm->addRow(mpEnvParamsButtonBox);

  logMessage("show environmental parameters dialog",
             Qgis::MessageLevel::Success);
}

void EnvQueryGroup::refreshEnvParams() {
  using namespace wsp;
  EnvManager &envManager = EnvManager::getInstance();
  envManager.setWeather(
      static_cast<wsp::WeatherType>(mpWeatherCombo->currentIndex()));
  envManager.setTemperature(mpTemperatureSpin->value());
  envManager.setPressure(mpPressureSpin->value());
  mpWeatherLabel->setText(
      QString("Weather: %1").arg(envManager.getWeatherString()));
  mpTemperatureLabel->setText(
      QString("Temperature: %1 C").arg(envManager.getTemperature(), 0, 'f', 1));
  mpPressureLabel->setText(
      QString("Pressure: %1 hPa").arg(envManager.getPressure(), 0, 'f', 1));
}

void EnvQueryGroup::createSlots() {
  connect(mpBtnRefreshData, &QPushButton::clicked,
          &wsp::EnvManager::getInstance(),
          &wsp::EnvManager::generateRandomWeather);
  connect(mpEnvParamsButtonBox, &QDialogButtonBox::accepted, mpEnvParamsDialog,
          &QDialog::accept);
  connect(mpEnvParamsButtonBox, &QDialogButtonBox::rejected, mpEnvParamsDialog,
          &QDialog::reject);
  connect(mpEnvParamsDialog, &QDialog::accepted, this,
          &EnvQueryGroup::refreshEnvParams);
}

EnvQueryGroup::EnvQueryGroup(QWidget *parent)
    : FunctionGroup(tr("Environmental Data"), "basicDataGroup", parent) {
  setObjectName("basicDataGroup");

  wsp::EnvManager &envManager = wsp::EnvManager::getInstance();
  mpWeatherLabel = new QLabel(
      QString("Weather: %1").arg(envManager.getWeatherString()), this);
  mpTemperatureLabel = new QLabel(
      QString("Temperature: %1 C").arg(envManager.getTemperature(), 0, 'f', 1),
      this);
  mpPressureLabel = new QLabel(
      QString("Pressure: %1 hPa").arg(envManager.getPressure(), 0, 'f', 1),
      this);
  createButtons();

  mpGroupLayout->addWidget(mpWeatherLabel);
  mpGroupLayout->addWidget(mpTemperatureLabel);
  mpGroupLayout->addWidget(mpPressureLabel);
  mpGroupLayout->addWidget(mpBtnRefreshData);

  createDialog();
  createSlots();

  logMessage("basic data group created", Qgis::MessageLevel::Success);
}

LeftDockWidget::LeftDockWidget(QWidget *parent) : QDockWidget(parent) {
  setObjectName("leftDockWidget");
  setAllowedAreas(Qt::LeftDockWidgetArea);
  setFeatures(QDockWidget::DockWidgetMovable |
              QDockWidget::DockWidgetFloatable);
  setMinimumWidth(200);

  createScrollArea(this);
  createDockContent(mpScrollArea);
  mpMainLayout = new QVBoxLayout(mpDockContent);
  mpMainLayout->setContentsMargins(0, 0, 0, 0);
  mpMainLayout->setSpacing(0);
  mpViewGroup = new ViewGroup(mpDockContent);
  mpMainLayout->addWidget(mpViewGroup);
  mpRouteGroup = new RouteGroup(mpDockContent);
  mpMainLayout->addWidget(mpRouteGroup);
  mpFlightSimGroup = new FlightSimGroup(mpDockContent);
  mpMainLayout->addWidget(mpFlightSimGroup);
  mpMainLayout->addStretch();
  logMessage("Add stretch", Qgis::MessageLevel::Info);
  mpFlightQueryGroup = new FlightQueryGroup(mpDockContent);
  mpMainLayout->addWidget(mpFlightQueryGroup);
  mpEnvQueryGroup = new EnvQueryGroup(mpDockContent);
  mpMainLayout->addWidget(mpEnvQueryGroup);
  logMessage("left dock widget created", Qgis::MessageLevel::Success);
}