#include "DockWidget.h"
#include "../log/QgisDebug.h"
#include "../core/WorkspaceState.h"

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

RightDockWidget::RightDockWidget(QWidget *parent) : QDockWidget(parent) {
  logMessage("create right dock widget", Qgis::MessageLevel::Success);
    // initialize right dock widget
  QDockWidget *pRightDockWidget = new QDockWidget(tr("Property Panel"));
  pRightDockWidget->setObjectName("pRightDockWidget");
  pRightDockWidget->setAllowedAreas(Qt::RightDockWidgetArea);
  pRightDockWidget->setFeatures(QDockWidget::DockWidgetMovable |
                                QDockWidget::DockWidgetFloatable);
  addDockWidget(Qt::RightDockWidgetArea, pRightDockWidget);
  logMessage("create right dock widget", Qgis::MessageLevel::Success);

  // Create a main container widget for the right dock
  QWidget *mainContainer = new QWidget(pRightDockWidget);
  QVBoxLayout *mainLayout = new QVBoxLayout(mainContainer);

  // create file tree
  mpFileTreeWidget = new QTreeWidget();
  mpFileTreeWidget->setObjectName("mpFileTreeWidget");
  mpFileTreeWidget->setHeaderLabel(tr("File List"));
  logMessage("create file tree", Qgis::MessageLevel::Success);

  // set right-click menu
  mpFileTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(mpFileTreeWidget, &QTreeWidget::customContextMenuRequested, this,
          &MainWindow::Unrealized);
  logMessage("connect file tree to unrealized", Qgis::MessageLevel::Info);

  // add double click event processing
  connect(mpFileTreeWidget, &QTreeWidget::itemDoubleClicked, this,
          &MainWindow::onTreeItemDoubleClicked);

  // create select directory button
  QToolButton *selectDirectoryButton = new QToolButton();
  selectDirectoryButton->setObjectName("selectDirectoryButton");
  selectDirectoryButton->setText(tr("Select Directory"));
  connect(selectDirectoryButton, &QToolButton::clicked, this,
          &MainWindow::onSelectDirectoryClicked);
  logMessage("connect select directory button to onSelectDirectoryClicked",
             Qgis::MessageLevel::Info);

  // Add widgets to main layout
  mainLayout->addWidget(selectDirectoryButton);
  mainLayout->addWidget(mpFileTreeWidget);

  // Set the main container as the dock widget's widget
  pRightDockWidget->setWidget(mainContainer);

  // load file list of specified directory to tree widget
  QString dirPath = ws::PathManager::getInstance().getRootDir();
  loadDirectoryFiles(dirPath);
  logMessage("load file list of specified directory to tree widget",
             Qgis::MessageLevel::Success);

  // Create right bottom tool box sidebar
  QDockWidget *mpRightDock = new QDockWidget(tr("tool box"), this);
  mpRightDock->setObjectName("mpRightDock");
  mpRightDock->setAllowedAreas(Qt::RightDockWidgetArea);
  addDockWidget(Qt::RightDockWidgetArea, mpRightDock);

  // Create tool box container
  QWidget *toolWidget = new QWidget(mpRightDock);
  QVBoxLayout *toolLayout = new QVBoxLayout(toolWidget);

  QTreeWidget *mpToolTree = new QTreeWidget();
  mpToolTree->setHeaderHidden(true); // hide header
  logMessage("create tool tree", Qgis::MessageLevel::Success);

  // tool tree
  // ===== route planning group =====
  QTreeWidgetItem *pRoutePlanningItem = new QTreeWidgetItem(mpToolTree);
  pRoutePlanningItem->setText(0, tr("RoutePlanning"));
  QTreeWidgetItem *pPaintItem = new QTreeWidgetItem(pRoutePlanningItem);
  pPaintItem->setText(0, tr("paint"));
  QTreeWidgetItem *pCreateRouteItem = new QTreeWidgetItem(pRoutePlanningItem);
  pCreateRouteItem->setText(0, tr("Create Route"));
  QTreeWidgetItem *pSetHomeItem = new QTreeWidgetItem(pRoutePlanningItem);
  pSetHomeItem->setText(0, tr("Set Home"));
  QTreeWidgetItem *pAddControlPointItem =
      new QTreeWidgetItem(pRoutePlanningItem);
  pAddControlPointItem->setText(0, tr("Add Control Point"));
  QTreeWidgetItem *pEditPointsItem = new QTreeWidgetItem(pRoutePlanningItem);
  pEditPointsItem->setText(0, tr("Edit Points"));
  QTreeWidgetItem *pGenerateRouteItem = new QTreeWidgetItem(pRoutePlanningItem);
  pGenerateRouteItem->setText(0, tr("Generate Route"));
  logMessage("create route planning item", Qgis::MessageLevel::Success);

  // ===== simulation control group =====
  QTreeWidgetItem *pSimulationItem = new QTreeWidgetItem(mpToolTree);
  pSimulationItem->setText(0, tr("Simulation"));
  QTreeWidgetItem *pStartItem = new QTreeWidgetItem(pSimulationItem);
  pStartItem->setText(0, tr("Start"));
  QTreeWidgetItem *pPauseItem = new QTreeWidgetItem(pSimulationItem);
  pPauseItem->setText(0, tr("Pause"));
  QTreeWidgetItem *pResumeItem = new QTreeWidgetItem(pSimulationItem);
  pResumeItem->setText(0, tr("Resume"));
  QTreeWidgetItem *pReturnHomeItem = new QTreeWidgetItem(pSimulationItem);
  pReturnHomeItem->setText(0, tr("Return Home"));
  QTreeWidgetItem *pStopSimulationItem = new QTreeWidgetItem(pSimulationItem);
  pStopSimulationItem->setText(0, tr("Stop Simulation"));
  logMessage("create simulation control item", Qgis::MessageLevel::Success);

  // ===== view switch group =====
  QTreeWidgetItem *pSwitchViewItem = new QTreeWidgetItem(mpToolTree);
  pSwitchViewItem->setText(0, tr("SwitchView"));
  QTreeWidgetItem *pResetItem = new QTreeWidgetItem(pSwitchViewItem);
  pResetItem->setText(0, tr("Reset"));
  QTreeWidgetItem *p3DItem = new QTreeWidgetItem(pSwitchViewItem);
  p3DItem->setText(0, tr("SwitchTo3D"));
  QTreeWidgetItem *p2DItem = new QTreeWidgetItem(pSwitchViewItem);
  p2DItem->setText(0, tr("SwitchTo2D"));
  logMessage("create view switch item", Qgis::MessageLevel::Success);

  // ===== new flight parameters group =====
  QTreeWidgetItem *pFlightParamsItem = new QTreeWidgetItem(mpToolTree);
  pFlightParamsItem->setText(0, tr("Flight Parameters"));
  QTreeWidgetItem *pQueryParamsItem = new QTreeWidgetItem(pFlightParamsItem);
  pQueryParamsItem->setText(0, tr("Query Parameters"));
  logMessage("create flight parameters item", Qgis::MessageLevel::Success);

  // ===== new environmental data group =====
  QTreeWidgetItem *pEnvDataItem = new QTreeWidgetItem(mpToolTree);
  pEnvDataItem->setText(0, tr("Environmental Data"));
  QTreeWidgetItem *pRefreshDataItem = new QTreeWidgetItem(pEnvDataItem);
  pRefreshDataItem->setText(0, tr("Refresh Data"));

  toolLayout->addWidget(mpToolTree);
  mpRightDock->setWidget(toolWidget);
  logMessage("create tool box", Qgis::MessageLevel::Success);

  QDoubleSpinBox *pBaseHeightSpin =
      this->safeFindChild<QDoubleSpinBox *>("pBaseHeightSpin");
  QDoubleSpinBox *pHeightSpin =
      this->safeFindChild<QDoubleSpinBox *>("pHeightSpin");
  QDoubleSpinBox *pSpeedSpin =
      this->safeFindChild<QDoubleSpinBox *>("pSpeedSpin");
  QDoubleSpinBox *pWidthSpin =
      this->safeFindChild<QDoubleSpinBox *>("pWidthSpin");
  QPushButton *pBtnAddControlPoint =
      this->safeFindChild<QPushButton *>("pBtnAddControlPoint");
  QPushButton *pBtnCreateRoute =
      this->safeFindChild<QPushButton *>("pBtnCreateRoute");
  QPushButton *pBtnEditPoint =
      this->safeFindChild<QPushButton *>("pBtnEditPoint");
  QPushButton *pBtnGenerate =
      this->safeFindChild<QPushButton *>("pBtnGenerate");
  QPushButton *pBtnPause = this->safeFindChild<QPushButton *>("pBtnPause");
  QPushButton *pBtnQueryParams =
      this->safeFindChild<QPushButton *>("pBtnQueryParams");
  QPushButton *pBtnRefreshData =
      this->safeFindChild<QPushButton *>("pBtnRefreshData");
  QPushButton *pBtnResume = this->safeFindChild<QPushButton *>("pBtnResume");
  QPushButton *pBtnReturn = this->safeFindChild<QPushButton *>("pBtnReturn");
  QPushButton *pBtnSetHome = this->safeFindChild<QPushButton *>("pBtnSetHome");
  QPushButton *pBtnStart = this->safeFindChild<QPushButton *>("pBtnStart");
  QPushButton *pBtnStop = this->safeFindChild<QPushButton *>("pBtnStop");

  connect(mpToolTree, &QTreeWidget::itemClicked, this,
          [=](QTreeWidgetItem *item, int) {
            if (item == pPaintItem) {
              Unrealized();
            } else if (item == p3DItem) {
              switchTo3D();
            } else if (item == p2DItem) {
              switchTo2D();
            }
            // bind route planning group functions
            else if (item == pCreateRouteItem) {
              pBtnCreateRoute->click();
            } else if (item == pSetHomeItem) {
              pBtnSetHome->click();
            } else if (item == pAddControlPointItem) {
              pBtnAddControlPoint->click();
            } else if (item == pEditPointsItem) {
              pBtnEditPoint->click();
            } else if (item == pGenerateRouteItem) {
              pBtnGenerate->click();
            }
            // bind simulation control group functions
            else if (item == pStartItem) {
              pBtnStart->click();
            } else if (item == pPauseItem) {
              pBtnPause->click();
            } else if (item == pResumeItem) {
              pBtnResume->click();
            } else if (item == pReturnHomeItem) {
              pBtnReturn->click();
            } else if (item == pStopSimulationItem) {
              pBtnStop->click();
            }
            // bind parameter query function
            else if (item == pQueryParamsItem) {
              pBtnQueryParams->click();
            }
            // bind environmental data refresh function
            else if (item == pRefreshDataItem) {
              pBtnRefreshData->click();
            }
          });
  logMessage("connect right widget to slots", Qgis::MessageLevel::Success);
}

JoyDockWidget::JoyDockWidget(QWidget *parent) : QDockWidget(parent) {
     QDockWidget *controlDock = new QDockWidget(tr("Flight Control"), this);
  QWidget *controlPanel = new QWidget(this);

  // main vertical layout
  QVBoxLayout *mainLayout = new QVBoxLayout(controlPanel);
  mainLayout->setContentsMargins(20, 20, 20, 20); // 增加边距
  mainLayout->setSpacing(30);                     // 摇杆与按钮间距

  // joystick row (horizontal layout)=====================================
  QHBoxLayout *joystickLayout = new QHBoxLayout(controlPanel);
  joystickLayout->setSpacing(40); // 增大摇杆间距

  // m_leftJoystick = new JoystickWidget();
  // m_rightJoystick = new JoystickWidget();

  // 设置大尺寸摇杆
  const int joystickSize = 180; // 摇杆尺寸
  // m_leftJoystick->setFixedSize(joystickSize, joystickSize);
  // m_rightJoystick->setFixedSize(joystickSize, joystickSize);

  QString joystickStyle = "background-color: #333333;"
                          "border-radius: " +
                          QString::number(joystickSize / 2) +
                          "px;"; // 动态计算圆角
  // m_leftJoystick->setStyleSheet(joystickStyle);
  // m_rightJoystick->setStyleSheet(joystickStyle);

  // joystickLayout->addWidget(m_leftJoystick);
  // joystickLayout->addWidget(m_rightJoystick);
  mainLayout->addLayout(joystickLayout);

  // 按钮行（保持大尺寸）===================================
  QHBoxLayout *buttonLayout = new QHBoxLayout(controlPanel);
  buttonLayout->setAlignment(Qt::AlignHCenter);
  buttonLayout->setSpacing(30); // 按钮间距

  m_btnManualMode = new QPushButton("手动模式", controlPanel);
  m_btnAutoMode = new QPushButton("自动模式", controlPanel);

  // 按钮样式（保持大尺寸但调整比例）
  QString buttonStyle = "QPushButton {"
                        "  background-color: #4A4A4A;"
                        "  border: 2px solid #5A5A5A;"
                        "  border-radius: 8px;"
                        "  padding: 15px 30px;"         // 增大内边距
                        "  min-width: 140px;"           // 保持大宽度
                        "  min-height: 45px;"           // 保持大高度
                        "  font: bold 16px '微软雅黑';" // 字体稍增大
                        "}"
                        "QPushButton:hover {"
                        "  background-color: #5A5A5A;"
                        "}";
  m_btnManualMode->setStyleSheet(buttonStyle);
  m_btnAutoMode->setStyleSheet(buttonStyle);

  buttonLayout->addWidget(m_btnManualMode);
  buttonLayout->addWidget(m_btnAutoMode);
  mainLayout->addLayout(buttonLayout);

  controlDock->setWidget(controlPanel);
  addDockWidget(Qt::RightDockWidgetArea, controlDock);
  logMessage("create joy dock widget", Qgis::MessageLevel::Success);
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

void MainWindow::onTreeItemExpanded(QTreeWidgetItem *item) {
  if (item->childCount() == 1 && item->child(0)->text(0).isEmpty()) {
    QString path = getItemFullPath(item);
    item->removeChild(item->child(0));
    loadDirectoryLevel(item, path, 1, 1);

    for (int i = 0; i < item->childCount(); ++i) {
      QTreeWidgetItem *child = item->child(i);
      QFileInfo fileInfo(getItemFullPath(child));

      if (fileInfo.isDir())
        child->setHidden(false);
      else
        child->setHidden(
            !child->text(0).endsWith(".obj")); // only show obj file
    }
  }
}

QString MainWindow::getItemFullPath(QTreeWidgetItem *item) {
  QStringList pathParts;
  while (item && item->parent()) {
    pathParts.prepend(item->text(0));
    item = item->parent();
  }
  if (item)
    pathParts.prepend(item->text(0));
  QString rootPath = ws::PathManager::getInstance().getRootDir();
  return QDir(rootPath).filePath(pathParts.join("/"));
}

void MainWindow::onTreeItemDoubleClicked(QTreeWidgetItem *item, int column) {
  if (!item)
    return;

  QString filePath = getItemFullPath(item);
  QFileInfo fileInfo(filePath);

  if (fileInfo.isFile() && fileInfo.suffix().toLower() == "obj") {
    mpOpenGLWidget->loadModel(filePath);
  }
}

void MainWindow::loadModel(const QString &objFilePath) {
  mpOpenGLWidget->loadModel(objFilePath);
}


void MainWindow::onSelectDirectoryClicked() {
  // open folder selection dialog
  QString currentDir = ws::PathManager::getInstance().getRootDir();
  QString dirPath = QFileDialog::getExistingDirectory(
      this, tr("Select Directory"), currentDir);
  if (!dirPath.isEmpty()) {
    loadDirectoryFiles(
        dirPath); // call loadDirectoryFiles to load selected directory
  }
  logMessage("select file list directory", Qgis::MessageLevel::Success);
}
// load file list of specified directory to QTreeWidget
void MainWindow::loadDirectoryFiles(const QString &path) {
  QDir dir(path);
  if (!dir.exists())
    return;

  mpFileTreeWidget->clear();

  QTreeWidgetItem *rootItem = new QTreeWidgetItem(mpFileTreeWidget);
  rootItem->setText(0, dir.dirName());
  loadDirectoryLevel(rootItem, path, 1, 3);

  connect(mpFileTreeWidget, &QTreeWidget::itemExpanded, this,
          &MainWindow::onTreeItemExpanded);
}

void MainWindow::loadDirectoryLevel(QTreeWidgetItem *parentItem,
                                    const QString &path, int level,
                                    int maxLevel) {
  if (level > maxLevel)
    return;

  QDir dir(path);
  QFileInfoList files =
      dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

  foreach (const QFileInfo &fileInfo, files) {
    QTreeWidgetItem *item = new QTreeWidgetItem(parentItem);
    item->setText(0, fileInfo.fileName());

    if (fileInfo.isDir()) {
      if (level < maxLevel) {
        loadDirectoryLevel(item, fileInfo.absoluteFilePath(), level + 1,
                           maxLevel);
      } else if (level == maxLevel) {
        new QTreeWidgetItem(item);
      }
    }
  }
}