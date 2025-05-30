#include "WorkspaceState.h"
#include <QProcessEnvironment>

namespace ws {
    void initializeWorkspaceState() {
        PathManager::getInstance();
    }
}

ws::PathManager::PathManager() {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    mRootDir = env.value("HOME", env.value("USERPROFILE"));
    // mRootDir = QString::fromStdString(getenv("HOME"));
}

ws::PathManager::~PathManager() {
    mObjTexturePairs.clear();
}

void ws::PathManager::findAllObjAndTexturePaths() {
    QDir dir(mRootDir);
    QStringList objFiles = dir.entryList(QStringList() << "*.obj", QDir::Files);
    QStringList textureFiles = dir.entryList(QStringList() << "*.jpg", QDir::Files);
}

ObjTexturePair ws::PathManager::getObjTexturePair(int index) const {
    if (index < 0 || index >= mObjTexturePairs.size()) {
        throw std::out_of_range("Index out of range");
    }
    return mObjTexturePairs[index];
}

ws::EnvManager::EnvManager() {
    mWeather = WeatherType::Sunny;
    mTemperature = 25.0;
    mPressure = 1013.25;
}
ws::EnvManager::~EnvManager() {}

ws::FlightManager::FlightManager() {
    mFlightSpeed = 10.0;
    mFlightAltitude = 100.0;
    mFlightBattery = 100.0;
    mBaseHeight = 0.0;
}
ws::FlightManager::~FlightManager() {
  mFlightPath.clear();
}

QObject* ws::WindowManager::pDefaultObject = nullptr;
ws::WindowManager::WindowManager() {
    mCurrentCanvas = CanvasType::ThreeD;
    is3DMapInited = false;
    is2DMapInited = false;
    if (!pDefaultObject)
        pDefaultObject = new QObject();
    mBounds = Bounds();
}
ws::WindowManager::~WindowManager() {
    delete pDefaultObject;
}
/*

void ws::FlightManager::generateFlightRoute(float height) // 生成航线
{
  m_routePlanner->m_editingMode = false;
}; // 生成航线

void ws::AnimationManager::startSimulation(float speed) {
  if (!m_routePlanner)
    return;

  m_flightPath = m_routePlanner->routePath();
  if (m_flightPath.size() < 2)
    return;

  m_animationProgress = 0.0f;
  m_isAnimating = true;
  m_animationTimer->start(16); // ~60 FPS
}

void ws::AnimationManager::pauseSimulation() { m_animationTimer->stop(); }

void ws::AnimationManager::resumeSimulation() { m_animationTimer->start(); }

void ws::AnimationManager::returnToHome() {
  m_animationTimer->stop();
  m_animationProgress = 0.0f;
  update();
}

void ws::AnimationManager::updateAnimation() {
  if (!m_isAnimating)
    return;

  // 计算新进度（示例使用固定速度，可根据实际速度参数调整）
  m_animationProgress += 0.002f;

  if (m_animationProgress >= 1.0f) {
    m_animationProgress = 1.0f;
    m_animationTimer->stop();
  }
  double baseHeight = ws::FlightManager::getInstance().getBaseHeight();
  float currentHeight = baseHeight + mfFlightHight + baseHeight;
  // 计算当前路径位置
  int index = static_cast<int>((m_flightPath.size() - 1) * m_animationProgress);
  float t = (m_flightPath.size() - 1) * m_animationProgress - index;
  index = qMin(index, m_flightPath.size() - 2);

  //   // 计算当前路径位置时应用模型旋转
  QVector3D start = mModelView * m_flightPath[index];   // 应用旋转
  QVector3D end = mModelView * m_flightPath[index + 1]; // 应用旋转
  m_aircraftPosition = start + t * (end - start);
  m_aircraftPosition.setZ(currentHeight);
  // 计算方向
  QVector3D direction = (end - start).normalized();
  float yaw = qRadiansToDegrees(atan2(direction.y(), direction.x()));
  m_aircraftOrientation = QQuaternion::fromEulerAngles(0, 0, -yaw);

  update();
}

void ws::AnimationManager::drawAircraft(const QVector3D &position,
                                  const QQuaternion &orientation) {
  mModelShader->bind();

  QMatrix4x4 model;
  model.translate(position);
  model.rotate(orientation);
  model.scale(5.0f); // 调整飞机大小

  QMatrix4x4 view;
  if (m_cameraFollowAircraft) {
    // 调整相机位置到无人机后上方，实现倾斜视角
    QVector3D cameraOffset =
        QVector3D(0, 30, 50); // Y分量控制高度，Z分量控制距离
    QVector3D cameraPosition =
        m_aircraftPosition + m_aircraftOrientation * cameraOffset;

    view.lookAt(cameraPosition,     // 相机位置
                m_aircraftPosition, // 目标点（无人机位置）
                m_aircraftOrientation *
                    QVector3D(0, 1, 1)); // 保持相机的上方向与无人机一致
  } else {
    view.translate(0, 0, mfDistance);
    view = view * mModelView;
  }

  QMatrix4x4 mvp = mProjection * view * model; // 确保应用模型矩阵

  mModelShader->setUniformValue("mvp", mvp);

  // 使用简单立方体作为临时飞机模型
  static QVector<Vertex> aircraftVertices = createAircraftModel();

  // 创建并绑定临时VAO/VBO
  QOpenGLVertexArrayObject::Binder vaoBinder(&mVAO);
  mVBO.bind();
  mVBO.allocate(aircraftVertices.constData(),
                aircraftVertices.size() * sizeof(Vertex));

  glDrawArrays(GL_TRIANGLES, 0, aircraftVertices.size());

  mVBO.release();
  mModelShader->release();
}

QVector<MyOpenGLWidget::Vertex> ws::AnimationManager::createAircraftModel() {
  QVector<Vertex> vertices;

  // 机身（使用红色纹理坐标区域）
  vertices << Vertex(QVector3D(-0.5, -0.5, 0.0), QVector2D(1.0, 0.0)) // 右下红
           << Vertex(QVector3D(0.5, -0.5, 0.0), QVector2D(1.0, 0.0)) // 左下红
           << Vertex(QVector3D(0.0, 1.0, 0.0), QVector2D(1.0, 0.0)); // 顶部红

  // 机翼（使用蓝色纹理坐标区域）
  vertices << Vertex(QVector3D(-1.0, 0.0, 0.0), QVector2D(0.0, 1.0)) // 左翼蓝
           << Vertex(QVector3D(1.0, 0.0, 0.0), QVector2D(0.0, 1.0)) // 右翼蓝
           << Vertex(QVector3D(0.0, 0.0, 1.0), QVector2D(0.0, 1.0)); // 顶部蓝

  return vertices;
}
void ws::AnimationManager::stopSimulation() {
  m_animationTimer->stop();
  m_isAnimating = false;
  m_animationProgress = 0.0f;
  update();
}
*/
