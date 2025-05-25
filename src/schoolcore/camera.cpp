#include "camera.h"
#include <QtMath>

Camera::Camera()
    : m_position(0.0f, 0.0f, 5.0f)
    , m_target(0.0f, 0.0f, 0.0f)
    , m_up(0.0f, 1.0f, 0.0f)
    , m_fov(45.0f)
    , m_aspectRatio(1.0f)
    , m_nearPlane(0.1f)
    , m_farPlane(1000.0f)
    , m_yaw(-90.0f)
    , m_pitch(0.0f)
{
    updateCameraVectors();
}

void Camera::setPosition(const QVector3D& position)
{
    m_position = position;
    updateCameraVectors();
}

void Camera::setTarget(const QVector3D& target)
{
    m_target = target;
    updateCameraVectors();
}

void Camera::setUpVector(const QVector3D& up)
{
    m_up = up.normalized();
    updateCameraVectors();
}

void Camera::setFieldOfView(float fov)
{
    m_fov = qBound(1.0f, fov, 179.0f);
}

void Camera::setAspectRatio(float ratio)
{
    m_aspectRatio = ratio;
}

void Camera::setNearPlane(float near)
{
    m_nearPlane = near;
}

void Camera::setFarPlane(float far)
{
    m_farPlane = far;
}

QMatrix4x4 Camera::viewMatrix() const
{
    QMatrix4x4 view;
    view.lookAt(m_position, m_position + m_front, m_up);
    return view;
}

QMatrix4x4 Camera::projectionMatrix() const
{
    QMatrix4x4 projection;
    projection.perspective(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);
    return projection;
}

void Camera::moveForward(float distance)
{
    m_position += m_front * distance;
}

void Camera::moveRight(float distance)
{
    m_position += m_right * distance;
}

void Camera::moveUp(float distance)
{
    m_position += m_up * distance;
}

void Camera::rotate(float yaw, float pitch)
{
    m_yaw += yaw;
    m_pitch = qBound(-89.0f, m_pitch + pitch, 89.0f);
    updateCameraVectors();
}

void Camera::handleMouseMove(const QPoint& delta)
{
    const float sensitivity = 0.1f;
    rotate(delta.x() * sensitivity, -delta.y() * sensitivity);
}

void Camera::handleMouseWheel(int delta)
{
    const float zoomSpeed = 0.1f;
    float zoom = delta > 0 ? -zoomSpeed : zoomSpeed;
    m_fov = qBound(1.0f, m_fov + zoom, 179.0f);
}

void Camera::updateCameraVectors()
{
    // 计算前向量
    QVector3D front;
    front.setX(qCos(qDegreesToRadians(m_yaw)) * qCos(qDegreesToRadians(m_pitch)));
    front.setY(qSin(qDegreesToRadians(m_pitch)));
    front.setZ(qSin(qDegreesToRadians(m_yaw)) * qCos(qDegreesToRadians(m_pitch)));
    m_front = front.normalized();

    // 计算右向量
    m_right = QVector3D::crossProduct(m_front, m_up).normalized();

    // 重新计算上向量
    m_up = QVector3D::crossProduct(m_right, m_front).normalized();
} 