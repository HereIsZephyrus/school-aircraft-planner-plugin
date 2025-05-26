#pragma once
#include <QString>
#include <QList>
#include <QDir>
//use a singleton to manage the workspace state
typedef std::pair<QString, QString> ObjTexturePair; // the first is the obj path, the second is the texture path
namespace ws{
void initializeWorkspaceState();

enum class CanvasType : unsigned char{
    ThreeD,
    TwoD
};
class WindowManager {
private:
    WindowManager();
    ~WindowManager();
public:
    static WindowManager& getInstance() {
        static WindowManager instance;
        return instance;
    }
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;
    void setCurrentCanvas(CanvasType canvas) {mCurrentCanvas = canvas;}
    CanvasType getCurrentCanvas() const {return mCurrentCanvas;}
    QObject* getDefaultObject() const {return pDefaultObject;}
private:
    CanvasType mCurrentCanvas;
    static QObject *pDefaultObject;
};
class PathManager {
private:
    PathManager();
    ~PathManager();
public:
    static PathManager& getInstance() {
        static PathManager instance;
        return instance;
    }
    PathManager(const PathManager&) = delete;
    PathManager& operator=(const PathManager&) = delete;
    void setRootDir(const QString& rootDir) {mRootDir = rootDir;}
    void findAllObjAndTexturePaths();
    QString getRootDir() const {return mRootDir;}
    void addObjTexturePair(const ObjTexturePair& objTexturePair) {mObjTexturePairs.append(objTexturePair);}
    QList<ObjTexturePair> getObjTexturePairs() const {return mObjTexturePairs;}
    ObjTexturePair getObjTexturePair(int index) const;
private:
    QString mRootDir;
    //QString mPath3D;
    //QString mPathTexture;
    QList<ObjTexturePair> mObjTexturePairs;
};

enum class WeatherType : unsigned char{
    Sunny,
    Cloudy,
    Rainy,
    Snowy,
    Foggy
};
class EnvManager{
private:
    EnvManager();
    ~EnvManager();
public:
    const QStringList weatherList = {"Sunny", "Cloudy", "Rainy", "Snowy", "Foggy"};
    static EnvManager& getInstance() {
        static EnvManager instance;
        return instance;
    }
    EnvManager(const EnvManager&) = delete;
    EnvManager& operator=(const EnvManager&) = delete;
    void setWeather(WeatherType weather) {mWeather = weather;}
    void setTemperature(double temperature) {mTemperature = temperature;}
    void setPressure(double pressure) {mPressure = pressure;}
    WeatherType getWeatherType() const {return mWeather;}
    QString getWeatherString() const {return weatherList[static_cast<unsigned char>(mWeather)];}
    double getTemperature() const {return mTemperature;}
    double getPressure() const {return mPressure;}
    static constexpr int minTemperature = -50;
    static constexpr int maxTemperature = 50;
    static constexpr int minPressure = 800;
    static constexpr int maxPressure = 1100;
private:
    WeatherType mWeather;
    double mTemperature;
    double mPressure;
};
class FlightManager{
private:
    FlightManager();
    ~FlightManager();
public:
    static FlightManager& getInstance() {
        static FlightManager instance;
        return instance;
    }
    FlightManager(const FlightManager&) = delete;
    FlightManager& operator=(const FlightManager&) = delete;
    void setFlightSpeed(double speed) {mFlightSpeed = speed;}
    double getFlightSpeed() const {return mFlightSpeed;}
    void setFlightAltitude(double altitude) {mFlightAltitude = altitude;}
    double getFlightAltitude() const {return mFlightAltitude;}
    void setFlightBattery(double battery) {mFlightBattery = battery;}
    double getFlightBattery() const {return mFlightBattery;}
    static constexpr int minFlightSpeed = 1;
    static constexpr int maxFlightSpeed = 50;
    static constexpr int minFlightAltitude = 50;
    static constexpr int maxFlightAltitude = 1000;
    static constexpr int minFlightBattery = 0;
    static constexpr int maxFlightBattery = 100;
private:
    double mFlightSpeed;
    double mFlightAltitude;
    double mFlightBattery;
};
}