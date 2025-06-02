#include "WorkspaceState.h"
#include <QProcessEnvironment>
#include <QRandomGenerator>
#include <cstdlib>
#include "../log/QgisDebug.h"

static QString GetHomeDirectory() {
#ifdef _WIN32
  return QString(getenv("USERPROFILE"));
#else
  const char *home = getenv("HOME");
  return QString(home);
#endif
}

namespace ws {
QObject *ws::WindowManager::pDefaultObject = nullptr;

void initializeWorkspaceState() {
  PathManager::getInstance();
  logMessage("WorkspaceState initialized", Qgis::MessageLevel::Success);
}
} // namespace ws

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
ws::FlightManager::~FlightManager() { mFlightPath.clear(); }

ws::WindowManager::WindowManager() {
  mCurrentCanvas = CanvasType::ThreeD;
  is3DMapInited = false;
  is2DMapInited = false;
  if (!pDefaultObject)
    pDefaultObject = new QObject();
  mBounds = Bounds();
}
ws::WindowManager::~WindowManager() { delete pDefaultObject; }

// add new slot function at the end of the file
QString ws::FlightManager::queryFlightParameters() {
  logMessage("generate random flight parameters", Qgis::MessageLevel::Info);
  double latitude = QRandomGenerator::global()->bounded(-90, 90);
  double longitude = QRandomGenerator::global()->bounded(-180, 180);

  QString params = QString("Current Flight Parameters:\n"
                           "Speed: %1 m/s\n"
                           "Altitude: %2 m\n"
                           "Battery: %3%\n"
                           "Position: (%4, %5)")
                       .arg(mFlightSpeed, 0, 'f', 1)
                       .arg(mFlightAltitude, 0, 'f', 1)
                       .arg(mFlightBattery, 0, 'f', 1)
                       .arg(latitude, 0, 'f', 6)
                       .arg(longitude, 0, 'f', 6);

  logMessage("generate random flight parameters", Qgis::MessageLevel::Success);
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

void ws::AnimationManager::pauseSimulation() {
    mIsPaused = true;
}

void ws::AnimationManager::resumeSimulation() {
    mIsPaused = false;
}

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
