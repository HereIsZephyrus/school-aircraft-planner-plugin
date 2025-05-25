/****************************************************************************
File:MyOpenGLWidget.h
Author:wkj
Date:2025.3.13
****************************************************************************/
#pragma once
#include "RoutePlanner.h"
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

class RoutePlanner;
class MyOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
  Q_OBJECT

public:
  MyOpenGLWidget(QWidget *parent = nullptr);
  ~MyOpenGLWidget();
  void loadObjModel(const QString &filePath, const QString &texturePath);
  void resetView();
  QVector3D getSurfacePointFromMouse();
  void addControlPoint(const QVector3D &point);
  void setRoutePlanner(RoutePlanner *planner);
  void drawPathSection(const QVector<QVector3D> &points, const QVector4D &color,
                       float lineWidth, bool dashed);

protected:
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int w, int h) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;
  QList<QOpenGLTexture *> m_textures; // 存储纹理
  void drawControlPoints();
  void drawConvexHull();
  void drawRoutePath();
  void drawBasePlane();
  struct Vertex {
    QVector3D position;
    QVector2D texCoord;
    // 默认构造函数
    Vertex() : position(QVector3D(0, 0, 0)), texCoord(QVector3D(0, 0, 0)) {}

    Vertex(QVector3D pos, QVector2D tex) : position(pos), texCoord(tex) {}
  };

private:
  RoutePlanner *m_routePlanner = nullptr;
  QMatrix4x4 mViewMatrix;

  // struct ModelData {
  //      QVector<Vertex> vertices;          // 存储顶点
  //      QOpenGLTexture* texture = nullptr; // 存储纹理
  //      QOpenGLVertexArrayObject vao;      // VAO
  //      QOpenGLBuffer vbo;                 // VBO
  //  };
  //  添加ModelData结构体存储模型信息
  struct Material {
    QString name;
    QVector3D ambient = QVector3D(0.2f, 0.2f, 0.2f);
    QVector3D diffuse = QVector3D(0.8f, 0.8f, 0.8f);
    QVector3D specular = QVector3D(1.0f, 1.0f, 1.0f);
    float shininess = 32.0f;
    QString diffuseTexture;
  };
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
  void initShaders();
  void initBuffers();
  QVector<Vertex> m_vertices;                 // 修改后的顶点数据
  QOpenGLTexture *m_currentTexture = nullptr; // 当前使用的纹理
  QVector<QVector<Vertex>> m_allVertices; // 存储所有模型的顶点数据
  QList<QOpenGLTexture *> m_allTextures;  // 存储所有纹理
  QVector<QVector2D> m_texCoords;         // 存储纹理坐标
  QOpenGLBuffer mVBO;
  QOpenGLVertexArrayObject mVAO;
  QOpenGLShaderProgram mShaderProgram;
  QOpenGLTexture *m_texture = nullptr;
  QMatrix4x4 mProjection;
  QMatrix4x4 mModelView;
  QPoint m_lastMousePos;
  float mfDistance = -5.0f;
  float mfFlightHight = 50.0f; //高
  QVector<QVector3D> m_currentRoute;
  // 存储点数据的缓冲区
  QOpenGLBuffer m_pointVBO;
  QOpenGLBuffer m_hullVBO;
  QOpenGLBuffer m_routeVBO;
  QOpenGLVertexArrayObject m_pointVAO;
  QOpenGLVertexArrayObject m_hullVAO;
  QOpenGLVertexArrayObject m_routeVAO;

  // 绘制线
  QOpenGLShaderProgram m_lineShader;
  QVector3D calculateRayIntersection(const QVector3D &origin,
                                     const QVector3D &direction);
  float m_baseHeight = 25.0f; // 初始标准高度
  float m_initialBaseHeight = 25.0f;
  QOpenGLVertexArrayObject m_basePlaneVAO;
  QOpenGLBuffer m_basePlaneVBO{QOpenGLBuffer::VertexBuffer};
public slots:
  void updateFlightHeight(double height);
  void handleMouseMove(QMouseEvent *event);
  void setBaseHeight(double height);

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

private:
  double m_maxAltitude = 500.0; // 默认最大高度
public:
  void loadMtl(const QString &mtlPath, ModelData *modelData);
  QVector3D calculateModelCenter(ModelData *modelData);

  struct ModelBounds {
    QVector3D min;
    QVector3D max;
    QVector3D center;
    QVector3D originalCenter; // 存储原始中心
  };

  struct GlobalBounds {
    QVector3D sceneMin = QVector3D(FLT_MAX, FLT_MAX, FLT_MAX);
    QVector3D sceneMax = QVector3D(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    QVector3D sceneCenter;
  } m_globalBounds;

  QMap<ModelData *, ModelBounds> m_modelBoundsMap; // 模型边界映射
  void calculateModelBounds(ModelData *modelData, ModelBounds &bounds);
  void updateGlobalBounds(const ModelBounds &modelBounds);
  void applyGlobalCentering();
};