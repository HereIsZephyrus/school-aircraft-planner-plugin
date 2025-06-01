#include "../log/QgisDebug.h"
#include "../core/WorkspaceState.h"
#include "LeftDockWidget.h"

void LeftDockWidget::createScrollArea(QWidget* parent) {
    mpScrollArea = new QScrollArea(parent);
    mpScrollArea->setObjectName("leftDockWidgetScrollArea");
    mpScrollArea->setWidgetResizable(true);
    mpScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mpScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mpMainLayout->addWidget(mpScrollArea);
}

void LeftDockWidget::createDockContent(QWidget* parent) {
    mpDockContent = new QWidget(parent);
    mpDockContent->setObjectName("dockContent");
    mpDockContent->setMinimumWidth(175);
    mpMainLayout->addWidget(mpDockContent);
}

void ViewGroup::createSlots() {
    connect(mpBtnSwitchTo3D, &QPushButton::clicked, this, &ViewGroup::switchTo3D);
    connect(mpBtnSwitchTo2D, &QPushButton::clicked, this, &ViewGroup::switchTo2D);
    connect(mpBtnReset, &QPushButton::clicked, this, &ViewGroup::viewReset);
}

ViewGroup::ViewGroup(QWidget* parent) : FunctionGroup(tr("View Switch"), "viewGroup", parent) {
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
}

void RouteGroup::createButtons() {
    // create button container
    mpButtonContainer = new QWidget(this);
    mpButtonContainer->setObjectName("buttonContainer");

    // create button layout
    mpButtonLayout = new QVBoxLayout(mpButtonContainer);
    mpButtonLayout->setContentsMargins(0, 0, 0, 0);
    mpButtonLayout->setSpacing(5);  // set button spacing

    // create button
    mpBtnCreateRoute = new QPushButton(tr("Create Route"), mpButtonContainer);
    mpBtnCreateRoute->setObjectName("createRouteButton");
    mpBtnEditRoute = new QPushButton(tr("Edit Route"), mpButtonContainer);
    mpBtnEditRoute->setObjectName("editRouteButton");

    mpButtonLayout->addWidget(mpBtnCreateRoute);
    mpButtonLayout->addWidget(mpBtnEditRoute);
}

void RouteGroup::createSlots() {
    connect(mpBtnCreateRoute, &QPushButton::clicked, this, &RouteGroup::createRoute);
    connect(mpBtnEditRoute, &QPushButton::clicked, this, &RouteGroup::editRoute);
}

RouteGroup::RouteGroup(QWidget* parent) : FunctionGroup(tr("Route Planning"), "routeGroup", parent) {
    createSpins();
    mpGroupLayout->addRow("Base Height:", mpBaseHeightSpin);
    mpGroupLayout->addRow("Set Altitude:", mpHeightSpin);
    mpGroupLayout->addRow("Flight Path Width:", mpWidthSpin);
    mpGroupLayout->addRow(tr("Controls:"), mpButtonContainer);
    createButtons();
    mpGroupLayout->addRow(mpButtonContainer);
    createSlots();

    logMessage("route planning group created", Qgis::MessageLevel::Success);
}

void FlightSimGroup::createSpins() {
    using namespace ws;
    FlightManager &flightManager = FlightManager::getInstance();
    mpSpeedSpin = new QDoubleSpinBox(this);
    mpSpeedSpin->setObjectName("pSpeedSpin");
    mpSpeedSpin->setRange(FlightManager::minFlightSpeed,
                        FlightManager::maxFlightSpeed);
    mpSpeedSpin->setValue(flightManager.getFlightSpeed());
    mpSpeedSpin->setSuffix(" m/s");
}

void FlightSimGroup::createSlots() {
    connect(mpBtnStart, &QPushButton::clicked, this, &FlightSimGroup::simulationStart);
    connect(mpBtnPause, &QPushButton::clicked, this, &FlightSimGroup::simulationPause);
    connect(mpBtnResume, &QPushButton::clicked, this, &FlightSimGroup::simulationResume);
    connect(mpBtnReturn, &QPushButton::clicked, this, &FlightSimGroup::simulationReturnHome);
    connect(mpBtnStop, &QPushButton::clicked, this, &FlightSimGroup::simulationStop);
}

void FlightSimGroup::createButtons() {
    mpControlRow1 = new QHBoxLayout(this);
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
    mpBtnReturn = new QPushButton("Return Home", this);
    mpBtnReturn->setObjectName("returnButton");
    mpControlRow2->addWidget(mpBtnResume);
    mpControlRow2->addWidget(mpBtnReturn);

    mpControlRow3 = new QHBoxLayout();
    mpControlRow3->setObjectName("controlRow3");
    mpBtnStop = new QPushButton("Stop Simulation", this);
    mpBtnStop->setObjectName("stopButton");
    mpControlRow3->addWidget(mpBtnStop);
}

FlightSimGroup::FlightSimGroup(QWidget* parent) : FunctionGroup(tr("Flight Simulation"), "flightSimGroup", parent) {
    setObjectName("flightSimGroup");
    createSpins();
    createButtons();
    mpGroupLayout->addRow("Flight Speed:", mpSpeedSpin);
    mpGroupLayout->addRow(mpControlRow1);
    mpGroupLayout->addRow(mpControlRow2);
    mpGroupLayout->addRow(mpControlRow3);
    createSlots();

    logMessage("flight simulation group created", Qgis::MessageLevel::Success);
}

void FlightQueryGroup::createSlots() {
    connect(mpBtnQueryParams, &QPushButton::clicked, this, &FlightQueryGroup::queryFlightParams);
}

void FlightQueryGroup::createDialog() {
    using namespace ws;
    FlightManager &flightManager = FlightManager::getInstance();
    mpFlightParamsDisplay = new QLabel(flightManager.queryFlightParameters(), this);
    mpFlightParamsDisplay->setWordWrap(true);
    mpFlightParamsDisplay->setFrameStyle(QFrame::Box);

    mpFlightParamsDialog = new QDialog(this);
    mpFlightParamsDialog->setObjectName("flightParamsDialog");
    mpFlightParamsDialog->setModal(true);
    mpFlightParamsDialog->setWindowTitle(tr("Flight Parameters Settings"));

    mpFlightParamsForm = new QFormLayout(mpFlightParamsDialog);

    mpSpeedSpin = new QDoubleSpinBox(mpFlightParamsDialog);
    mpSpeedSpin->setRange(FlightManager::minFlightSpeed,FlightManager::maxFlightSpeed);
    mpSpeedSpin->setValue(flightManager.getFlightSpeed());
    mpFlightParamsForm->addRow(tr("Flight Speed (m/s):"), mpSpeedSpin);

    mpAltitudeSpin = new QDoubleSpinBox(mpFlightParamsDialog);
    mpAltitudeSpin->setRange(FlightManager::minFlightAltitude,FlightManager::maxFlightAltitude);
    mpAltitudeSpin->setValue(flightManager.getFlightAltitude());
    mpFlightParamsForm->addRow(tr("Max Altitude (m):"), mpAltitudeSpin);

    mpBatterySpin = new QDoubleSpinBox(mpFlightParamsDialog);
    mpBatterySpin->setRange(FlightManager::minFlightBattery,FlightManager::maxFlightBattery);
    mpBatterySpin->setValue(flightManager.getFlightBattery());
    mpFlightParamsForm->addRow(tr("Battery Capacity (%):"), mpBatterySpin);

    mpFlightParamsButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, mpFlightParamsDialog);
    mpFlightParamsForm->addRow(mpFlightParamsButtonBox);

    logMessage("show flight parameters dialog", Qgis::MessageLevel::Success);
}

void FlightQueryGroup::refreshFlightParams() {
    using namespace ws;
    FlightManager &flightManager = FlightManager::getInstance();
    flightManager.setFlightSpeed(mpSpeedSpin->value());
    flightManager.setFlightAltitude(mpAltitudeSpin->value());
    flightManager.setFlightBattery(mpBatterySpin->value());
    mpFlightParamsDisplay->setText(flightManager.queryFlightParameters());
}

void FlightQueryGroup::createSlots() {
    connect(mpFlightParamsButtonBox, &QDialogButtonBox::accepted, mpFlightParamsDialog, &QDialog::accept);
    connect(mpFlightParamsButtonBox, &QDialogButtonBox::rejected, mpFlightParamsDialog, &QDialog::reject);
    connect(mpFlightParamsDialog, &QDialog::accepted, this, &FlightQueryGroup::refreshFlightParams);
}

FlightQueryGroup::FlightQueryGroup(QWidget* parent) : FunctionGroup(tr("Flight Query"), "flightQueryGroup", parent) {
    setObjectName("flightQueryGroup");
    
    mpGroupLayout->addWidget(mpFlightParamsDisplay);
    mpGroupLayout->addWidget(mpBtnQueryParams);

    createDialog();
    createSlots();

    logMessage("flight parameters query group created", Qgis::MessageLevel::Success);
}

void EnvQueryGroup::createSlots() {
    connect(mpBtnRefreshData, &QPushButton::clicked, this, &EnvQueryGroup::queryEnvParams);
}

void EnvQueryGroup::refreshEnvParams() {
    using namespace ws;
    EnvManager &envManager = EnvManager::getInstance();
    
}

void EnvQueryGroup::createButtons() {
    mpBtnRefreshData = new QPushButton("Refresh Data", this);
    mpBtnRefreshData->setObjectName("pBtnRefreshData");
}

void EnvQueryGroup::createDialog() {
  mpEnvParamsDialog = new QDialog(this);
  mpEnvParamsDialog->setWindowTitle(tr("Environmental Parameters Settings"));

  mpEnvParamsForm = new QFormLayout(mpEnvParamsDialog);

  ws::EnvManager &envManager = ws::EnvManager::getInstance();
  mpWeatherCombo = new QComboBox(mpEnvParamsDialog);
  mpWeatherCombo->addItems(envManager.weatherList);
  mpWeatherCombo->setCurrentText(envManager.getWeatherString());
  mpEnvParamsForm->addRow(tr("Weather Condition:"), mpWeatherCombo);

  mpTemperatureSpin = new QDoubleSpinBox(mpEnvParamsDialog);
  mpTemperatureSpin->setRange(ws::EnvManager::minTemperature,
                     ws::EnvManager::maxTemperature);
  mpTemperatureSpin->setValue(envManager.getTemperature());
  mpEnvParamsForm->addRow(tr("Temperature (°C):"), mpTemperatureSpin);

  mpPressureSpin = new QDoubleSpinBox(mpEnvParamsDialog);
  mpPressureSpin->setRange(ws::EnvManager::minPressure,
                         ws::EnvManager::maxPressure);
  mpPressureSpin->setValue(envManager.getPressure());
  mpEnvParamsForm->addRow(tr("Pressure (hPa):"), mpPressureSpin);

  mpEnvParamsButtonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, mpEnvParamsDialog);
  mpEnvParamsForm->addRow(mpEnvParamsButtonBox);

  logMessage("show environmental parameters dialog",Qgis::MessageLevel::Success);
}

void EnvQueryGroup::refreshEnvParams() {
    using namespace ws;
    EnvManager &envManager = EnvManager::getInstance();
    envManager.setWeather(static_cast<ws::WeatherType>(mpWeatherCombo->currentIndex()));
    envManager.setTemperature(mpTemperatureSpin->value());
    envManager.setPressure(mpPressureSpin->value());
    mpWeatherLabel->setText(QString("Weather: %1").arg(envManager.getWeatherString()));
    mpTemperatureLabel->setText(QString("Temperature: %1 C").arg(envManager.getTemperature(), 0, 'f', 1));
    mpPressureLabel->setText(QString("Pressure: %1 hPa").arg(envManager.getPressure(), 0, 'f', 1));
}

void EnvQueryGroup::createSlots() {
    connect(mpBtnRefreshData, &QPushButton::clicked, this, &ws::EnvManager::generateRandomWeather);
    connect(mpEnvParamsButtonBox, &QDialogButtonBox::accepted, mpEnvParamsDialog, &QDialog::accept);
    connect(mpEnvParamsButtonBox, &QDialogButtonBox::rejected, mpEnvParamsDialog, &QDialog::reject);
    connect(mpEnvParamsDialog, &QDialog::accepted, this, &EnvQueryGroup::refreshEnvParams);
}

EnvQueryGroup::EnvQueryGroup(QWidget* parent) : FunctionGroup(tr("Environmental Data"), "basicDataGroup", parent) {
    setObjectName("basicDataGroup");

    ws::EnvManager &envManager = ws::EnvManager::getInstance();
    mpWeatherLabel = new QLabel(QString("Weather: %1").arg(envManager.getWeatherString()), this);
    mpTemperatureLabel = new QLabel(QString("Temperature: %1 C").arg(envManager.getTemperature(), 0, 'f', 1), this);
    mpPressureLabel = new QLabel(QString("Pressure: %1 hPa").arg(envManager.getPressure(), 0, 'f', 1), this);
    createButtons();

    mpGroupLayout->addRow("Weather:", mpWeatherLabel);
    mpGroupLayout->addRow("Temperature:", mpTemperatureLabel);
    mpGroupLayout->addRow("Pressure:", mpPressureLabel);
    mpGroupLayout->addRow(mpBtnRefreshData);

    createDialog();
    createSlots();
    
    logMessage("basic data group created", Qgis::MessageLevel::Success);
}

LeftDockWidget::LeftDockWidget(QWidget *parent) : QDockWidget(parent) {
    setObjectName("leftDockWidget");
    setAllowedAreas(Qt::LeftDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    setMinimumWidth(200);

    mpMainLayout = new QVBoxLayout(this);
    createScrollArea(this);
    createDockContent(mpScrollArea);
    mpViewGroup = new ViewGroup(mpDockContent);
    mpRouteGroup = new RouteGroup(mpDockContent);
    mpFlightSimGroup = new FlightSimGroup(mpDockContent);
    mpMainLayout->addStretch();
    mpFlightQueryGroup = new FlightQueryGroup(mpDockContent);
    mpEnvQueryGroup = new EnvQueryGroup(mpDockContent);
    logMessage("left dock widget created", Qgis::MessageLevel::Success);
}