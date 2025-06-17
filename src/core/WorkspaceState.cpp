#include "WorkspaceState.h"
#include "../log/QgisDebug.h"
#include "../opengl/Camera.h"
#include "../core/RoutePlanner.h"
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

QObject *wsp::WindowManager::pDefaultObject = nullptr;

void wsp::initializeWorkspaceState() {
  PathManager::getInstance();
  logMessage("WorkspaceState initialized", Qgis::MessageLevel::Success);
}

wsp::PathManager::PathManager() {
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  mRootDir = GetHomeDirectory();
}

wsp::PathManager::~PathManager() { mObjTexturePairs.clear(); }

void wsp::PathManager::findAllObjAndTexturePaths() {
  QDir dir(mRootDir);
  QStringList objFiles = dir.entryList(QStringList() << "*.obj", QDir::Files);
  QStringList textureFiles =
      dir.entryList(QStringList() << "*.jpg", QDir::Files);
}

ObjTexturePair wsp::PathManager::getObjTexturePair(int index) const {
  if (index < 0 || index >= mObjTexturePairs.size()) {
    throw std::out_of_range("Index out of range");
  }
  return mObjTexturePairs[index];
  if (index < 0 || index >= mObjTexturePairs.size()) {
    throw std::out_of_range("Index out of range");
  }
  return mObjTexturePairs[index];
}

wsp::EnvManager::EnvManager() {
  mWeather = WeatherType::Sunny;
  mTemperature = 25.0;
  mPressure = 1013.25;
}
wsp::EnvManager::~EnvManager() {}

wsp::FlightManager::FlightManager() {
  mFlightSpeed = 5.0;
  mFlightBattery = 100.0;
  mBaseHeight = 100.0;
  mMaxAlititude = maxBaseHeight;
  mManualMode = true;
}
wsp::FlightManager::~FlightManager() { mFlightPath.clear(); }

wsp::WindowManager::WindowManager()
    : QObject(nullptr), mCurrentCanvas(CanvasType::ThreeD),
      is3DMapInited(false), is2DMapInited(false) {
  mUpdateTimer = new QTimer(this);
  connect(mUpdateTimer, &QTimer::timeout, this,
          &WindowManager::updateCameraMovement);
  mUpdateTimer->start(16);
  if (!pDefaultObject)
    pDefaultObject = new QObject();
  mBounds = Bounds();
  isEditMode = false;
  geoTransform.setToIdentity();
  targetCrs = QgsCoordinateReferenceSystem("EPSG:4547");
  baseDrawHeight = 30.0f;
}

wsp::WindowManager::~WindowManager() {
  if (mUpdateTimer) {
    mUpdateTimer->stop();
    delete mUpdateTimer;
  }
  delete pDefaultObject;
}

void wsp::WindowManager::keyPressEvent(QKeyEvent *event) {
  mKeyStates[event->key()] = true;
}

void wsp::WindowManager::keyReleaseEvent(QKeyEvent *event) {
  mKeyStates[event->key()] = false;
}

bool wsp::WindowManager::isKeyPressed(int key) const {
  return mKeyStates.value(key, false);
}

void wsp::WindowManager::update3DCameraMovement() {
  float step = FlightManager::getInstance().getFlightSpeed() / 10;
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

void wsp::WindowManager::update2DCameraMovement() {
  return;
}

void wsp::WindowManager::updateCameraMovement() {
  if (mCurrentCanvas == CanvasType::ThreeD) {
    update3DCameraMovement();
  }
  if (mCurrentCanvas == CanvasType::TwoD) {
    update2DCameraMovement();
  }
}

QString wsp::FlightManager::displayFlightParams() {
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

QVector3D wsp::WindowManager::getGeoTransform(QVector3D modelPosition){
  QVector4D geoPosition = geoTransform * QVector4D(modelPosition,1.0);
  return QVector3D(geoPosition.x(), geoPosition.y(), geoPosition.z());
}
QVector3D wsp::WindowManager::getModelTransform(QVector3D geoPosition){
  QVector4D modelPosition = geoTransform.inverted() * QVector4D(geoPosition,1.0);
  return QVector3D(modelPosition.x(), modelPosition.y(), modelPosition.z());
}
void wsp::EnvManager::generateRandomWeather() {
  logMessage("generate random weather data", Qgis::MessageLevel::Info);
  WeatherType weather =
      static_cast<WeatherType>(QRandomGenerator::global()->bounded(5));

  wsp::EnvManager &envManager = wsp::EnvManager::getInstance();
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

void wsp::AnimationManager::startSimulation() {
  if (wsp::FlightManager::getInstance().isManualMode()) {
    logMessage("Cannot start simulation in manual mode", Qgis::MessageLevel::Warning);
    return;
  }
  mIsAnimating = true;
  mAnimationProgress = 0.0f;
  currentPathIndex = 0;
  Camera &camera = Camera::getInstance();
  RoutePlanner &routePlanner = RoutePlanner::getInstance();
  mPath = RoutePlanner::getInstance().getRoutePath();
  camera.setPosition(routePlanner.getHomePoint());
}

void wsp::AnimationManager::pauseSimulation() { mIsAnimating = true; }

void wsp::AnimationManager::resumeSimulation() { mIsAnimating = false; }

void wsp::AnimationManager::returnToHome() {
  mAnimationProgress = 0.0f;
  mIsAnimating = false;
  currentPathIndex = 0;
  Camera &camera = Camera::getInstance();
  RoutePlanner &routePlanner = RoutePlanner::getInstance();
  camera.setPosition(routePlanner.getHomePoint()); // Reset camera position
}

void wsp::AnimationManager::stopSimulation() {
  mIsAnimating = false;
  mAnimationProgress = 0.0f;
}

wsp::AnimationManager::AnimationManager() : QObject() {
  mIsAnimating = false;
  mAnimationProgress = 0.0f;
  mAnimationSpeed = 0.1f;
}

void wsp::FlightManager::setManualMode(bool manual) {
  mManualMode = manual;
  if (manual) {
    logMessage("Flight Manager is now in manual mode", Qgis::MessageLevel::Info);
    Camera::getInstance().behindView();
  } else {
    logMessage("Flight Manager is now in automatic mode", Qgis::MessageLevel::Info);
    Camera::getInstance().insideView();
  }
}

QMatrix4x4 wsp::calculateCoordTransform(const QVector3D& s1, const QVector3D& s2, const QVector3D& s3,
                              const QVector3D& t1, const QVector3D& t2, const QVector3D& t3) {
    QVector3D deltaS12 = s2 - s1;
    QVector3D deltaS13 = s3 - s1;
    
    QVector3D deltaT12 = t2 - t1;
    QVector3D deltaT13 = t3 - t1;
    
    QVector3D u = deltaS12.normalized();
    QVector3D v = QVector3D::crossProduct(u, deltaS13).normalized();
    QVector3D w = QVector3D::crossProduct(u, v);
    
    QVector3D u_prime = deltaT12.normalized();
    QVector3D v_prime = QVector3D::crossProduct(u_prime, deltaT13).normalized();
    QVector3D w_prime = QVector3D::crossProduct(u_prime, v_prime);
    
    QMatrix4x4 sourceBasis(u.x(), v.x(), w.x(), 0,
                          u.y(), v.y(), w.y(), 0,
                          u.z(), v.z(), w.z(), 0,
                          0,    0,    0,    1);
    
    QMatrix4x4 targetBasis(u_prime.x(), v_prime.x(), w_prime.x(), 0,
                           u_prime.y(), v_prime.y(), w_prime.y(), 0,
                           u_prime.z(), v_prime.z(), w_prime.z(), 0,
                           0,           0,           0,           1);
    
    QMatrix4x4 rotation = targetBasis * sourceBasis.inverted();
    
    QVector3D translation = t1 - rotation * s1;
    
    QMatrix4x4 transform;
    transform.setToIdentity();
    transform *= rotation;
    transform.translate(translation);
    
    return transform;
}

void Bounds::merge(const Bounds& bounds){
  min = QVector3D(std::min(min.x(), bounds.min.x()), 
                  std::min(min.y(), bounds.min.y()), 
                  std::min(min.z(), bounds.min.z()));
  max = QVector3D(std::max(max.x(), bounds.max.x()), 
                  std::max(max.y(), bounds.max.y()), 
                  std::max(max.z(), bounds.max.z()));
}