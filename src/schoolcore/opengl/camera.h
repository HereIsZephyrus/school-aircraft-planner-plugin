#ifndef CAMERA_H
#define CAMERA_H

#include <QVector3D>
#include <QMatrix4x4>
#include <QQuaternion>
#include <qvector3d.h>

class Camera {
private:
    Camera(QVector3D initalPosition = QVector3D(0.0f, 0.0f, 5.0f),
           QVector3D initalTarget = QVector3D(0.0f, 0.0f, 0.0f),
           QVector3D initalUp = QVector3D(0.0f, 1.0f, 0.0f),
           float initalFov = 45.0f,
           float initalAspectRatio = 1.0f,
           float initalNearPlane = 0.1f,
           float initalFarPlane = 1000.0f,
           float initalYaw = -90.0f,
           float initalPitch = 0.0f);
public:
    static Camera& getInstance() {
        static Camera instance;
        return instance;
    }
    ~Camera();
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

    void setPosition(const QVector3D& position);
    void setTarget(const QVector3D& target);
    void setUpVector(const QVector3D& up);
    void setFieldOfView(float fov);
    void setAspectRatio(float ratio);
    void setNearPlane(float near);
    void setFarPlane(float far);

    QVector3D position() const { return mPosition; }
    QVector3D target() const { return mTarget; }
    QVector3D upVector() const { return mUp; }
    float fieldOfView() const { return mFov; }
    float aspectRatio() const { return mAspectRatio; }
    float nearPlane() const { return mNearPlane; }
    float farPlane() const { return mFarPlane; }
    QMatrix4x4 viewMatrix() const;
    QMatrix4x4 projectionMatrix() const;

    void moveForward(float distance);
    void moveRight(float distance);
    void moveUp(float distance);
    void rotate(float yaw, float pitch);
    void handleMouseMove(const QPoint& delta);
    void handleMouseWheel(int delta);

private:
    QVector3D mPosition, mTarget;      
    QVector3D mUp,mRight, mFront;       
    float mFov, mAspectRatio, mNearPlane, mFarPlane;      
    float mYaw, mPitch;

    void updateCameraVectors();
    void updateProjectionMatrix();
};

#endif // CAMERA_H 