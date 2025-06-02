#include "RightDockWidget.h"
#include "../core/WorkspaceState.h"
#include "../log/QgisDebug.h"
#include <QFileDialog>

RightDockWidget::~RightDockWidget() {
  logMessage("RightDockWidget destroyed", Qgis::MessageLevel::Success);
}

void FileTreeWidget::createSlots() {
  connect(mpSelectDirectoryButton, &QToolButton::clicked, this,
          &FileTreeWidget::onSelectDirectoryClicked);
  connect(mpTreeWidget, &QTreeWidget::itemDoubleClicked, this,
          &FileTreeWidget::onTreeItemDoubleClicked);
  connect(mpTreeWidget, &QTreeWidget::itemExpanded, this,
          &FileTreeWidget::onTreeItemExpanded);
}

void FileTreeWidget::createSelectDirectoryButton() {
  // create select directory button
  mpSelectDirectoryButton = new QToolButton(this);
  mpSelectDirectoryButton->setObjectName("selectDirectoryButton");
  mpSelectDirectoryButton->setText(tr("Select Directory"));
  mpSelectDirectoryButton->move(0, 0);
  logMessage("connect select directory button to onSelectDirectoryClicked",
             Qgis::MessageLevel::Info);
}

void FileTreeWidget::createTreeWidget() {
  mpTreeWidget = new QTreeWidget(this);
  mpTreeWidget->setObjectName("fileTreeWidget");
  mpTreeWidget->setHeaderLabel(tr("File List"));
  mpRootItem = nullptr;
  QString dirPath = ws::PathManager::getInstance().getRootDir();
  logMessage("init empty file tree", Qgis::MessageLevel::Info);
  loadDirectoryFiles(dirPath);
}

FileTreeWidget::FileTreeWidget(QWidget *parent) : QWidget(parent) {
  setObjectName("fileTreeWidget");
  
  createSelectDirectoryButton();
  createTreeWidget();
  
  mpMainLayout = new QVBoxLayout(this);
  mpMainLayout->addWidget(mpSelectDirectoryButton);
  mpMainLayout->addWidget(mpTreeWidget);
  createSlots();
  logMessage("create file tree", Qgis::MessageLevel::Success);
}

RightDockWidget::RightDockWidget(QWidget *parent) : QDockWidget(parent) {
  setObjectName("pRightDockWidget");
  setAllowedAreas(Qt::RightDockWidgetArea);
  setFeatures(QDockWidget::DockWidgetMovable |
              QDockWidget::DockWidgetFloatable);

  mpMainContainer = new QWidget(this);
  setWidget(mpMainContainer);
  mpMainLayout = new QVBoxLayout(mpMainContainer);
  mpMainLayout->setContentsMargins(0, 0, 0, 0);
  mpMainLayout->setSpacing(0);
  mpFileTreeWidget = new FileTreeWidget(mpMainContainer);
  mpMainLayout->addWidget(mpFileTreeWidget);
  mpToolTreeWidget = new ToolTreeWidget(mpMainContainer);
  mpMainLayout->addWidget(mpToolTreeWidget);
  mJoystickWidget = new JoyDockWidget(mpMainContainer);
  mpMainLayout->addWidget(mJoystickWidget);

  logMessage("right dock widget created", Qgis::MessageLevel::Success);
}

JoyDockWidget::JoyDockWidget(QWidget *parent) : QWidget(parent) {
  setObjectName("joyDockWidget");
  
  mpMainLayout = new QVBoxLayout(this);
  mpMainLayout->setContentsMargins(0, 0, 0, 0);
  
  mpControlPanel = new QWidget(this);
  mpControlPanel->setObjectName("controlPanel");
  
  QVBoxLayout* controlLayout = new QVBoxLayout(mpControlPanel);
  controlLayout->setContentsMargins(20, 20, 20, 20);
  controlLayout->setSpacing(30);

  mpJoystickLayout = new QHBoxLayout();
  mpJoystickLayout->setSpacing(40);

  const int joystickSize = 180;
  QString joystickStyle = "background-color: #333333;"
                         "border-radius: " +
                         QString::number(joystickSize / 2) + "px;";

  controlLayout->addLayout(mpJoystickLayout);

  mpButtonLayout = new QHBoxLayout();
  mpButtonLayout->setAlignment(Qt::AlignHCenter);
  mpButtonLayout->setSpacing(30);

  mpManualBtn = new QPushButton("手动模式", mpControlPanel);
  mpAutoBtn = new QPushButton("自动模式", mpControlPanel);

  QString buttonStyle = "QPushButton {"
                       "  background-color: #4A4A4A;"
                       "  border: 2px solid #5A5A5A;"
                       "  border-radius: 8px;"
                       "  padding: 15px 30px;"
                       "  min-width: 140px;"
                       "  min-height: 45px;"
                       "  font: bold 16px '微软雅黑';"
                       "}"
                       "QPushButton:hover {"
                       "  background-color: #5A5A5A;"
                       "}";
  mpManualBtn->setStyleSheet(buttonStyle);
  mpAutoBtn->setStyleSheet(buttonStyle);

  mpButtonLayout->addWidget(mpManualBtn);
  mpButtonLayout->addWidget(mpAutoBtn);
  
  controlLayout->addLayout(mpButtonLayout);

  mpMainLayout->addWidget(mpControlPanel);

  logMessage("create joy dock widget", Qgis::MessageLevel::Success);
}

void FileTreeWidget::onTreeItemExpanded(QTreeWidgetItem *item) {
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

QString FileTreeWidget::getItemFullPath(QTreeWidgetItem *item) {
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

void FileTreeWidget::onTreeItemDoubleClicked(QTreeWidgetItem *item,
                                             int column) {
  if (!item)
    return;

  QString filePath = getItemFullPath(item);
  QFileInfo fileInfo(filePath);

  if (fileInfo.isFile() && fileInfo.suffix().toLower() == "obj") {
    emit loadModel(filePath);
  }
}

void FileTreeWidget::onSelectDirectoryClicked() {
  // open folder selection dialog
  QString currentDir = ws::PathManager::getInstance().getRootDir();
  QString dirPath = QFileDialog::getExistingDirectory(
      this, tr("Select Directory"), currentDir);
  logMessage(QString("select file list directory: %1").arg(dirPath),
             Qgis::MessageLevel::Info);
  if (!dirPath.isEmpty())
    loadDirectoryFiles(dirPath); // call loadDirectoryFiles to load selected directory
  logMessage("select file list directory", Qgis::MessageLevel::Success);
}
// load file list of specified directory to QTreeWidget
void FileTreeWidget::loadDirectoryFiles(const QString &path) {
  QDir dir(path);
  if (!dir.exists()){
    logMessage(QString("directory not exists: %1").arg(path),
               Qgis::MessageLevel::Critical);
    return;
  }

  if (mpTreeWidget)
    mpTreeWidget->clear();

  logMessage("add new root item", Qgis::MessageLevel::Info);
  mpRootItem = new QTreeWidgetItem(mpTreeWidget);
  mpRootItem->setText(0, dir.dirName());
  logMessage("load directory level", Qgis::MessageLevel::Info);
  loadDirectoryLevel(mpRootItem, path, 1, 3);

  logMessage("load file list of specified directory to tree widget",
             Qgis::MessageLevel::Success);
}

void FileTreeWidget::loadDirectoryLevel(QTreeWidgetItem *parentItem,
                                        const QString &path, int level,
                                        int maxLevel) {
  if (level > maxLevel)
    return;

  QDir dir(path);
  QFileInfoList files =
      dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

  for (const QFileInfo &fileInfo : files) {
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

void ToolTreeWidget::createSlots() {
  connect(this, &QTreeWidget::itemClicked, this,
          &ToolTreeWidget::onTreeItemClicked);
}

ToolTreeWidget::ToolTreeWidget(QWidget *parent) : QTreeWidget(parent) {
  setObjectName("toolTreeWidget");
  setHeaderLabel(tr("Tool Box"));
  setHeaderHidden(true); // hide header

  mpRoutePlanningToolbox = new RoutePlanningToolbox(this);
  mpSimulationToolbox = new SimulationToolbox(this);
  mpParameterToolbox = new ParameterToolbox(this);

  createSlots();
  logMessage("create tool box", Qgis::MessageLevel::Success);
}

void ToolTreeWidget::onTreeItemClicked(QTreeWidgetItem *item, int column) {
  if (!item)
    return;

  if (mpRoutePlanningToolbox->isCreateRoute(item)) {
    emit createRoute();
  } else if (mpRoutePlanningToolbox->isEditRoute(item)) {
    emit editRoute();
  } else if (mpSimulationToolbox->isStart(item)) {
    emit simulationStart();
  } else if (mpSimulationToolbox->isPause(item)) {
    emit simulationPause();
  } else if (mpSimulationToolbox->isResume(item)) {
    emit simulationResume();
  } else if (mpSimulationToolbox->isReturn(item)) {
    emit simulationReturnHome();
  } else if (mpSimulationToolbox->isStop(item)) {
    emit simulationStop();
  } else if (mpParameterToolbox->isFlightParams(item)) {
    emit queryFlightParams();
  } else if (mpParameterToolbox->isEnvironmentParams(item)) {
    emit queryEnvParams();
  }
}

RoutePlanningToolbox::RoutePlanningToolbox(QTreeWidget *parent)
    : QTreeWidgetItem(parent) {
  setText(0, ("Route Planning"));
  mpCreateRoute = new QTreeWidgetItem(this);
  mpCreateRoute->setText(0, ("Create Route"));
  mpEditRoute = new QTreeWidgetItem(this);
  mpEditRoute->setText(0, ("Edit Route"));
}

SimulationToolbox::SimulationToolbox(QTreeWidget *parent)
    : QTreeWidgetItem(parent) {
  setText(0, ("Simulation"));
  mpStart = new QTreeWidgetItem(this);
  mpStart->setText(0, ("Start"));
  mpPause = new QTreeWidgetItem(this);
  mpPause->setText(0, ("Pause"));
  mpResume = new QTreeWidgetItem(this);
  mpResume->setText(0, ("Resume"));
  mpReturn = new QTreeWidgetItem(this);
  mpReturn->setText(0, ("Return"));
  mpStop = new QTreeWidgetItem(this);
  mpStop->setText(0, ("Stop"));
}

ParameterToolbox::ParameterToolbox(QTreeWidget *parent)
    : QTreeWidgetItem(parent) {
  setText(0, ("Parameter"));
  mpFlightParams = new QTreeWidgetItem(this);
  mpFlightParams->setText(0, ("Flight Parameters"));
  mpEnvironmentParams = new QTreeWidgetItem(this);
  mpEnvironmentParams->setText(0, ("Environment Parameters"));
}

void JoyDockWidget::handleJoystickMove(float dx, float dy) {
    // 处理摇杆移动
    // TODO: 实现摇杆控制逻辑
    logMessage("Joystick moved: dx=" + QString::number(dx) + ", dy=" + QString::number(dy), 
               Qgis::MessageLevel::Info);
}

void JoyDockWidget::switchToManualMode() {
    // 切换到手动模式
    mpManualBtn->setEnabled(false);
    mpAutoBtn->setEnabled(true);
    logMessage("Switched to manual mode", Qgis::MessageLevel::Info);
}

void JoyDockWidget::switchToAutoMode() {
    // 切换到自动模式
    mpManualBtn->setEnabled(true);
    mpAutoBtn->setEnabled(false);
    logMessage("Switched to auto mode", Qgis::MessageLevel::Info);
}