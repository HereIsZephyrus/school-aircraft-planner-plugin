#include "RightDockWidget.h"
#include "../log/QgisDebug.h"
#include "../core/WorkspaceState.h"

void FileTreeWidget::createSlots() {
  connect(mpSelectDirectoryButton, &QToolButton::clicked, this,
          &FileTreeWidget::onSelectDirectoryClicked);
  connect(mpFileTreeWidget, &QTreeWidget::itemExpanded, this,
          &FileTreeWidget::onTreeItemExpanded);
}

void FileTreeWidget::createSelectDirectoryButton() {
  // create select directory button
  mpSelectDirectoryButton = new QToolButton();
  mpSelectDirectoryButton->setObjectName("selectDirectoryButton");
  mpSelectDirectoryButton->setText(tr("Select Directory"));
  logMessage("connect select directory button to onSelectDirectoryClicked",
             Qgis::MessageLevel::Info);

  connect(this, &QTreeWidget::itemDoubleClicked, this, &FileTreeWidget::onTreeItemDoubleClicked);
  mpMainLayout->addWidget(mpSelectDirectoryButton);
}

FileTreeWidget::FileTreeWidget(QWidget *parent) : QTreeWidget(parent) {
  setObjectName("fileTreeWidget");
  setHeaderLabel(tr("File List"));

  createSelectDirectoryButton();
  createSlots();
  
  mpRootItem = nullptr;
  QString dirPath = ws::PathManager::getInstance().getRootDir();
  loadDirectoryFiles(dirPath);
  logMessage("create file tree", Qgis::MessageLevel::Success);
}

RightDockWidget::RightDockWidget(QWidget *parent) : QDockWidget(parent) {
  setObjectName("pRightDockWidget");
  setAllowedAreas(Qt::RightDockWidgetArea);
  setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

  mpMainContainer = new QWidget(this);
  setWidget(mpMainContainer);
  mpMainLayout = new QVBoxLayout(mpMainContainer);
  mpFileTreeWidget = new FileTreeWidget(mpMainContainer);
  mpMainLayout->addWidget(mpFileTreeWidget);
  mpToolTreeWidget = new ToolTreeWidget(mpMainContainer);
  mpMainLayout->addWidget(mpToolTreeWidget);
  mpJoyDockWidget = new JoyDockWidget(mpMainContainer);
  mpMainLayout->addWidget(mpJoyDockWidget);

  createSlots();
  logMessage("right dock widget created", Qgis::MessageLevel::Success);
}

JoyDockWidget::JoyDockWidget(QWidget *parent) : QDockWidget(parent) {
  setObjectName("joyDockWidget");
  mpMainLayout = new QVBoxLayout(this);
  mpControlDock = new QDockWidget(tr("Flight Control"), this);
  mpControlPanel = new QWidget(this);

  mpMainLayout = new QVBoxLayout(mpControlPanel);
  mpMainLayout->setContentsMargins(20, 20, 20, 20); // 增加边距
  mpMainLayout->setSpacing(30);                     // 摇杆与按钮间距

  mpJoystickLayout = new QHBoxLayout(mpControlPanel);
  mpJoystickLayout->setSpacing(40);

  // m_leftJoystick = new JoystickWidget();
  // m_rightJoystick = new JoystickWidget();

  // 设置大尺寸摇杆
  const int joystickSize = 180; // 摇杆尺寸
  // m_leftJoystick->setFixedSize(joystickSize, joystickSize);
  // m_rightJoystick->setFixedSize(joystickSize, joystickSize);

  QString joystickStyle = "background-color: #333333;"
                          "border-radius: " +
                          QString::number(joystickSize / 2) +
                          "px;";
  // m_leftJoystick->setStyleSheet(joystickStyle);
  // m_rightJoystick->setStyleSheet(joystickStyle);

  // joystickLayout->addWidget(m_leftJoystick);
  // joystickLayout->addWidget(m_rightJoystick);
  mpMainLayout->addLayout(mpJoystickLayout);

  // 按钮行（保持大尺寸）===================================
  mpButtonLayout = new QHBoxLayout(mpControlPanel);
  mpButtonLayout->setAlignment(Qt::AlignHCenter);
  mpButtonLayout->setSpacing(30); // 按钮间距

  mpManualBtn = new QPushButton("手动模式", mpControlPanel);
  mpAutoBtn = new QPushButton("自动模式", mpControlPanel);

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
  mpManualBtn->setStyleSheet(buttonStyle);
  mpAutoBtn->setStyleSheet(buttonStyle);

  mpButtonLayout->addWidget(mpManualBtn);
  mpButtonLayout->addWidget(mpAutoBtn);
  mpMainLayout->addLayout(mpButtonLayout);

  mpControlDock->setWidget(mpControlPanel);
  addDockWidget(Qt::RightDockWidgetArea, mpControlDock);
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

void FileTreeWidget::onTreeItemDoubleClicked(QTreeWidgetItem *item, int column) {
  if (!item)
    return;

  QString filePath = getItemFullPath(item);
  QFileInfo fileInfo(filePath);

  if (fileInfo.isFile() && fileInfo.suffix().toLower() == "obj") {
    mpOpenGLWidget->loadModel(filePath);
  }
}

void FileTreeWidget::onSelectDirectoryClicked() {
  // open folder selection dialog
  QString currentDir = ws::PathManager::getInstance().getRootDir();
  QString dirPath = QFileDialog::getExistingDirectory(
      this, tr("Select Directory"), currentDir);
  if (!dirPath.isEmpty())
    loadDirectoryFiles(dirPath); // call loadDirectoryFiles to load selected directory
  logMessage("select file list directory", Qgis::MessageLevel::Success);
}
// load file list of specified directory to QTreeWidget
void FileTreeWidget::loadDirectoryFiles(const QString &path) {
  QDir dir(path);
  if (!dir.exists())
    return;

  mpFileTreeWidget->clear();

  if (mpRootItem)
    delete mpRootItem;

  mpRootItem = new QTreeWidgetItem(mpFileTreeWidget);
  mpRootItem->setText(0, dir.dirName());
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

  foreach (const QFileInfo &fileInfo, files) {
    QTreeWidgetItem *item = new QTreeWidgetItem(parentItem);
    item->setText(0, fileInfo.fileName());

    if (fileInfo.isDir()) {
      if (level < maxLevel) {
        loadDirectoryLevel(item, fileInfo.absoluteFilePath(), level + 1, maxLevel);
      } else if (level == maxLevel) {
        new QTreeWidgetItem(item);
      }
    }
  }
}

void ToolTreeWidget::createSlots() {
  connect(this, &QTreeWidget::itemClicked, this, &ToolTreeWidget::onTreeItemClicked);
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

RoutePlanningToolbox::RoutePlanningToolbox(QTreeWidget *parent) : QTreeWidgetItem(parent) {
  setObjectName("routePlanningToolbox");
  setText(0, tr("Route Planning"));
  mpCreateRoute = new QTreeWidgetItem(this);
  mpCreateRoute->setText(0, tr("Create Route"));
  mpEditRoute = new QTreeWidgetItem(this);
  mpEditRoute->setText(0, tr("Edit Route"));
}

SimulationToolbox::SimulationToolbox(QTreeWidget *parent) : QTreeWidgetItem(parent) {
  setObjectName("simulationToolbox");
  setText(0, tr("Simulation"));
  mpStart = new QTreeWidgetItem(this);
  mpStart->setText(0, tr("Start"));
  mpPause = new QTreeWidgetItem(this);
  mpPause->setText(0, tr("Pause"));
  mpResume = new QTreeWidgetItem(this);
  mpResume->setText(0, tr("Resume"));
  mpReturn = new QTreeWidgetItem(this);
  mpReturn->setText(0, tr("Return"));
  mpStop = new QTreeWidgetItem(this);
  mpStop->setText(0, tr("Stop"));
}

ParameterToolbox::ParameterToolbox(QTreeWidget *parent) : QTreeWidgetItem(parent) {
  setObjectName("parameterToolbox");
  setText(0, tr("Parameter"));
  mpFlightParams = new QTreeWidgetItem(this);
  mpFlightParams->setText(0, tr("Flight Parameters"));
  mpEnvironmentParams = new QTreeWidgetItem(this);
  mpEnvironmentParams->setText(0, tr("Environment Parameters"));
}