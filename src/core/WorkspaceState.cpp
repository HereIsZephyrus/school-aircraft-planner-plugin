#include "WorkspaceState.h"
#include "../log/QgisDebug.h"
#include "../opengl/Camera.h"
#include <QProcessEnvironment>
#include <QRandomGenerator>
#include <QTimer>
#include <cstdlib>

static QString GetHomeDirectory() {
#ifdef _WIN32
  return QString(getenv("USERPROFILE"));
#else
  const char *home = getenv("HOME");
  return QString(home);
#endif
}

QObject *ws::WindowManager::pDefaultObject = nullptr;

void ws::initializeWorkspaceState() {
  PathManager::getInstance();
  logMessage("WorkspaceState initialized", Qgis::MessageLevel::Success);
}

ws::PathManager::PathManager() {
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  mRootDir = GetHomeDirectory();
}

ws::PathManager::~PathManager() { mObjTexturePairs.clear(); }

void ws::PathManager::findAllObjAndTexturePaths() {
  QDir dir(mRootDir);
  QStringList objFiles = dir.entryList(QStringList() << "*.obj", QDir::Files);
  QStringList textureFiles =
      dir.entryList(QStringList() << "*.jpg", QDir::Files);
}

ObjTexturePair ws::PathManager::getObjTexturePair(int index) const {
  if (index < 0 || index >= mObjTexturePairs.size()) {
    throw std::out_of_range("Index out of range");
  }
  return mObjTexturePairs[index];
  if (index < 0 || index >= mObjTexturePairs.size()) {
    throw std::out_of_range("Index out of range");
  }
  return mObjTexturePairs[index];
}

ws::EnvManager::EnvManager() {
  mWeather = WeatherType::Sunny;
  mTemperature = 25.0;
  mPressure = 1013.25;
  mWeather = WeatherType::Sunny;
  mTemperature = 25.0;
  mPressure = 1013.25;
}
ws::EnvManager::~EnvManager() {}

ws::FlightManager::FlightManager() {
  mFlightSpeed = 10.0;
  mFlightBattery = 100.0;
  mBaseHeight = 0.0;
  mFlightSpeed = 10.0;
  mFlightBattery = 100.0;
  mBaseHeight = 0.0;
  mMaxAlititude = maxBaseHeight;
}
ws::FlightManager::~FlightManager() { mFlightPath.clear(); }

ws::WindowManager::WindowManager()
    : QObject(nullptr), mCurrentCanvas(CanvasType::ThreeD),
      is3DMapInited(false), is2DMapInited(false) {
  // 初始化更新定时器
  mUpdateTimer = new QTimer(this);
  connect(mUpdateTimer, &QTimer::timeout, this,
          &WindowManager::updateCameraMovement);
  mUpdateTimer->start(16); // 约60fps的更新频率
  if (!pDefaultObject)
    pDefaultObject = new QObject();
  mBounds = Bounds();
}

ws::WindowManager::~WindowManager() {
  if (mUpdateTimer) {
    mUpdateTimer->stop();
    delete mUpdateTimer;
  }
  delete pDefaultObject;
}

void ws::WindowManager::keyPressEvent(QKeyEvent *event) {
  mKeyStates[event->key()] = true;
}

void ws::WindowManager::keyReleaseEvent(QKeyEvent *event) {
  mKeyStates[event->key()] = false;
}

bool ws::WindowManager::isKeyPressed(int key) const {
  return mKeyStates.value(key, false);
}

void ws::WindowManager::update3DCameraMovement() {
  float step = 0.5f;
  Camera &camera = Camera::getInstance();
  
  if (isKeyPressed(Qt::Key_W))
    camera.moveForward(step);
  if (isKeyPressed(Qt::Key_S))
    camera.moveBackward(step);
  if (isKeyPressed(Qt::Key_A))
    camera.moveLeft(step);
  if (isKeyPressed(Qt::Key_D))
    camera.moveRight(step);
  if (isKeyPressed(Qt::Key_Q))
    camera.moveUp(step);
  if (isKeyPressed(Qt::Key_E))
    camera.moveDown(step);
  if (isKeyPressed(Qt::Key_R))
    camera.resetView();
}

void ws::WindowManager::update2DCameraMovement() {
  return;
}

void ws::WindowManager::updateCameraMovement() {
  if (mCurrentCanvas == CanvasType::ThreeD) {
    update3DCameraMovement();
  }
  if (mCurrentCanvas == CanvasType::TwoD) {
    update2DCameraMovement();
  }
}

// add new slot function at the end of the file
QString ws::FlightManager::queryFlightParameters() {
  QString params = QString("Current Flight Parameters:\n"
                           "Speed: %1 m/s\n"
                           "Altitude: %2 m\n"
                           "Battery: %3%\n"
                           "Position:\n (%4, %5)")
                       .arg(mFlightSpeed, 0, 'f', 1)
                       .arg(mAircraftPosition.z(), 0, 'f', 1)
                       .arg(mFlightBattery, 0, 'f', 1)
                       .arg(mAircraftPosition.x(), 0, 'f', 4)
                       .arg(mAircraftPosition.y(), 0, 'f', 4);
  return params;
}

void ws::EnvManager::generateRandomWeather() {
  logMessage("generate random weather data", Qgis::MessageLevel::Info);
  WeatherType weather =
      static_cast<WeatherType>(QRandomGenerator::global()->bounded(5));

  ws::EnvManager &envManager = ws::EnvManager::getInstance();
  double temperature =
      QRandomGenerator::global()->bounded(envManager.minTemperature * 10,
                                          envManager.maxTemperature * 10) /
      10.0;

  double pressure =
      QRandomGenerator::global()->bounded(envManager.minPressure * 10,
                                          envManager.maxPressure * 10) /
      10.0;

  setWeather(weather);
  setTemperature(temperature);
  setPressure(pressure);
}

void ws::AnimationManager::startSimulation() {
  mIsAnimating = true;
  mIsPaused = false;
  mAnimationProgress = 0.0f;
}

void ws::AnimationManager::pauseSimulation() { mIsPaused = true; }

void ws::AnimationManager::resumeSimulation() { mIsPaused = false; }

void ws::AnimationManager::returnToHome() {
  mAnimationProgress = 0.0f;
  mIsAnimating = false;
  mIsPaused = false;
}


void ws::AnimationManager::stopSimulation() {
  mIsAnimating = false;
  mIsPaused = false;
  mAnimationProgress = 0.0f;
}

ws::AnimationManager::AnimationManager() : QObject() {
  mIsAnimating = false;
  mIsPaused = false;
  mAnimationProgress = 0.0f;
  mAnimationSpeed = 1.0f;
  mAnimationDirection = QVector3D(1.0f, 0.0f, 0.0f);
}

void Bounds::merge(const Bounds& bounds){
  min = QVector3D(std::min(min.x(), bounds.min.x()), 
                  std::min(min.y(), bounds.min.y()), 
                  std::min(min.z(), bounds.min.z()));
  max = QVector3D(std::max(max.x(), bounds.max.x()), 
                  std::max(max.y(), bounds.max.y()), 
                  std::max(max.z(), bounds.max.z()));
}