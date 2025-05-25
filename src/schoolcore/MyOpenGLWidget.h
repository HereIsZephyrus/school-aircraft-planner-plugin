/****************************************************************************
File:MyOpenGLWidget.h
Author:wkj
Date:2025.3.13
****************************************************************************/
#pragma once
#include "qgis_debug.h"
#include "Primitive.h"
#include "RoutePlanner.h"
#include <GL/gl.h>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QVector2D>
#include <QVector3D>
#include <QVector>
#include <QtMath>
#include <cfloat>
#include <memory>

class RoutePlanner;
class MyOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
  Q_OBJECT

public:
  MyOpenGLWidget(QWidget *parent = nullptr);
  ~MyOpenGLWidget();
  void resetView();
  QVector3D getSurfacePointFromMouse();
  void addControlPoint(const QVector3D &point);
  void setRoutePlanner(RoutePlanner *planner);
  void drawPathSection(const QVector<QVector3D> &points, const QVector4D &color,
                       float lineWidth, bool dashed);

protected:
  std::shared_ptr<gl::Model> modelWidget;
  std::shared_ptr<gl::BasePlane> basePlaneWidget;
  std::shared_ptr<gl::ControlPoints> ControlPointsWidget;
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int w, int h) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;
  void drawControlPoints();
  void drawConvexHull();
  void drawRoutePath();
  void drawBasePlane();
  QString retriveMtlPath(const QString &objfilePath);

private:
  void setupOpenGLContext();
  void initCanvas();
  std::shared_ptr<gl::BasePlane> initBasePlane();
  std::shared_ptr<gl::ControlPoints> initControlPoints();
  std::shared_ptr<gl::Model> initModel();
  void initBuffers();
  
private:
  RoutePlanner *m_routePlanner = nullptr;
  QMatrix4x4 mViewMatrix;

  struct ModelData {
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer vbo;
    QVector<Vertex> vertices;

    // 修改为存储材质
    QVector<Material> materials;
    QMap<QString, QOpenGLTexture *> textures; // 存储材质对应的纹理映射

    // 存储材质分组
    QMap<QString, QVector<Vertex>> materialGroups;
  };
  QList<ModelData *> m_models; // 存储模型数据
  QVector<Vertex> m_vertices;                 // 修改后的顶点数据
  QOpenGLTexture *m_currentTexture = nullptr; // 当前使用的纹理
  QVector<QVector<Vertex>> m_allVertices; // 存储所有模型的顶点数据
  QList<QOpenGLTexture *> m_allTextures;  // 存储所有纹理
  QVector<QVector2D> m_texCoords;         // 存储纹理坐标
  QOpenGLBuffer mVBO;
  QOpenGLVertexArrayObject mVAO;
  QOpenGLTexture *m_texture = nullptr;
  QMatrix4x4 mModelView;
  QPoint m_lastMousePos;
  float mfDistance = -5.0f;
  float mfFlightHight = 50.0f; //高
  QVector<QVector3D> m_currentRoute;
  // 存储点数据的缓冲区
  /*
  QOpenGLBuffer m_pointVBO;
  QOpenGLBuffer m_hullVBO;
  QOpenGLBuffer m_routeVBO;
  QOpenGLVertexArrayObject m_pointVAO;
  QOpenGLVertexArrayObject m_hullVAO;
  QOpenGLVertexArrayObject m_routeVAO;

  std::shared_ptr<QOpenGLShaderProgram> mLineShader;
  std::shared_ptr<QOpenGLShaderProgram> mModelShader;
  QVector3D calculateRayIntersection(const QVector3D &origin,
                                     const QVector3D &direction);
  float m_initialBaseHeight = 25.0f;
  QOpenGLVertexArrayObject m_basePlaneVAO;
  QOpenGLBuffer m_basePlaneVBO{QOpenGLBuffer::VertexBuffer};
  */
public slots:
  void updateFlightHeight(double height);
  void handleMouseMove(QMouseEvent *event);

  void generateFlightRoute(float height); // 生成航线
  void startSimulation(float speed);      // 开始模拟
  void pauseSimulation();                 // 暂停模拟
  void resumeSimulation();                // 继续模拟
  void returnToHome();                    // 自动返回

signals:
  void glInitialized();

private:
  // 动画计时器
  QTimer *m_animationTimer;
  float m_animationProgress; // 0~1之间的进度值
  bool m_isAnimating;
  QVector3D m_aircraftPosition;
  QQuaternion m_aircraftOrientation;
  QVector<QVector3D> m_flightPath; // 存储航线路径
  QVector3D m_homePosition;
  void updateAnimation();
  void drawAircraft(const QVector3D &position, const QQuaternion &orientation);
  QVector<Vertex> createAircraftModel();
  bool m_cameraFollowAircraft; // 跟随摄像机
  void keyPressEvent(QKeyEvent *event);
  QVector3D m_viewTranslation; // 图平移
public slots:
  void stopSimulation();

  void setMaxAltitude(double altitude) {
    m_maxAltitude = altitude;
    update();
  }
};