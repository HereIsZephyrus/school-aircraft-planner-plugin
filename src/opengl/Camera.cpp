#include "Camera.h"
#include "../log/QgisDebug.h"
#include <QtMath>

Camera::Camera(){
  resetView();
  updateCameraVectors();
}
Camera::~Camera(){
  logMessage("Camera destroyed", Qgis::MessageLevel::Success);
}
void Camera::setPosition(const QVector3D &position) {
  mPosition = position;
  updateCameraVectors();
}

void Camera::setTarget(const QVector3D &target) {
  mTarget = target;
  updateCameraVectors();
}

void Camera::setUpVector(const QVector3D &up) {
  mUp = up.normalized();
  updateCameraVectors();
}

void Camera::setFieldOfView(float fov) { mFov = qBound(1.0f, fov, 179.0f); }

void Camera::setAspectRatio(float ratio) { mAspectRatio = ratio; }

void Camera::setNearPlane(float near) { mNearPlane = near; }

void Camera::setFarPlane(float far) { mFarPlane = far; }

QMatrix4x4 Camera::viewMatrix() const {
  QMatrix4x4 view;
  view.lookAt(mPosition, mPosition + mFront, mUp);
  return view;
}

QMatrix4x4 Camera::projectionMatrix() const {
  QMatrix4x4 projection;
  projection.perspective(mFov, mAspectRatio, mNearPlane, mFarPlane);
  return projection;
}

void Camera::moveForward(float distance) { mPosition += mFront * distance; }

void Camera::moveRight(float distance) { mPosition += mRight * distance; }

void Camera::moveUp(float distance) { mPosition += mUp * distance; }

void Camera::rotate(float yaw, float pitch) {
  mYaw += yaw;
  mPitch = qBound(-89.0f, mPitch + pitch, 89.0f);
  updateCameraVectors();
}

void Camera::handleMouseMove(const QPoint &delta) {
  const float sensitivity = 0.1f;
  rotate(delta.x() * sensitivity, -delta.y() * sensitivity);
}

void Camera::handleMouseWheel(int delta) {
  const float zoomSpeed = 0.1f;
  float zoom = delta > 0 ? -zoomSpeed : zoomSpeed;
  mFov = qBound(1.0f, mFov + zoom, 179.0f);
}

void Camera::updateCameraVectors() {
  QVector3D front;
  front.setX(qCos(qDegreesToRadians(mYaw)) * qCos(qDegreesToRadians(mPitch)));
  front.setY(qSin(qDegreesToRadians(mPitch)));
  front.setZ(qSin(qDegreesToRadians(mYaw)) * qCos(qDegreesToRadians(mPitch)));
  mFront = front.normalized();
  mRight = QVector3D::crossProduct(mFront, mUp).normalized();
  mUp = QVector3D::crossProduct(mRight, mFront).normalized();
}

void Camera::resetView() {
  mFov = 45.0f;
  mAspectRatio = 16.0f / 9.0f;
  mNearPlane = 0.1f;
  mFarPlane = 1000.0f;
  mYaw = -90.0f;
  mPitch = 0.0f;
  mPosition = QVector3D(0.0f, 0.0f, 50.0f);
  mTarget = QVector3D(0.0f, 0.0f, 0.0f);
  mUp = QVector3D(0.0f, 1.0f, 0.0f);
}