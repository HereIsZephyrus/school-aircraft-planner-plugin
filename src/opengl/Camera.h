#ifndef CAMERA_H
#define CAMERA_H

#include <QVector3D>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QPoint>

const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  10.0f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;

class Camera {
private:
    Camera();
public:
    ~Camera();
    static Camera& getInstance() {
        static Camera instance;
        return instance;
    }
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

    QVector3D mPosition;
    QVector3D mFront;
    QVector3D mUp;
    QVector3D mRight;
    QVector3D mWorldUp;
    
    QQuaternion mRotation;
    
    float mMovementSpeed;
    float mMouseSensitivity;
    float mZoom;
    
    float mAspectRatio;
    float mNearPlane;
    float mFarPlane;

    QMatrix4x4 viewMatrix() const;
    QMatrix4x4 projectionMatrix() const;

    void setPosition(const QVector3D& position);
    void setAspectRatio(float ratio);
    void setNearPlane(float near);
    void setFarPlane(float far);
    void setUpVector(const QVector3D& up);
    
    void moveForward(float deltaTime);
    void moveBackward(float deltaTime);
    void moveLeft(float deltaTime);
    void moveRight(float deltaTime);
    void moveUp(float deltaTime);
    void moveDown(float deltaTime);
    
    void rotate(const QQuaternion& rotation);
    void handleMouseMove(const QPoint& delta);
    void handleMouseWheel(int delta);
    
    void resetView();

private:
    void updateCameraVectors();
};

#endif // CAMERA_H
