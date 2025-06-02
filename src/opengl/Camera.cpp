#include "Camera.h"
#include "../log/QgisDebug.h"
#include <QtMath>

Camera::Camera() {
    // 初始化相机参数
    mPosition = QVector3D(0.0f, 0.0f, 30.0f);
    mWorldUp = QVector3D(0.0f, 1.0f, 0.0f);
    mYaw = YAW;
    mPitch = PITCH;
    mMovementSpeed = SPEED;
    mMouseSensitivity = SENSITIVITY;
    mZoom = ZOOM;
    mAspectRatio = 1.0f;
    mNearPlane = 0.1f;
    mFarPlane = 100.0f;
    
    updateCameraVectors();
}

Camera::~Camera() {
    logMessage("Camera destroyed", Qgis::MessageLevel::Success);
}

void Camera::setPosition(const QVector3D& position) {
    mPosition = position;
    updateCameraVectors();
}

void Camera::setUpVector(const QVector3D &up) {
  mUp = up.normalized();
  updateCameraVectors();
}

void Camera::setAspectRatio(float ratio) {
    mAspectRatio = ratio;
}

void Camera::setNearPlane(float near) {
    mNearPlane = near;
}

void Camera::setFarPlane(float far) {
    mFarPlane = far;
}

QMatrix4x4 Camera::viewMatrix() const {
    QMatrix4x4 view;
    view.lookAt(mPosition, mPosition + mFront, mUp);
    return view;
}

QMatrix4x4 Camera::projectionMatrix() const {
    QMatrix4x4 projection;
    projection.perspective(mZoom, mAspectRatio, mNearPlane, mFarPlane);
    return projection;
}

void Camera::moveForward(float deltaTime) {
    float velocity = mMovementSpeed * deltaTime;
    mPosition += mFront * velocity;
}

void Camera::moveBackward(float deltaTime) {
    float velocity = mMovementSpeed * deltaTime;
    mPosition -= mFront * velocity;
}

void Camera::moveLeft(float deltaTime) {
    float velocity = mMovementSpeed * deltaTime;
    mPosition -= mRight * velocity;
}

void Camera::moveRight(float deltaTime) {
    float velocity = mMovementSpeed * deltaTime;
    mPosition += mRight * velocity;
}

void Camera::moveUp(float deltaTime) {
    float velocity = mMovementSpeed * deltaTime;
    mPosition += mUp * velocity;
}

void Camera::moveDown(float deltaTime) {
    float velocity = mMovementSpeed * deltaTime;
    mPosition -= mUp * velocity;
}

void Camera::rotate(float yaw, float pitch, bool constrainPitch) {
    mYaw += yaw;
    mPitch += pitch;

    // 限制俯仰角，防止万向节死锁
    if (constrainPitch) {
        mPitch = qBound(-89.0f, mPitch, 89.0f);
    }

    updateCameraVectors();
}

void Camera::handleMouseMove(const QPoint& delta, bool constrainPitch) {
    float xoffset = delta.x() * mMouseSensitivity;
    float yoffset = -delta.y() * mMouseSensitivity; // 注意y轴方向相反
    
    rotate(xoffset, yoffset, constrainPitch);
}

void Camera::handleMouseWheel(int delta) {
    mZoom -= delta;
    mZoom = qBound(1.0f, mZoom, 45.0f);
}

void Camera::resetView() {
    mPosition = QVector3D(0.0f, 0.0f, 3.0f);
    mWorldUp = QVector3D(0.0f, 1.0f, 0.0f);
    mYaw = YAW;
    mPitch = PITCH;
    mZoom = ZOOM;
    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    // 计算新的前向量
    QVector3D front;
    front.setX(qCos(qDegreesToRadians(mYaw)) * qCos(qDegreesToRadians(mPitch)));
    front.setY(qSin(qDegreesToRadians(mPitch)));
    front.setZ(qSin(qDegreesToRadians(mYaw)) * qCos(qDegreesToRadians(mPitch)));
    mFront = front.normalized();
    
    // 重新计算右向量和上向量
    mRight = QVector3D::crossProduct(mFront, mWorldUp).normalized();
    mUp = QVector3D::crossProduct(mRight, mFront).normalized();
}