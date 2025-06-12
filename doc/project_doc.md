# 整体概述

本项目是一个校园三维无人机模拟平台，提供三维场景下的无人机航线规划与飞行仿真功能，支持 OBJ 格式 3D 模型加载与展示、二维地图联动显示、交互式航线规划、三维飞行仿真以及多视角观察等功能。

## 功能模块和类说明

### 1. 主窗口模块 (`MainWindow`)
- **文件**：`MainWindow.cpp`, `MainWindow.h`, `MainWindow.ui`
- **功能**：程序的主窗口，负责显示用户界面和处理用户操作。
- **接口用例**：
  - `showUserManual()`: 显示用户手册对话框。
```cpp
MainWindow& w = MainWindow::getInstance();
w.showUserManual();
```

### 2. 日志模块 (`QgisDebug`)
- **文件**：`log/QgisDebug.cpp`, `log/QgisDebug.h`
- **功能**：提供日志记录功能，用于记录程序运行过程中的信息、警告和错误。
- **接口用例**：
  - `logMessage(const QString& message, Qgis::MessageLevel level)`: 记录日志信息。
```cpp
logMessage("Starting application...", Qgis::MessageLevel::Info);
```

### 3. OpenGL 模块 (`opengl`)
- **文件**：`opengl/Camera.cpp`, `opengl/Camera.h`, `opengl/Primitive.cpp`, `opengl/Primitive.h`
- **功能**：负责处理 OpenGL 渲染相关的操作，包括相机控制和基本图元绘制。
- **类说明**：
  - **`Camera`**: 相机类，用于控制视角。
  - **`Primitive`**: 基本图元类，是其他图元类的基类。
    - **接口用例**：
```cpp
// 创建一个基本图元
QVector<QVector3D> vertices;
// 初始化 vertices...
Primitive primitive(GL_TRIANGLES, vertices, 3);
```
  - **`ColorPrimitive`**: 带颜色的图元类，继承自 `Primitive`。
    - **接口用例**：
```cpp
QVector4D color(1.0f, 0.0f, 0.0f, 1.0f);
ColorPrimitive colorPrimitive(GL_LINES, vertices, color);
```
  - **`BasePlane`**: 基础平面类，继承自 `ColorPrimitive`。
    - **接口用例**：
```cpp
Bounds bounds;
// 初始化 bounds...
double baseHeight = 0.0;
BasePlane basePlane(bounds, baseHeight, color);
```
  - **`RoutePath`**: 航线路径类，继承自 `ColorPrimitive`。
    - **接口用例**：
```cpp
RoutePath routePath(vertices, color);
```
  - **`OrientLine`**: 方向线类，继承自 `ColorPrimitive`。
    - **接口用例**：
```cpp
OrientLine orientLine(vertices, color);
```
  - **`ControlPoints`**: 控制点类，继承自 `ColorPrimitive`。
    - **接口用例**：
```cpp
ControlPoints controlPoints(vertices, color);
```
  - **`SinglePoint`**: 单点类，继承自 `ColorPrimitive`。
    - **接口用例**：
```cpp
QVector3D vertex(0.0f, 0.0f, 0.0f);
SinglePoint singlePoint(vertex, color);
```
  - **`ConvexHull`**: 凸包类，继承自 `ColorPrimitive`。
    - **接口用例**：
```cpp
ConvexHull convexHull(vertices, color);
```

### 4. 核心模块 (`core`)
- **文件**：`core/Model.cpp`, `core/Model.h`, `core/RoutePlanner.cpp`, `core/RoutePlanner.h`, `core/SharedContextManager.cpp`, `core/SharedContextManager.h`, `core/WorkspaceState.cpp`, `core/WorkspaceState.h`
- **功能**：包含程序的核心逻辑，如模型管理、航线规划和工作空间状态管理。
- **类说明**：
  - **`Model`**: 模型类，负责管理 3D 模型。
  - **`RoutePlanner`**: 航线规划类，负责航线的创建和编辑。
  - **`SharedContextManager`**: 共享上下文管理类，用于管理共享的上下文信息。
  - **`WorkspaceState`**: 工作空间状态类，用于保存和恢复工作空间的状态。

### 5. 图形用户界面模块 (`gui`)
- **文件**：`gui/LeftDockWidget.cpp`, `gui/LeftDockWidget.h`, `gui/RightDockWidget.cpp`, `gui/RightDockWidget.h`, `gui/Menu.cpp`, `gui/Menu.h`, `gui/OpenGLCanvas.cpp`, `gui/OpenGLCanvas.h`, `gui/StyleManager.cpp`, `gui/StyleManager.h`
- **功能**：提供用户界面相关的组件和功能。
- **类说明**：
  - **`LeftDockWidget`**: 左侧控制面板类，包含视图切换、航线规划和仿真控制等功能。
    - **接口用例**：
```cpp
LeftDockWidget leftDockWidget;
leftDockWidget.createScrollArea(parent);
leftDockWidget.createDockContent(parent);
```
  - **`RightDockWidget`**: 右侧属性面板类，包含文件树和工具箱等功能。
    - **接口用例**：
```cpp
RightDockWidget rightDockWidget;
```
  - **`FileTreeWidget`**: 文件树类，用于显示加载的 3D 模型目录结构。
    - **接口用例**：
```cpp
FileTreeWidget fileTreeWidget;
fileTreeWidget.loadDirectoryFiles(dirPath);
```
  - **`ToolTreeWidget`**: 工具箱类，用于快速访问常用工具。
    - **接口用例**：
```cpp
ToolTreeWidget toolTreeWidget;
```
  - **`JoyDockWidget`**: 操纵杆控制面板类，用于切换手动和自动模式。
    - **接口用例**：
```cpp
JoyDockWidget joyDockWidget;
```

### 6. 插件模块 (`plugin`)
- **文件**：`plugin.cpp`, `plugin.h`
- **功能**：提供 QGIS 插件相关的功能。
- **接口用例**：
  - `type()`: 返回插件类型。
```cpp
int pluginType = type();
```

### 7. 独立应用程序入口 (`main`)
- **文件**：`main.cpp`
- **功能**：独立应用程序的入口点，负责初始化 QGIS 应用程序和显示主窗口。
- **接口用例**：
```cpp
int main(int argc, char *argv[]) {
    QgsApplication app(argc, argv, true);
    QgsApplication::setPrefixPath(QGIS_PATH, true);
    QgsApplication::initQgis();

    MainWindow& w = MainWindow::getInstance();
    w.show();

    int exitCode = app.exec();

    w.close();
    w.release();

    QgsApplication::exitQgis();

    return exitCode;
}
```

# GUI概述
本项目的GUI部分主要由主窗口以及左右两个停靠窗口构成，以下详细阐述各部分的组织逻辑：

## 窗口逻辑

### 1. 主窗口 (`MainWindow`)
主窗口是整个应用程序的核心容器，负责统筹和管理其他窗口组件。其组织逻辑如下：
- **菜单栏 (`MenuBar`)**：位于主窗口顶部，提供了一系列的操作选项，涵盖项目管理、视图切换、模拟控制、航线规划、设置以及帮助等功能模块。
- **中央画布 (`Canvas`)**：作为主窗口的核心区域，用于展示三维场景和二维地图，为用户呈现直观的视觉效果。
- **左侧停靠窗口 (`LeftDockWidget`)**：位于主窗口左侧，包含多个功能分组，如视图切换、航线规划、飞行模拟以及参数查询等，为用户提供便捷的操作入口。
- **右侧停靠窗口 (`RightDockWidget`)**：位于主窗口右侧，包含文件树、工具箱和操纵杆控制面板，方便用户管理文件、使用工具和切换控制模式。

### 2. 左侧停靠窗口 (`LeftDockWidget`)
左侧停靠窗口由多个功能分组组成，每个分组对应不同的功能模块，具体如下：
- **视图切换分组 (`ViewGroup`)**：提供了切换3D视图、2D地图以及重置视图的按钮，方便用户调整视角。
- **航线规划分组 (`RouteGroup`)**：包含高度、宽度等参数设置，以及创建和编辑航线的按钮，支持用户进行航线规划。
- **飞行模拟分组 (`FlightSimGroup`)**：提供飞行速度设置和模拟控制按钮，如开始、暂停、继续、返回和停止等，方便用户进行飞行模拟操作。
- **飞行参数查询分组 (`FlightQueryGroup`)**：提供查询和设置飞行参数的功能，如飞行速度、最大高度和电池容量等。
- **环境参数查询分组 (`EnvQueryGroup`)**：提供查询和设置环境参数的功能，如天气、温度和气压等。

### 3. 右侧停靠窗口 (`RightDockWidget`)
右侧停靠窗口包含以下组件：
- **文件树 (`FileTreeWidget`)**：用于显示和管理加载的3D模型文件，支持用户选择目录并加载其中的OBJ文件。
- **工具箱 (`ToolTreeWidget`)**：提供常用工具的快捷访问，如航线创建、编辑和模拟控制等。
- **操纵杆控制面板 (`JoyDockWidget`)**：提供手动和自动模式切换按钮，方便用户控制无人机的飞行模式。

## 功能通讯

项目中的各个组件之间通过信号与槽机制进行功能通讯，实现了不同功能模块之间的交互和协作。以下是主要的通讯关系：

### 1. 主窗口与其他组件的通讯
- **菜单栏与画布的通讯**：菜单栏的视图切换、模拟控制等操作通过信号与槽机制传递给画布，实现相应的功能。例如，点击“3D视图”菜单项会触发 `switchTo3D` 信号，画布接收到该信号后切换到3D视图。
```cpp
connect(mpMenuBar, &MenuBar::switchTo3D, mpCanvas, &Canvas::switchTo3D);
```
- **菜单栏与其他管理器的通讯**：菜单栏的模拟控制、航线规划等操作通过信号与槽机制传递给相应的管理器，实现相应的功能。例如，点击“开始模拟”菜单项会触发 `simulationStart` 信号，动画管理器接收到该信号后开始模拟。
```cpp
connect(mpMenuBar, &MenuBar::simulationStart, &AnimationManager::getInstance(), &AnimationManager::startSimulation);
```

### 2. 左侧停靠窗口与其他组件的通讯
- **视图切换分组与画布的通讯**：视图切换分组的按钮操作通过信号与槽机制传递给画布，实现视图的切换和重置。例如，点击“3D视图”按钮会触发 `switchTo3D` 信号，画布接收到该信号后切换到3D视图。
```cpp
connect(mpLeftDockWidget->getViewGroup(), &ViewGroup::switchTo3D, mpCanvas, &Canvas::switchTo3D);
```
- **航线规划分组与航线规划器的通讯**：航线规划分组的创建和编辑航线按钮操作通过信号与槽机制传递给航线规划器，实现航线的创建和编辑。例如，点击“创建航线”按钮会触发 `createRoute` 信号，航线规划器接收到该信号后创建航线。
```cpp
connect(mpLeftDockWidget->getRouteGroup(), &RouteGroup::createRoute, &RoutePlanner::getInstance(), &RoutePlanner::createControlPoint);
```
- **飞行模拟分组与动画管理器的通讯**：飞行模拟分组的模拟控制按钮操作通过信号与槽机制传递给动画管理器，实现飞行模拟的控制。例如，点击“开始模拟”按钮会触发 `simulationStart` 信号，动画管理器接收到该信号后开始模拟。
```cpp
connect(mpLeftDockWidget->getFlightSimGroup(), &FlightSimGroup::simulationStart, &AnimationManager::getInstance(), &AnimationManager::startSimulation);
```
- **飞行参数查询分组与飞行管理器的通讯**：飞行参数查询分组的查询和设置飞行参数操作通过信号与槽机制传递给飞行管理器，实现飞行参数的查询和设置。例如，点击“查询飞行参数”按钮会触发 `queryFlightParams` 信号，飞行管理器接收到该信号后查询飞行参数。
```cpp
connect(mpLeftDockWidget->getFlightQueryGroup(), &FlightQueryGroup::queryFlightParams, &FlightManager::getInstance(), &FlightManager::queryFlightParameters);
```

### 3. 右侧停靠窗口与其他组件的通讯
- **文件树与画布的通讯**：文件树的文件选择操作通过信号与槽机制传递给画布，实现3D模型的加载。例如，双击文件树中的OBJ文件会触发 `loadModel` 信号，画布接收到该信号后加载相应的3D模型。
```cpp
connect(mpRightDockWidget->getFileTreeWidget(), &FileTreeWidget::loadModel, mpCanvas->getOpenGLWidget(), &OpenGLCanvas::loadModel);
```
- **工具箱与其他管理器的通讯**：工具箱的工具操作通过信号与槽机制传递给相应的管理器，实现相应的功能。例如，点击“创建航线”工具会触发 `createRoute` 信号，航线规划器接收到该信号后创建航线。
```cpp
connect(mpRightDockWidget->getToolTreeWidget(), &ToolTreeWidget::createRoute, &RoutePlanner::getInstance(), &RoutePlanner::createControlPoint);
```

# 全局状态管理

在`core`目录下的`WorkspaceState`文件（`WorkspaceState.h`和`WorkspaceState.cpp`）中定义了多个单例类，用于对项目的全局状态进行管理。下面将详细介绍这些单例类，包括它们的角色和接口。

### 1. `WindowManager`
- **角色**：负责管理窗口相关的状态和操作，包括当前画布类型、边界信息、键盘事件处理以及相机移动更新等。
- **接口**：
    - **获取单例实例**：
```cpp
static WindowManager& getInstance();
```
    - **设置和获取当前画布类型**：
```cpp
void setCurrentCanvas(CanvasType canvas);
CanvasType getCurrentCanvas() const;
```
    - **获取默认对象**：
```cpp
QObject* getDefaultObject() const;
```
    - **设置和获取边界信息**：
```cpp
const Bounds& getBounds() const;
void setBounds(const Bounds& bounds);
```
    - **键盘事件处理**：
```cpp
void keyPressEvent(QKeyEvent* event);
void keyReleaseEvent(QKeyEvent* event);
bool isKeyPressed(int key) const;
```
    - **更新相机移动**：
```cpp
void updateCameraMovement();
```
    - **设置和获取编辑模式**：
```cpp
bool isEditing() const;
void setEditing(bool editing);
```

### 2. `PathManager`
- **角色**：管理文件路径相关信息，包括根目录的设置、查找所有OBJ和纹理文件路径以及存储OBJ和纹理文件对。
- **接口**：
    - **获取单例实例**：
```cpp
static PathManager& getInstance();
```
    - **设置和获取根目录**：
```cpp
void setRootDir(const QString& rootDir);
QString getRootDir() const;
```
    - **查找所有OBJ和纹理文件路径**：
```cpp
void findAllObjAndTexturePaths();
```
    - **添加和获取OBJ和纹理文件对**：
```cpp
void addObjTexturePair(const ObjTexturePair& objTexturePair);
QList<ObjTexturePair> getObjTexturePairs() const;
ObjTexturePair getObjTexturePair(int index) const;
```

### 3. `EnvManager`
- **角色**：管理环境相关的状态和操作，包括天气类型、温度和气压的设置和获取，以及生成随机天气。
- **接口**：
    - **获取单例实例**：
```cpp
static EnvManager& getInstance();
```
    - **设置和获取天气、温度和气压**：
```cpp
void setWeather(WeatherType weather);
void setTemperature(double temperature);
void setPressure(double pressure);
WeatherType getWeatherType() const;
QString getWeatherString() const;
double getTemperature() const;
double getPressure() const;
```
    - **生成随机天气**：
```cpp
void generateRandomWeather();
```

### 4. `FlightManager`
- **角色**：管理飞行相关的状态和操作，包括飞行速度、电池电量、高度、位置等参数的设置和获取，以及手动模式的切换和飞行参数查询。
- **接口**：
    - **获取单例实例**：
```cpp
static FlightManager& getInstance();
```
    - **设置和获取飞行参数**：
```cpp
void setFlightSpeed(double speed);
double getFlightSpeed() const;
void setFlightBattery(double battery);
double getFlightBattery() const;
void setBaseHeight(double height);
double getBaseHeight() const;
void setPorision(QVector3D newPosition);
QVector3D getPosition() const;
void setMaxAltitude(double alititude);
double getMaxAltitude() const;
```
    - **设置和获取手动模式**：
```cpp
void setManualMode(bool manual);
bool isManualMode() const;
```
    - **查询飞行参数**：
```cpp
QString queryFlightParameters();
```

### 5. `AnimationManager`
- **角色**：管理动画相关的状态和操作，包括动画速度的设置、动画的开始、暂停、继续、返回和停止等。
- **接口**：
    - **获取单例实例**：
```cpp
static AnimationManager& getInstance();
```
    - **设置和获取动画速度**：
```cpp
void setAnimationSpeed(double speed);
double getAnimationSpeed() const;
```
    - **判断动画是否正在进行**：
```cpp
bool isAnimating() const;
```
    - **动画控制操作**：
```cpp
void startSimulation();
void pauseSimulation();
void resumeSimulation();
void returnToHome();
void stopSimulation();
```

这些单例类通过各自的接口提供了对项目全局状态的统一管理，使得不同模块可以方便地访问和修改相关状态。同时，单例模式确保了每个类在整个项目中只有一个实例，避免了资源的浪费和状态的不一致。

# OpenGL画布

在 `OpenGLCanvas` 文件中，`OpenGLCanvas` 类继承自 `QOpenGLWidget` 并实现了 OpenGL 绘制的核心逻辑。下面将详细介绍其绘制逻辑的流程：

## 1. 初始化阶段
- **构造函数 (`OpenGLCanvas::OpenGLCanvas`)**：
    - 设置 OpenGL 上下文的格式，包括版本、深度缓冲区、模板缓冲区、采样数等。
    - 设置焦点策略并获取焦点。
    - 创建一个定时器 `updateTimer`，每隔 16 毫秒触发一次 `QOpenGLWidget::update` 函数，用于定时更新绘制。
    - 根据窗口的宽度和高度设置相机的宽高比。
```cpp
OpenGLCanvas::OpenGLCanvas(QWidget *parent) : QOpenGLWidget(parent) {
    QSurfaceFormat format;
    format.setVersion(4, 1);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSamples(4);
    format.setOption(QSurfaceFormat::DebugContext);
    setFormat(format);
    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    updateTimer = std::make_unique<QTimer>();
    updateTimer->setInterval(16);
    updateTimer->moveToThread(QApplication::instance()->thread());
    connect(updateTimer.get(), &QTimer::timeout, this,
            QOverload<>::of(&QOpenGLWidget::update), Qt::QueuedConnection);
    updateTimer->start();

    double width = static_cast<double>(this->width());
    double height = static_cast<double>(this->height());
    Camera::getInstance().setAspectRatio(width / height);
}
```
- **`initializeGL` 函数**：
    - 检查 OpenGL 上下文的有效性。
    - 初始化 OpenGL 函数。
    - 输出 OpenGL 版本信息。
    - 启用深度测试和混合功能。
    - 初始化共享上下文管理器。
    - 创建 `OpenGLScene` 对象。
    - 设置 `RoutePlanner` 的上下文。
```cpp
void OpenGLCanvas::initializeGL() {
    if (!context()->isValid()) {
        logMessage("Invalid OpenGL context", Qgis::MessageLevel::Critical);
        return;
    }
    initializeOpenGLFunctions();

    QString version = QString::fromUtf8((const char *)glGetString(GL_VERSION));
    logMessage("OpenGL Version: " + version, Qgis::MessageLevel::Info);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(20.0f);

    if (!SharedContextManager::getInstance().initialize(context())) {
        logMessage("Failed to initialize shared context manager", Qgis::MessageLevel::Critical);
        return;
    }

    mpScene = std::make_unique<OpenGLScene>(context());
    RoutePlanner::getInstance().setContext(context());
    logMessage("OpenGL context initialized", Qgis::MessageLevel::Success);
}
```

## 2. 调整窗口大小阶段
- **`resizeGL` 函数**：
    - 根据窗口的新宽度和高度计算宽高比。
    - 更新相机的宽高比。
```cpp
void OpenGLCanvas::resizeGL(int w, int h) {
    double aspectRatio = static_cast<double>(w) / static_cast<double>(h);
    Camera::getInstance().setAspectRatio(aspectRatio);
}
```

## 3. 绘制阶段
- **`paintGL` 函数**：
    - 检查 OpenGL 上下文的有效性和窗口的可见性。
    - 清除颜色缓冲区和深度缓冲区。
    - 启用深度测试和混合功能。
    - 获取相机的视图矩阵和投影矩阵。
    - 调用 `OpenGLScene` 的 `paintScene` 函数进行场景绘制。
    - 检查相机的处理状态。
    - 发射 `refreash3DParms` 信号。
    - 检查 OpenGL 错误。
```cpp
void OpenGLCanvas::paintGL() {
    if (!isValid()) {
        logMessage("OpenGLCanvas is not valid", Qgis::MessageLevel::Critical);
        return;
    }
    if (!isVisible())
        return;
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Camera &camera = Camera::getInstance();
    QMatrix4x4 view = camera.viewMatrix();
    QMatrix4x4 projection = camera.projectionMatrix();

    if (mpScene) {
        mpScene->paintScene(view, projection);
    }

    camera.checkProcess();
    emit refreash3DParms();
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        logMessage(QString("OpenGL error in paintGL: %1").arg(err), 
                   Qgis::MessageLevel::Critical);
    }
}
```
- **`OpenGLScene::paintScene` 函数**：
    - 检查 OpenGL 上下文的有效性。
    - 启用面剔除功能，剔除背面。
    - 如果存在 `modelWidget`，调用其 `draw` 函数进行模型绘制。
    - 根据 `WindowManager` 的编辑状态，决定绘制选择线还是无人机模型。
    - 调用 `RoutePlanner` 的 `drawRoutes` 函数绘制航线。
```cpp
void OpenGLScene::paintScene(const QMatrix4x4 &view, const QMatrix4x4 &projection) {
    if (!QOpenGLContext::currentContext()) {
        logMessage("OpenGL context is not current", Qgis::MessageLevel::Critical);
        return;
    }
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    if (modelWidget) {
        modelWidget->draw(view, projection);
    }
    if (wsp::WindowManager::getInstance().isEditing()) {
      if (RoutePlanner::getInstance().getDrawMode() != RouteDrawMode::PREVIEWING_ROUTE)
        selectLine->draw(view, projection);
    }else{
      glCullFace(GL_FRONT);
      if (droneWidget) {
          droneWidget->draw(view, projection);
      }
    }
    RoutePlanner::getInstance().drawRoutes(view, projection);
}
```

## 4. 资源清理阶段
- **析构函数 (`OpenGLCanvas::~OpenGLCanvas`)**：
    - 停止定时器。
    - 清理 `OpenGLScene` 的资源。
```cpp
OpenGLCanvas::~OpenGLCanvas() {
    logMessage("ready to destroy OpenGLCanvas", Qgis::MessageLevel::Info);

    if (updateTimer) {
        updateTimer->stop();
        updateTimer = nullptr;
    }

    if (mpScene) {
        mpScene->cleanupResources();
        mpScene = nullptr;
    }

    logMessage("OpenGLCanvas destroyed", Qgis::MessageLevel::Success);
}
```

综上所述，`OpenGLCanvas` 的绘制逻辑流程包括初始化、调整窗口大小、绘制和资源清理四个阶段。在绘制阶段，通过 `paintGL` 函数调用 `OpenGLScene` 的 `paintScene` 函数，实现了模型、选择线、无人机模型和航线的绘制。