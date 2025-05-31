
void MainWindow::createJoyDockWidgets() {
  // create flight control dock widget
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
}

void MainWindow::createLeftDockWidget() {
  // initialize left dock widget
  QDockWidget *pLeftDockWidget = new QDockWidget(tr("Control Panel"), this);
  pLeftDockWidget->setObjectName("pLeftDockWidget");
  pLeftDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea);
  pLeftDockWidget->setFeatures(QDockWidget::DockWidgetMovable |
                               QDockWidget::DockWidgetFloatable);
  pLeftDockWidget->setMinimumWidth(200);

  QScrollArea *scrollArea = new QScrollArea(pLeftDockWidget);
  scrollArea->setObjectName("scrollArea");
  scrollArea->setWidgetResizable(true);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  QWidget *pDockContent = new QWidget(pLeftDockWidget);
  pDockContent->setObjectName("pDockContent");
  pDockContent->setMinimumWidth(175);
  QVBoxLayout *pMainLayout = new QVBoxLayout(pDockContent);
  pMainLayout->setContentsMargins(10, 10, 10, 10);
  pMainLayout->setSpacing(10);

  // ===== view switch group =====
  QGroupBox *pViewGroup = new QGroupBox("View Switch", pDockContent);
  pViewGroup->setObjectName("pViewGroup");
  QVBoxLayout *pViewLayout = new QVBoxLayout(pViewGroup);
  pViewLayout->setObjectName("pViewLayout");
  pViewLayout->addWidget(mpBtnReset = new QPushButton("Reset", pViewGroup));
  pViewLayout->addWidget(mpBtnSwitchTo3D =
                             new QPushButton("3D View", pViewGroup));
  pViewLayout->addWidget(mpBtnSwitchTo2D =
                             new QPushButton("2D Map", pViewGroup));
  pMainLayout->addWidget(pViewGroup);
  logMessage("create view switch group", Qgis::MessageLevel::Success);

  // ===== route planning group =====
  QGroupBox *pRouteGroup = new QGroupBox("Route Planning");
  pRouteGroup->setObjectName("pRouteGroup");
  QFormLayout *pRouteLayout = new QFormLayout();
  pRouteLayout->setObjectName("pRouteLayout");
  logMessage("create route planning group", Qgis::MessageLevel::Success);

  // add base height control widget to route planning group
  QDoubleSpinBox *pBaseHeightSpin = new QDoubleSpinBox(pRouteGroup);
  pBaseHeightSpin->setObjectName("pBaseHeightSpin");
  pBaseHeightSpin->setRange(-1000.0, 1000.0);
  pBaseHeightSpin->setValue(0.0);
  pBaseHeightSpin->setSuffix(" m");
  pRouteLayout->addRow("Base Height:", pBaseHeightSpin);

  // set height selection box
  QDoubleSpinBox *pHeightSpin = new QDoubleSpinBox(pRouteGroup);
  pHeightSpin->setObjectName("pHeightSpin");
  pHeightSpin->setRange(1.0, 300.0);
  pHeightSpin->setValue(75.0);
  pHeightSpin->setSuffix(" m");
  pRouteLayout->addRow("Set Altitude:", pHeightSpin);

  // set height selection box
  QDoubleSpinBox *pWidthSpin = new QDoubleSpinBox(pRouteGroup);
  pWidthSpin->setObjectName("pWidthSpin");
  pWidthSpin->setRange(1.0, 300.0);
  pWidthSpin->setValue(10.0);
  pWidthSpin->setSuffix(" m");
  pRouteLayout->addRow("Flight Path Width:", pWidthSpin);

  // set operation buttons: create route, add, move, clear control points,
  // generate route
  QPushButton *pBtnCreateRoute =
      new QPushButton("Create Route", pRouteGroup); // start creating route
  pBtnCreateRoute->setObjectName("pBtnCreateRoute");
  QPushButton *pBtnSetHome = new QPushButton("Set Home Point", pRouteGroup);
  pBtnSetHome->setObjectName("pBtnSetHome");
  QPushButton *pBtnAddControlPoint = new QPushButton(
      "Add Control Point", pRouteGroup); // add control point button
  pBtnAddControlPoint->setObjectName("pBtnAddControlPoint");
  QPushButton *pBtnEditPoint = new QPushButton(
      "Edit Points", pRouteGroup); // bring up the toolbar, select the point,
                                   // and move, delete point function
  pBtnEditPoint->setObjectName("pBtnEditPoint");
  QPushButton *pBtnGenerate =
      new QPushButton("Generate Route", pRouteGroup); // generate route
  pBtnGenerate->setObjectName("pBtnGenerate");

  QWidget *buttonContainer = new QWidget(pRouteGroup);
  QVBoxLayout *pBtnColumn = new QVBoxLayout(buttonContainer);
  pBtnColumn->setObjectName("pBtnColumn");
  pBtnColumn->setContentsMargins(0, 0, 0, 0);
  pBtnColumn->addWidget(pBtnCreateRoute);
  pBtnColumn->addWidget(pBtnSetHome);
  pBtnColumn->addWidget(pBtnAddControlPoint);
  pBtnColumn->addWidget(pBtnEditPoint);
  pBtnColumn->addWidget(pBtnGenerate);
  pRouteLayout->addRow(buttonContainer);
  pRouteGroup->setLayout(pRouteLayout);
  pMainLayout->addWidget(pRouteGroup);

  logMessage("create route planning group", Qgis::MessageLevel::Success);

  // ===== flight simulation group =====
  QGroupBox *pSimGroup = new QGroupBox("Flight Simulation");
  pSimGroup->setObjectName("pSimGroup");
  QFormLayout *pSimLayout = new QFormLayout(pSimGroup);
  pSimLayout->setObjectName("pSimLayout");
  logMessage("create flight simulation group", Qgis::MessageLevel::Success);

  QDoubleSpinBox *pSpeedSpin = new QDoubleSpinBox(pSimGroup);
  pSpeedSpin->setObjectName("pSpeedSpin");
  pSpeedSpin->setRange(ws::FlightManager::minFlightSpeed,
                       ws::FlightManager::maxFlightSpeed);
  pSpeedSpin->setValue(ws::FlightManager::getInstance().getFlightSpeed());
  pSpeedSpin->setSuffix(" m/s");
  pSimLayout->addRow("Flight Speed:", pSpeedSpin);
  logMessage("create flight speed spin box", Qgis::MessageLevel::Success);

  QPushButton *pBtnStart = new QPushButton("Start", pSimGroup);
  pBtnStart->setObjectName("pBtnStart");
  QPushButton *pBtnPause = new QPushButton("Pause", pSimGroup);
  pBtnPause->setObjectName("pBtnPause");
  QPushButton *pBtnResume = new QPushButton("Resume", pSimGroup);
  pBtnResume->setObjectName("pBtnResume");
  QPushButton *pBtnReturn = new QPushButton("Return Home", pSimGroup);
  pBtnReturn->setObjectName("pBtnReturn");
  QPushButton *pBtnStop = new QPushButton("Stop Simulation", pSimGroup);
  pBtnStop->setObjectName("pBtnStop");
  QHBoxLayout *pControlRow1 = new QHBoxLayout();
  pControlRow1->setObjectName("pControlRow1");
  pControlRow1->addWidget(pBtnStart);
  pControlRow1->addWidget(pBtnPause);
  QHBoxLayout *pControlRow2 = new QHBoxLayout();
  pControlRow2->setObjectName("pControlRow2");
  pControlRow2->addWidget(pBtnResume);
  pControlRow2->addWidget(pBtnReturn);
  QHBoxLayout *pControlRow3 = new QHBoxLayout();
  pControlRow3->setObjectName("pControlRow3");
  pControlRow3->addWidget(pBtnStop);
  logMessage("create flight simulation group", Qgis::MessageLevel::Success);

  pSimLayout->addRow(pControlRow1);
  pSimLayout->addRow(pControlRow2);
  pSimLayout->addRow(pControlRow3);
  pMainLayout->addWidget(pSimGroup);

  pMainLayout->addStretch();

  scrollArea->setWidget(pDockContent);
  pLeftDockWidget->setWidget(scrollArea);
  addDockWidget(Qt::LeftDockWidgetArea, pLeftDockWidget);

  // ===== flight parameters query group =====
  QGroupBox *pFlightParamsGroup =
      new QGroupBox("Flight Parameters", pDockContent);
  pFlightParamsGroup->setObjectName("pFlightParamsGroup");
  QVBoxLayout *pFlightParamsLayout = new QVBoxLayout(pFlightParamsGroup);
  pFlightParamsLayout->setObjectName("pFlightParamsLayout");
  QPushButton *pBtnQueryParams =
      new QPushButton("Query Parameters", pFlightParamsGroup);
  pBtnQueryParams->setObjectName("pBtnQueryParams");
  m_pFlightParamsDisplay =
      new QLabel("No flight data available", pFlightParamsGroup);
  m_pFlightParamsDisplay->setWordWrap(true);
  m_pFlightParamsDisplay->setFrameStyle(QFrame::Box);
  pFlightParamsLayout->addWidget(pBtnQueryParams);
  pFlightParamsLayout->addWidget(m_pFlightParamsDisplay);
  pMainLayout->addWidget(pFlightParamsGroup);
  logMessage("create flight parameters query group",
             Qgis::MessageLevel::Success);

  // ===== basic data group =====
  QGroupBox *pBasicDataGroup =
      new QGroupBox("Environmental Data", pDockContent);
  pBasicDataGroup->setObjectName("pBasicDataGroup");
  QFormLayout *pBasicDataLayout = new QFormLayout(pBasicDataGroup);
  pBasicDataLayout->setObjectName("pBasicDataLayout");
  m_pWeatherLabel = new QLabel("Weather: -", pBasicDataGroup);
  m_pTemperatureLabel = new QLabel("Temperature: -", pBasicDataGroup);
  m_pPressureLabel = new QLabel("Pressure: -", pBasicDataGroup);
  QPushButton *pBtnRefreshData =
      new QPushButton("Refresh Data", pBasicDataGroup);
  pBtnRefreshData->setObjectName("pBtnRefreshData");
  pBasicDataLayout->addRow("Weather:", m_pWeatherLabel);
  pBasicDataLayout->addRow("Temperature:", m_pTemperatureLabel);
  pBasicDataLayout->addRow("Pressure:", m_pPressureLabel);
  pBasicDataLayout->addRow(pBtnRefreshData);
  pMainLayout->addWidget(pBasicDataGroup);
  logMessage("create basic data group", Qgis::MessageLevel::Success);
}
void MainWindow::createRightDockWidget() {
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
