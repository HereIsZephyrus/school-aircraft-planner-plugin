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
    mDis2Camera = 10.0f;
    
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

void Camera::setNearPlane(float mfnear) {
    mNearPlane = mfnear;
}

void Camera::setFarPlane(float mffar) {
    mFarPlane = mffar;
}

QMatrix4x4 Camera::viewMatrix() const {
    QMatrix4x4 view;
    view.lookAt(mPosition, mPosition + mFront, mUp);
    //view.lookAt(mPosition, mPosition + mFront - mUp * 0.3, mUp + mFront * 0.3);
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

static float length(const QVector3D& vec) {
    return qSqrt(vec.x() * vec.x() + vec.y() * vec.y() + vec.z() * vec.z());
}

void Camera::checkProcess(){
    using namespace wsp;
    if (FlightManager::getInstance().isManualMode())
        return;
    AnimationManager& animationManager = wsp::AnimationManager::getInstance();
    if (!animationManager.isAnimating())
        return;
    int currentIndex = animationManager.currentPathIndex;
    QVector3D currentStart = animationManager.mPath[2 * currentIndex];
    QVector3D currentEnd = animationManager.mPath[2 * currentIndex + 1];
    QVector3D direction = (currentEnd - currentStart).normalized();
    float dis = length(currentEnd - mPosition);
    if (length(currentStart - currentEnd) < length(currentStart - mPosition)) {
        ++animationManager.currentPathIndex;
        if (animationManager.currentPathIndex >= animationManager.mPath.size() / 2)
            animationManager.currentPathIndex = 0;
        mPosition = animationManager.mPath[2 * animationManager.currentPathIndex];
    }
    mFront = direction;
    moveForward(animationManager.getAnimationSpeed());
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

QQuaternion Camera::zeroPitchDirect() {
    float w = mRotation.scalar(), x = mRotation.x(), y = mRotation.y(), z = mRotation.z();
    float m20 = 2.0f * (x*z - w*y);
    QQuaternion result;
    
    float yaw = qAtan2(2.0f*(x*y + w*z), w*w + x*x - y*y - z*z);
    float roll = qAtan2(2.0f*(y*z + w*x), w*w - x*x - y*y + z*z);
    //QQuaternion yawQuat = QQuaternion::fromAxisAndAngle(0, 0, 1, -qRadiansToDegrees(yaw));
    QQuaternion rollQuat = QQuaternion::fromAxisAndAngle(1, 0, 0, qRadiansToDegrees(roll));
    QQuaternion yawQuat = QQuaternion::fromAxisAndAngle(0, 0, 1, qRadiansToDegrees(yaw));
    result = rollQuat * yawQuat;
    
    return result.normalized();
}

void Camera::behindView(){
    mPosition -= mFront * mDis2Camera;
}

void Camera::insideView(){
    mPosition += mFront * mDis2Camera;
}