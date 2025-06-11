#ifndef WORKSPACE_STATE_H
#define WORKSPACE_STATE_H
#include <QString>
#include <QList>
#include <QDir>
#include <QMatrix4x4>
#include <QVector3D>
#include <QMap>
#include <QKeyEvent>
#include <QTimer>
#include <float.h>
#include <qvector3d.h>
#include <stdexcept>

//use a singleton to manage the workspace state
typedef std::pair<QString, QString> ObjTexturePair; // the first is the obj path, the second is the texture path
struct Bounds{
    QVector3D min;
    QVector3D max;
    QVector3D center;
    Bounds( QVector3D min = QVector3D(FLT_MAX, FLT_MAX, FLT_MAX), 
            QVector3D max = QVector3D(-FLT_MAX, -FLT_MAX, -FLT_MAX), 
            QVector3D center = QVector3D(0, 0, 0)) 
        : min(min), max(max), center(center) {}
    void merge(const Bounds& bounds);
};
namespace ws{
void initializeWorkspaceState();

enum class CanvasType : unsigned char{
    ThreeD,
    TwoD
};
class WindowManager : public QObject{
    Q_OBJECT
    
private:
    WindowManager();
public:
    virtual ~WindowManager();
    static WindowManager& getInstance() {
        static WindowManager instance;
        return instance;
    }
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;
    void setCurrentCanvas(CanvasType canvas) {mCurrentCanvas = canvas;}
    CanvasType getCurrentCanvas() const {return mCurrentCanvas;}
    QObject* getDefaultObject() const {return pDefaultObject;}
    const Bounds& getBounds() const {return mBounds;}
    void setBounds(const Bounds& bounds) {mBounds = bounds;}
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    bool isKeyPressed(int key) const;
    void updateCameraMovement();

private:
    CanvasType mCurrentCanvas;
    static QObject *pDefaultObject;
    bool is3DMapInited,is2DMapInited;
    Bounds mBounds;
    QMap<int, bool> mKeyStates;
    QTimer* mUpdateTimer;
    void update3DCameraMovement();
    void update2DCameraMovement();
};
class PathManager : public QObject{
    Q_OBJECT

private:
    PathManager();
public:
    virtual ~PathManager();
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
class EnvManager : public QObject{
    Q_OBJECT

private:
    EnvManager();
public:
    virtual ~EnvManager();
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

public slots:
    void generateRandomWeather();

private:
    WeatherType mWeather;
    double mTemperature;
    double mPressure;
};
class FlightManager : public QObject{
    Q_OBJECT

private:
    FlightManager();
public:
    virtual ~FlightManager();
    static FlightManager& getInstance() {
        static FlightManager instance;
        return instance;
    }
    FlightManager(const FlightManager&) = delete;
    FlightManager& operator=(const FlightManager&) = delete;
    void setFlightSpeed(double speed) {mFlightSpeed = speed;}
    double getFlightSpeed() const {return mFlightSpeed;}
    void setFlightBattery(double battery) {mFlightBattery = battery;}
    double getFlightBattery() const {return mFlightBattery;}
    void setBaseHeight(double height) {mBaseHeight = height;}
    double getBaseHeight() const {return mBaseHeight;}
    void setPorision(QVector3D newPosition) {mAircraftPosition = newPosition;}
    QVector3D getPosition() const {return mAircraftPosition;}
    void setMaxAltitude(double alititude) {mMaxAlititude = alititude;}
    double getMaxAltitude() const {return mMaxAlititude;}
    //void generateFlightRoute(float height);
    static constexpr int minFlightSpeed = 1;
    static constexpr int maxFlightSpeed = 50;
    static constexpr int minFlightAltitude = 50;
    static constexpr int maxFlightAltitude = 1000;
    static constexpr int minFlightBattery = 0;
    static constexpr int maxFlightBattery = 100;
    static constexpr int minBaseHeight = 0;
    static constexpr int maxBaseHeight = 100;

public slots:
    QString queryFlightParameters();
private:
    double mFlightSpeed,mFlightBattery;
    double mBaseHeight, mMaxAlititude;
    QVector3D mAircraftPosition;
    QQuaternion mAircraftOrientation;
    QVector<QVector3D> mFlightPath;
    QVector3D mHomePosition;
};

class AnimationManager : public QObject {
    Q_OBJECT

private:
    AnimationManager();
    ~AnimationManager() = default;
public:
    static AnimationManager& getInstance() {
        static AnimationManager instance;
        return instance;
    }
    AnimationManager(const AnimationManager&) = delete;
    AnimationManager& operator=(const AnimationManager&) = delete;
    void setAnimationSpeed(double speed) {mAnimationSpeed = speed;}
    double getAnimationSpeed() const {return mAnimationSpeed;}
    void setAnimationDirection(QVector3D direction) {mAnimationDirection = direction;}
    QVector3D getAnimationDirection() const {return mAnimationDirection;}

public slots:
    void startSimulation();
    void pauseSimulation();
    void resumeSimulation();
    void returnToHome();
    void stopSimulation();

private:
    double mAnimationSpeed;
    QVector3D mAnimationDirection;
    float mAnimationProgress;
    bool mIsAnimating;
    bool mIsPaused;
    void updateAnimation();
    void drawAircraft(const QVector3D &position, const QQuaternion &orientation);
    bool mCameraFollowAircraft;
    QVector3D mViewTranslation;
};
}

#endif