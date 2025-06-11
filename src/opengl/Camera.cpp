#include "Camera.h"
#include "../log/QgisDebug.h"
#include "../MainWindow.h"
#include <QtMath>

Camera::Camera() {
    mPosition = QVector3D(0.0f, 0.0f, 5.0f);
    mWorldUp = QVector3D(0.0f, 0.0f, 1.0f);
    mRotation = QQuaternion::fromEulerAngles(0.0f, -YAW, 0.0f); // 初始旋转
    mMovementSpeed = SPEED;
    mMouseSensitivity = SENSITIVITY;
    mZoom = ZOOM;
    mAspectRatio = 1.0f;
    mNearPlane = 0.1f;
    mFarPlane = 1000.0f;
    
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
    mWorldUp = up.normalized();
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
    view.lookAt(mPosition, mPosition + mFront - mUp * 0.3, mUp + mFront * 0.3);
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

void Camera::rotate(const QQuaternion& rotation) {
    mRotation = rotation * mRotation;
    updateCameraVectors();
}

void Camera::handleMouseMove(const QPoint& delta) {
    float xoffset = delta.x() * mMouseSensitivity;
    float yoffset = -delta.y() * mMouseSensitivity;
    
    QQuaternion horizontalRotation = QQuaternion::fromAxisAndAngle(mWorldUp, xoffset);
    QQuaternion verticalRotation = QQuaternion::fromAxisAndAngle(mRight, yoffset);
    
    mRotation = horizontalRotation * verticalRotation * mRotation;
    updateCameraVectors();
}

void Camera::handleMouseWheel(int delta) {
    mZoom -= delta;
    mZoom = qBound(1.0f, mZoom, 45.0f);
}

void Camera::resetView() {
    mPosition = QVector3D(0.0f, 0.0f, 3.0f);
    mWorldUp = QVector3D(0.0f, 0.0f, 1.0f);
    mRotation = QQuaternion::fromEulerAngles(0.0f, -YAW, 0.0f);
    mZoom = ZOOM;
    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    mFront = mRotation.rotatedVector(QVector3D(0.0f, 0.0f, -1.0f));
    mFront.normalize();
    
    mRight = QVector3D::crossProduct(mFront, mWorldUp).normalized();
    mUp = QVector3D::crossProduct(mRight, mFront).normalized();
}