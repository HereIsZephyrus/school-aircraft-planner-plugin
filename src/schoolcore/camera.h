#ifndef CAMERA_H
#define CAMERA_H

#include <QVector3D>
#include <QMatrix4x4>
#include <QQuaternion>

class Camera {
public:
    Camera();

    // 设置相机位置和朝向
    void setPosition(const QVector3D& position);
    void setTarget(const QVector3D& target);
    void setUpVector(const QVector3D& up);

    // 获取相机参数
    QVector3D position() const { return m_position; }
    QVector3D target() const { return m_target; }
    QVector3D upVector() const { return m_up; }
    float fieldOfView() const { return m_fov; }
    float aspectRatio() const { return m_aspectRatio; }
    float nearPlane() const { return m_nearPlane; }
    float farPlane() const { return m_farPlane; }

    // 设置投影参数
    void setFieldOfView(float fov);
    void setAspectRatio(float ratio);
    void setNearPlane(float near);
    void setFarPlane(float far);

    // 获取视图和投影矩阵
    QMatrix4x4 viewMatrix() const;
    QMatrix4x4 projectionMatrix() const;

    // 相机移动控制
    void moveForward(float distance);
    void moveRight(float distance);
    void moveUp(float distance);
    void rotate(float yaw, float pitch);

    // 鼠标控制
    void handleMouseMove(const QPoint& delta);
    void handleMouseWheel(int delta);

private:
    QVector3D m_position;    // 相机位置
    QVector3D m_target;      // 观察目标点
    QVector3D m_up;          // 上方向向量
    QVector3D m_right;       // 右方向向量
    QVector3D m_front;       // 前方向向量

    float m_fov;            // 视场角
    float m_aspectRatio;    // 宽高比
    float m_nearPlane;      // 近平面
    float m_farPlane;       // 远平面

    float m_yaw;           // 偏航角
    float m_pitch;         // 俯仰角

    void updateCameraVectors();
};

#endif // CAMERA_H 