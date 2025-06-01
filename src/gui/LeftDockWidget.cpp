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
    mpBtnSetHome = new QPushButton(tr("Set Home"), mpButtonContainer);
    mpBtnAddControlPoint = new QPushButton(tr("Add Control Point"), mpButtonContainer);
    mpBtnEditPoint = new QPushButton(tr("Edit Point"), mpButtonContainer);
    mpBtnGenerate = new QPushButton(tr("Generate"), mpButtonContainer);

    // set button object name
    mpBtnCreateRoute->setObjectName("createRouteButton");
    mpBtnSetHome->setObjectName("setHomeButton");
    mpBtnAddControlPoint->setObjectName("addControlPointButton");
    mpBtnEditPoint->setObjectName("editPointButton");
    mpBtnGenerate->setObjectName("generateButton");

    // add to layout
    mpButtonLayout->addWidget(mpBtnCreateRoute);
    mpButtonLayout->addWidget(mpBtnSetHome);
    mpButtonLayout->addWidget(mpBtnAddControlPoint);
    mpButtonLayout->addWidget(mpBtnEditPoint);
    mpButtonLayout->addWidget(mpBtnGenerate);
}

RouteGroup::RouteGroup(QWidget* parent) : FunctionGroup(tr("Route Planning"), "routeGroup", parent) {
    createSpins();
    createButtons();
    mpGroupLayout->addRow("Base Height:", mpBaseHeightSpin);
    mpGroupLayout->addRow("Set Altitude:", mpHeightSpin);
    mpGroupLayout->addRow("Flight Path Width:", mpWidthSpin);
    mpGroupLayout->addRow(tr("Controls:"), mpButtonContainer);

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

    logMessage("flight simulation group created", Qgis::MessageLevel::Success);
}

void FlightQueryGroup::createButtons() {
    mpBtnQueryParams = new QPushButton("Query Parameters", this);
    mpBtnQueryParams->setObjectName("pBtnQueryParams");
    mpFlightParamsDisplay = new QLabel("No flight data available", this);
    mpFlightParamsDisplay->setWordWrap(true);
    mpFlightParamsDisplay->setFrameStyle(QFrame::Box);
}

FlightQueryGroup::FlightQueryGroup(QWidget* parent) : FunctionGroup(tr("Flight Query"), "flightQueryGroup", parent) {
    setObjectName("flightQueryGroup");
    createButtons();
    
    mpGroupLayout->addWidget(mpBtnQueryParams);
    mpGroupLayout->addWidget(mpFlightParamsDisplay);
    logMessage("flight parameters query group created",
                Qgis::MessageLevel::Success);
}

void BasicDataGroup::createButtons() {
    mpBtnRefreshData = new QPushButton("Refresh Data", this);
    mpBtnRefreshData->setObjectName("pBtnRefreshData");
    mpWeatherLabel = new QLabel("Weather: -", this);
    mpTemperatureLabel = new QLabel("Temperature: -", this);
    mpPressureLabel = new QLabel("Pressure: -", this);
}

BasicDataGroup::BasicDataGroup(QWidget* parent) : FunctionGroup(tr("Environmental Data"), "basicDataGroup", parent) {
    setObjectName("basicDataGroup");

    createButtons();
    mpGroupLayout->addRow("Weather:", mpWeatherLabel);
    mpGroupLayout->addRow("Temperature:", mpTemperatureLabel);
    mpGroupLayout->addRow("Pressure:", mpPressureLabel);
    mpGroupLayout->addRow(mpBtnRefreshData);
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
    mpBasicDataGroup = new BasicDataGroup(mpDockContent);
    logMessage("left dock widget created", Qgis::MessageLevel::Success);
}

void MainWindow::showFlightParamsDialog() {
  logMessage("show flight parameters dialog", Qgis::MessageLevel::Info);
  QDialog *dialog = new QDialog(this);
  dialog->setWindowTitle(tr("Flight Parameters Settings"));

  QFormLayout *form = new QFormLayout(dialog);

  ws::FlightManager &flightManager = ws::FlightManager::getInstance();
  QDoubleSpinBox *speedSpin = new QDoubleSpinBox(dialog);
  speedSpin->setRange(ws::FlightManager::minFlightSpeed,
                      ws::FlightManager::maxFlightSpeed);
  speedSpin->setValue(ws::FlightManager::getInstance().getFlightSpeed());
  form->addRow(tr("Flight Speed (m/s):"), speedSpin);

  QDoubleSpinBox *altitudeSpin = new QDoubleSpinBox(dialog);
  altitudeSpin->setRange(ws::FlightManager::minFlightAltitude,
                         ws::FlightManager::maxFlightAltitude);
  altitudeSpin->setValue(ws::FlightManager::getInstance().getFlightAltitude());
  form->addRow(tr("Max Altitude (m):"), altitudeSpin);

  QDoubleSpinBox *batterySpin = new QDoubleSpinBox(dialog);
  batterySpin->setRange(ws::FlightManager::minFlightBattery,
                        ws::FlightManager::maxFlightBattery);
  batterySpin->setValue(ws::FlightManager::getInstance().getFlightBattery());
  form->addRow(tr("Battery Capacity (%):"), batterySpin);

  QDialogButtonBox *buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, dialog);
  form->addRow(buttonBox);

  connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
  logMessage("connect flight parameters dialog button box accepted",
             Qgis::MessageLevel::Success);
  connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
  logMessage("connect flight parameters dialog button box rejected",
             Qgis::MessageLevel::Success);

  if (dialog->exec() == QDialog::Accepted) {
    flightManager.setFlightSpeed(speedSpin->value());
    flightManager.setFlightAltitude(altitudeSpin->value());
    flightManager.setFlightBattery(batterySpin->value());

    queryFlightParameters(); // 刷新飞行参数显示
  }
  logMessage("show flight parameters dialog", Qgis::MessageLevel::Success);
}
void MainWindow::showEnvironmentalParamsDialog() {
  logMessage("show environmental parameters dialog", Qgis::MessageLevel::Info);
  QDialog *dialog = new QDialog(this);
  dialog->setWindowTitle(tr("Environmental Parameters Settings"));

  QFormLayout *form = new QFormLayout(dialog);

  ws::EnvManager &envManager = ws::EnvManager::getInstance();
  QComboBox *weatherCombo = new QComboBox(dialog);
  weatherCombo->addItems(envManager.weatherList);
  weatherCombo->setCurrentText(envManager.getWeatherString());
  form->addRow(tr("Weather Condition:"), weatherCombo);

  QDoubleSpinBox *tempSpin = new QDoubleSpinBox(dialog);
  tempSpin->setRange(ws::EnvManager::minTemperature,
                     ws::EnvManager::maxTemperature);
  tempSpin->setValue(envManager.getTemperature());
  form->addRow(tr("Temperature (°C):"), tempSpin);

  QDoubleSpinBox *pressureSpin = new QDoubleSpinBox(dialog);
  pressureSpin->setRange(ws::EnvManager::minPressure,
                         ws::EnvManager::maxPressure);
  pressureSpin->setValue(envManager.getPressure());
  form->addRow(tr("Pressure (hPa):"), pressureSpin);

  QDialogButtonBox *buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, dialog);
  form->addRow(buttonBox);

  connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
  logMessage("connect environmental parameters dialog button box accepted",
             Qgis::MessageLevel::Success);
  connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
  logMessage("connect environmental parameters dialog button box rejected",
             Qgis::MessageLevel::Success);

  if (dialog->exec() == QDialog::Accepted) {
    envManager.setWeather(
        static_cast<ws::WeatherType>(weatherCombo->currentIndex()));
    envManager.setTemperature(tempSpin->value());
    envManager.setPressure(pressureSpin->value());

    refreshBasicData(); // refresh environment data display
  }
  logMessage("show environmental parameters dialog",
             Qgis::MessageLevel::Success);
}

// add new slot function at the end of the file
void MainWindow::queryFlightParameters() {
  logMessage("generate random flight parameters", Qgis::MessageLevel::Info);
  double speed = ws::FlightManager::getInstance().getFlightSpeed();
  double altitude = ws::FlightManager::getInstance().getFlightAltitude();
  double battery = ws::FlightManager::getInstance().getFlightBattery();
  double latitude = QRandomGenerator::global()->bounded(-90, 90);
  double longitude = QRandomGenerator::global()->bounded(-180, 180);

  QString params = QString("Current Flight Parameters:\n"
                           "Speed: %1 m/s\n"
                           "Altitude: %2 m\n"
                           "Battery: %3%\n"
                           "Position: (%4, %5)")
                       .arg(speed, 0, 'f', 1)
                       .arg(altitude, 0, 'f', 1)
                       .arg(battery, 0, 'f', 1)
                       .arg(latitude, 0, 'f', 6)
                       .arg(longitude, 0, 'f', 6);

  m_pFlightParamsDisplay->setText(params);
  logMessage("generate random flight parameters", Qgis::MessageLevel::Success);
}

void MainWindow::refreshBasicData() {
  logMessage("generate random weather data", Qgis::MessageLevel::Info);
  QStringList weatherTypes = {"Sunny", "Cloudy", "Rainy", "Snowy", "Foggy"};
  QString weather = weatherTypes[QRandomGenerator::global()->bounded(5)];

  ws::EnvManager &envManager = ws::EnvManager::getInstance();
  double temperature =
      QRandomGenerator::global()->bounded(envManager.minTemperature * 10,
                                          envManager.maxTemperature * 10) /
      10.0;

  double pressure =
      QRandomGenerator::global()->bounded(envManager.minPressure * 10,
                                          envManager.maxPressure * 10) /
      10.0;

  m_pWeatherLabel->setText(weather);
  m_pTemperatureLabel->setText(QString("%1 C").arg(temperature, 0, 'f', 1));
  m_pPressureLabel->setText(
      QString("%1 hPa").arg(pressure, 0, 'f', 1)); // 注意这里有两个闭合括号
}