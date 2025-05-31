#include "MainWindow.h"
#include "core/WorkspaceState.h"
#include "gui/StyleManager.h"
#include "log/QgisDebug.h"
#include <QAction>
#include <QApplication>
#include <memory>
#include <QFile>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  logMessage("Application started", Qgis::MessageLevel::Info);
  initWindowStatus();
  ws::initializeWorkspaceState();
  logMessage("MainWindow constructor called", Qgis::MessageLevel::Success);
  init3DWidget();
  init2DWidget();

  logMessage("function class initialized", Qgis::MessageLevel::Success);

  mpMenuBar = new MenuBar(this);
  logMessage("create menu bar", Qgis::MessageLevel::Success);
  mpCanvas = new Canvas(this);
  QMainWindow::setCentralWidget(mpCanvas);
  logMessage("create canvas", Qgis::MessageLevel::Success);
  mpLeftDockWidget = new LeftDockWidget(this);
  logMessage("create left dock widget", Qgis::MessageLevel::Success);
  mpRightDockWidget = new RightDockWidget(this);
  logMessage("create right dock widget", Qgis::MessageLevel::Success);
  createSlots();
  logMessage("create main window", Qgis::MessageLevel::Success);

  connect(mpRoutePlanner.get(), &RoutePlanner::dataUpdated,
          mpOpenGLWidget.get(), QOverload<>::of(&QOpenGLWidget::update));
  logMessage("connect route planner signal to update mapcanvas",
             Qgis::MessageLevel::Success);
  createJoyDockWidgets();
  logMessage("create dock widgets", Qgis::MessageLevel::Success);
}

MainWindow::~MainWindow() {
  logMessage("MainWindow destroyed", Qgis::MessageLevel::Info);
}

static QSize setWindowSize(QRect screenGeometry, int maxWidth, int maxHeight,
                           int minWidth, int minHeight) {
  // calc current screen size
  double width_d = screenGeometry.width() * 0.8;
  double height_d = screenGeometry.height() * 0.8;

  int width = qMin(maxWidth, static_cast<int>(width_d));
  int height = qMin(maxHeight, static_cast<int>(height_d));
  width = qMax(minWidth, width);
  height = qMax(minHeight, height);
  return QSize(width, height);
}
void MainWindow::initWindowStatus() {
  StyleManager::initializeStyle(); // initialize global style
  QScreen *screen = QApplication::primaryScreen();
  QRect screenGeometry = screen->geometry();
  QSize windowSize = setWindowSize(screenGeometry, 2700, 1500, 800, 600);
  setMinimumSize(windowSize);
  int x = (screenGeometry.width() - windowSize.width()) / 2,
      y = (screenGeometry.height() - windowSize.height()) / 2;
  move(x, y);

  setWindowFlags(Qt::Window);
  setWindowTitle("3D Flight Simulation");
}

void MainWindow::Unrealized() {}

void MainWindow::createSlots() {
  logMessage("create slots", Qgis::MessageLevel::Info);
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

  connect(pBtnSetHome, &QPushButton::clicked, [this]() {
    if (!mpRoutePlanner->m_settingHomePointMode) {
      // enter setting home point mode
      mpRoutePlanner->setSettingHomePointMode(true);
      logMessage("enter setting home point mode", Qgis::MessageLevel::Success);
    }
  });
  connect(pBtnQueryParams, &QPushButton::clicked, this,
          &MainWindow::queryFlightParameters);
  connect(pBtnRefreshData, &QPushButton::clicked, this,
          &MainWindow::refreshBasicData);
  logMessage("connect left widget to slots", Qgis::MessageLevel::Success);

  // connect signal in main window constructor
  connect(mpStackedWidget, &QStackedWidget::currentChanged, this,
          [=](int index) {
            if (index == 0 &&
                ws::WindowManager::getInstance().getCurrentCanvas() ==
                    ws::CanvasType::ThreeD) { // assume OpenGLCanvas is the
                                              // first page (index 0)
              mpOpenGLWidget->setFocus();     // switch back to force focus
            }
          });
  logMessage("connect signal in main window constructor",
             Qgis::MessageLevel::Info);
  // ================= signal and slot connection =================

  connect(mpBtnReset, &QPushButton::clicked, this, &MainWindow::resetView);
  logMessage("connect reset view button to reset view",
             Qgis::MessageLevel::Info);
  connect(mpBtnSwitchTo3D, &QPushButton::clicked, this,
          &MainWindow::switchTo3D);
  logMessage("connect switch to 3D button to switch to 3D",
             Qgis::MessageLevel::Info);
  connect(mpBtnSwitchTo2D, &QPushButton::clicked, this,
          &MainWindow::switchTo2D);
  logMessage("connect switch to 2D button to switch to 2D",
             Qgis::MessageLevel::Info);

  connect(pBtnCreateRoute, &QPushButton::clicked, mpRoutePlanner.get(),
          &RoutePlanner::enterRoutePlanningMode); // start route planning
  logMessage("connect create route button to enter route planning mode",
             Qgis::MessageLevel::Info);
  connect(pBtnAddControlPoint, &QPushButton::clicked, [this]() {
    if (!mpRoutePlanner->m_isAddingControlPoint) {
      mpRoutePlanner->m_isAddingControlPoint =
          true; // enter "add control point" mode
    } else
      mpRoutePlanner->m_isAddingControlPoint = false; // exit
  });
  logMessage("connect add control point button to enter add control point mode",
             Qgis::MessageLevel::Info);
  connect(pBtnEditPoint, &QPushButton::clicked, [this]() {
    if (!mpRoutePlanner->m_editingMode) {
      mpRoutePlanner->m_editingMode = true; // enter "edit point" mode
    } else
      mpRoutePlanner->m_editingMode = false; // exit
  });
  logMessage("connect edit point button to enter edit point mode",
             Qgis::MessageLevel::Info);
  connect(pBtnGenerate, &QPushButton::clicked, [=]() {
    // mpOpenGLWidget->generateFlightRoute(pHeightSpin->value());
  });
  logMessage("connect generate flight route button to generate flight route",
             Qgis::MessageLevel::Info);
  /*
  connect(pHeightSpin, SIGNAL(valueChanged(double)), mpOpenGLWidget.get(),
          SLOT(updateFlightHeight(double))); //传递行高到MyOpenGLWidget类中
  logMessage("connect height spin box to update flight height",
  Qgis::MessageLevel::Info); connect(pWidthSpin, SIGNAL(valueChanged(double)),
  mpRoutePlanner.get(), SLOT(setScanSpacing(double)));
  //传递航带宽度到RoutePlanner类中 logMessage("connect width spin box to set
  scan spacing", Qgis::MessageLevel::Info);
*/
  /*
  // flight simulation related connections
  connect(pBtnStart, &QPushButton::clicked,
          [=]() { mpOpenGLWidget->startSimulation(pSpeedSpin->value()); });
  connect(pBtnPause, &QPushButton::clicked, mpOpenGLWidget.get(),
          &OpenGLCanvas::pauseSimulation);
  connect(pBtnResume, &QPushButton::clicked, mpOpenGLWidget.get(),
          &OpenGLCanvas::resumeSimulation);
  connect(pBtnReturn, &QPushButton::clicked, mpOpenGLWidget.get(),
          &OpenGLCanvas::returnToHome);
  connect(pBtnStop, &QPushButton::clicked, mpOpenGLWidget.get(),
          &OpenGLCanvas::stopSimulation);
  connect(pBaseHeightSpin, SIGNAL(valueChanged(double)), mpOpenGLWidget.get(),
          SLOT(ws::FlightManager::getInstance().setBaseHeight(double)));
  logMessage("connected all slots on main window", Qgis::MessageLevel::Success);
  */
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

void MainWindow::showUserManual() {
  logMessage("show user manual", Qgis::MessageLevel::Info);
  QDialog *manualDialog = new QDialog(this);
  manualDialog->setWindowTitle(tr("User Manual"));
  manualDialog->setWindowFlag(Qt::WindowContextHelpButtonHint, true);

  QTextEdit *textEdit = new QTextEdit(manualDialog);
  
  QFile manualFile("resources/user_manual.txt");
  QString content;
  if (manualFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&manualFile);
    in.setCodec("UTF-8");
    content = in.readAll();
    manualFile.close();
  } else {
    content = tr("无法加载用户手册文件。");
    logMessage("Failed to load user manual file", Qgis::MessageLevel::Warning);
  }

  QFile copyrightFile("resources/copyright.txt");
  if (copyrightFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&copyrightFile);
    in.setCodec("UTF-8");
    content += "\n\n" + in.readAll();
    copyrightFile.close();
  } else {
    logMessage("无法加载开发者信息", Qgis::MessageLevel::Warning);
  }

  textEdit->setText(content);
  textEdit->setReadOnly(true);
  textEdit->setFixedSize(800, 600);
  textEdit->setWhatsThis(tr("If you have any questions or issues, please contact the developer at: 18372124178."));

  QVBoxLayout *dialogLayout = new QVBoxLayout(manualDialog);
  dialogLayout->addWidget(textEdit);
  manualDialog->setLayout(dialogLayout);

  manualDialog->exec();
  logMessage("show user manual", Qgis::MessageLevel::Success);
}