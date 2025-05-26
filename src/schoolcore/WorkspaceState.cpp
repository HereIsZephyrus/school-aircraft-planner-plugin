#include "WorkspaceState.h"

namespace ws {
    void initializeWorkspaceState() {
        PathManager::getInstance();
    }
}

ws::PathManager::PathManager() {
    mRootDir = QString::fromStdString(getenv("HOME"));
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
}
ws::FlightManager::~FlightManager() {}

ws::WindowManager::WindowManager() {
    mCurrentCanvas = CanvasType::ThreeD;
    pDefaultObject = new QObject();
}
ws::WindowManager::~WindowManager() {
    delete pDefaultObject;
}