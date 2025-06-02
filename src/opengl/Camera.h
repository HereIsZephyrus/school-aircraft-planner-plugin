#ifndef CAMERA_H
#define CAMERA_H

#include <QVector3D>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QPoint>

// 默认相机参数
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
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

    // 相机属性
    QVector3D mPosition;    // 相机位置
    QVector3D mFront;       // 相机前向量
    QVector3D mUp;         // 相机上向量
    QVector3D mRight;      // 相机右向量
    QVector3D mWorldUp;    // 世界空间上向量
    
    // 欧拉角
    float mYaw;           // 偏航角
    float mPitch;         // 俯仰角
    
    // 相机选项
    float mMovementSpeed;  // 移动速度
    float mMouseSensitivity; // 鼠标灵敏度
    float mZoom;          // 缩放值
    
    // 投影参数
    float mAspectRatio;   // 宽高比
    float mNearPlane;     // 近平面
    float mFarPlane;      // 远平面

    // 获取视图矩阵
    QMatrix4x4 viewMatrix() const;
    // 获取投影矩阵
    QMatrix4x4 projectionMatrix() const;

    // 设置相机参数
    void setPosition(const QVector3D& position);
    void setAspectRatio(float ratio);
    void setNearPlane(float near);
    void setFarPlane(float far);
    void setUpVector(const QVector3D& up);
    
    // 移动控制
    void moveForward(float deltaTime);
    void moveBackward(float deltaTime);
    void moveLeft(float deltaTime);
    void moveRight(float deltaTime);
    void moveUp(float deltaTime);
    void moveDown(float deltaTime);
    
    // 旋转控制
    void rotate(float yaw, float pitch, bool constrainPitch = true);
    void handleMouseMove(const QPoint& delta, bool constrainPitch = true);
    void handleMouseWheel(int delta);
    
    // 重置视图
    void resetView();

private:
    void updateCameraVectors();
};

#endif // CAMERA_H
