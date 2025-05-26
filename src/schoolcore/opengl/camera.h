#ifndef CAMERA_H
#define CAMERA_H

#include <QVector3D>
#include <QMatrix4x4>
#include <QQuaternion>

class Camera {
private:
    // initial camera parameters
    static constexpr float DEFAULT_FOV = 45.0f;
    static constexpr float DEFAULT_ASPECT_RATIO = 16.0f / 9.0f;
    static constexpr float DEFAULT_NEAR_PLANE = 0.1f;
    static constexpr float DEFAULT_FAR_PLANE = 1000.0f;
    static constexpr float DEFAULT_YAW = -90.0f;
    static constexpr float DEFAULT_PITCH = 0.0f;
    static constexpr QVector3D DEFAULT_POSITION = QVector3D(0.0f, 0.0f, 5.0f);
    static constexpr QVector3D DEFAULT_TARGET = QVector3D(0.0f, 0.0f, 0.0f);
    static constexpr QVector3D DEFAULT_UP = QVector3D(0.0f, 1.0f, 0.0f);
    static constexpr float ZOOM_FACTOR = 0.1f;
    
    Camera();
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
    float zoomFactor() const { return ZOOM_FACTOR; }

    void moveForward(float distance);
    void moveRight(float distance);
    void moveUp(float distance);
    void rotate(float yaw, float pitch);
    void handleMouseMove(const QPoint& delta);
    void handleMouseWheel(int delta);
    void resetView();
private:
    QVector3D mPosition, mTarget;      
    QVector3D mUp,mRight, mFront;       
    float mFov, mAspectRatio, mNearPlane, mFarPlane;      
    float mYaw, mPitch;

    void updateCameraVectors();
    void updateProjectionMatrix();
};

#endif // CAMERA_H 