#include "MyOpenGLWidget.h"
#include "../opengl/camera.h"
#include "../log/qgis_debug.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMouseEvent>
#include <QOpenGLFunctions_3_3_Core>
#include <QTextStream>
#include <QTransform>
#include <QWheelEvent>
#include <memory>
#include <qgis.h>
#include <qgsapplication.h>
#include <qtimer.h>
#include <qvector4d.h>
#include <QOpenGLFunctions_4_1_Core>

MyOpenGLWidget::MyOpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent) {
  QSurfaceFormat format;
  format.setVersion(4, 1);
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setDepthBufferSize(24);
  format.setStencilBufferSize(8);
  format.setSamples(4);
  setFormat(format);
  
  setFocusPolicy(Qt::StrongFocus);
  setFocus();
  
  // set timer
  updateTimer = std::make_unique<QTimer>();
  updateTimer->setInterval(16);
  connect(updateTimer.get(), &QTimer::timeout, this, QOverload<>::of(&QOpenGLWidget::update));
  updateTimer->start();

  modelWidget = nullptr;
  basePlaneWidget = nullptr;
  ControlPointsWidget = nullptr;
}

MyOpenGLWidget::~MyOpenGLWidget() {
  logMessage("ready to destroy MyOpenGLWidget", Qgis::MessageLevel::Info);
  makeCurrent();
  modelWidget = nullptr;
  basePlaneWidget = nullptr;
  ControlPointsWidget = nullptr;
  doneCurrent();
  updateTimer->stop();
  logMessage("MyOpenGLWidget destroyed", Qgis::MessageLevel::Success);
}

void MyOpenGLWidget::initCanvas() {
  basePlaneWidget = std::make_shared<gl::BasePlane>();
  // ControlPointsWidget = initControlPoints();
}

void MyOpenGLWidget::initializeGL() {
  
  initializeOpenGLFunctions();
  logMessage("initialize opengl functions", Qgis::MessageLevel::Info);
  
  if (!QOpenGLContext::currentContext()) {
    logMessage("No OpenGL context available", Qgis::MessageLevel::Critical);
    return;
  }
  
  QString version = QString::fromUtf8((const char *)glGetString(GL_VERSION));
  logMessage("OpenGL Version: " + version, Qgis::MessageLevel::Info);
  
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  makeCurrent();
  initCanvas();
  doneCurrent();
  logMessage("OpenGL context initialized", Qgis::MessageLevel::Success);
  emit glInitialized();
}
/*
  if (m_routePlanner) {
    drawControlPoints();
    drawConvexHull();
    drawRoutePath();
  }
  if (m_isAnimating || m_animationProgress > 0) {
    drawAircraft(m_aircraftPosition, m_aircraftOrientation);
  }
  */

void MyOpenGLWidget::resizeGL(int w, int h) {
  double aspectRatio = static_cast<double>(w) / static_cast<double>(h);
  Camera::getInstance().setAspectRatio(aspectRatio);
}

void MyOpenGLWidget::paintGL() {
  if (!isValid()) {
    logMessage("MyOpenGLWidget is not valid", Qgis::MessageLevel::Critical);
    return;
  }
  if (!isVisible())
    return;
  makeCurrent();
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  if (basePlaneWidget)
    basePlaneWidget->draw();
  if (modelWidget)
    modelWidget->draw();
  doneCurrent();
}

void MyOpenGLWidget::mousePressEvent(QMouseEvent *event) {
/*
  if (m_routePlanner && m_routePlanner->mCreateRoute) {
    // 优先处理添加控制点模式
    if (m_routePlanner->isSettingControlPointsMode() &&
        event->button() == Qt::LeftButton) {
      QVector3D surfacePoint = getSurfacePointFromMouse();
      if (surfacePoint != QVector3D(0, 0, 0)) {
        m_routePlanner->setControlPoints(surfacePoint);
        m_routePlanner->setSettingControlPointsMode(false); // 退出设置模式
        update();
        return;
      }
    }
    if (m_routePlanner->isAddingControlPoint() &&
        event->button() == Qt::LeftButton) {
      QVector3D surfacePoint = getSurfacePointFromMouse();
      if (surfacePoint != QVector3D(0, 0, 0)) {
        qDebug() << "Adding control point with coordinates: "
                 << "X:" << surfacePoint.x() << "Y:" << surfacePoint.y()
                 << "Z:" << surfacePoint.z();
        addControlPoint(surfacePoint); // 添加点并自动退出添加模式
      } else {
        qDebug() << "surfacePoint == QVector3D(0, 0, 0)";
      }
      qDebug() << "111stop mousePressEvent";
      return; // 阻断后续逻辑
    }

    //其次处理编辑模式（移动/删除点）

    if (m_routePlanner->isEditing()) {
      QVector3D surfacePoint = getSurfacePointFromMouse();

      if (event->button() == Qt::LeftButton) {
        // 选择最近的控制点
        float minDist = FLT_MAX;
        int selectedIndex = -1;
        QVector<QVector3D> points = m_routePlanner->controlPoints();

        for (int i = 0; i < points.size(); ++i) {
          float dist = (points[i] - surfacePoint).lengthSquared();
          if (dist < minDist) {
            qDebug() << "选择最近的控制点";
            minDist = dist;
            selectedIndex = i;
          }
        }

        if (minDist < 10000000.0f) { // 10单位的平方
          qDebug() << "minDist < 1000.0f";
          m_routePlanner->setSelectedPoint(selectedIndex);
        } else {
          m_routePlanner->setSelectedPoint(-1);
        }
      } else if (event->button() == Qt::RightButton) {
        // 删除选中的点
        m_routePlanner->removeSelectedPoint();
      }

      update();
      return;
    }
  }

  // 默认处理：视图旋转
  m_lastMousePos = event->pos();
*/
}

void MyOpenGLWidget::mouseMoveEvent(QMouseEvent *event) {
  /*
  if (m_cameraFollowAircraft) {
    return; // 跟随视角下不处理旋转
  }
  if (m_isAnimating)
    return;
  if (m_routePlanner) {
    // 优先处理添加控制点模式

    if (m_routePlanner->mCreateRoute) {

      if (m_routePlanner->isAddingControlPoint()) {
        return;
      }
      if (m_routePlanner->isSettingControlPointsMode()) {
        return;
      }
    }
    if (m_routePlanner->isEditing() && m_routePlanner->mCreateRoute) {
      handleMouseMove(event);
    } else {
      // 默认处理：视图旋转
      float dx = event->x() - m_lastMousePos.x();
      float dy = event->y() - m_lastMousePos.y();
      if (event->buttons() & Qt::LeftButton) {
        QMatrix4x4 rotation;
        rotation.rotate(dx, QVector3D(0, 1, 0));
        rotation.rotate(dy, QVector3D(1, 0, 0));
        mModelView = rotation * mModelView;
        update();
      }
      m_lastMousePos = event->pos();
    }
  }
  */
}

void MyOpenGLWidget::wheelEvent(QWheelEvent *event) {
  Camera &camera = Camera::getInstance();
  float currentFov = camera.fieldOfView();
  float zoomFactor = camera.zoomFactor();

  if (event->angleDelta().y() > 0)
    currentFov = qMax(currentFov - zoomFactor, 10.0f);
  else
    currentFov = qMin(currentFov + zoomFactor, 120.0f);

  camera.setFieldOfView(currentFov);
  update();
}

void MyOpenGLWidget::loadModel(const QString& objFilePath){
  modelWidget = std::make_shared<gl::Model>(objFilePath);
  logMessage("Model loaded", Qgis::MessageLevel::Success);
}
/*
void MyOpenGLWidget::setRoutePlanner(RoutePlanner *planner) {
  m_routePlanner = planner;
  connect(planner, &RoutePlanner::dataUpdated, this,
          QOverload<>::of(&QOpenGLWidget::update));
}
// MyOpenGLWidget.cpp 绘制控制点实现
void MyOpenGLWidget::drawControlPoints() {

  if (!m_routePlanner || m_routePlanner->controlPoints().isEmpty())
    return;

  if (!mLineShader->bind()) {
    logMessage("Failed to bind line shader", Qgis::MessageLevel::Critical);
    return;
  }

  glDisable(GL_DEPTH_TEST); // 禁用深度测试
  QMatrix4x4 view;
  view.translate(0, 0, mfDistance);
  view.translate(m_viewTranslation); // 添加平移
  QMatrix4x4 mvp = mProjection * view * mModelView;
  mLineShader->setUniformValue("mvp", mvp);

  // 绘制控制点
  QVector<QVector3D> points = m_routePlanner->controlPoints();
  QVector<float> vertexData;
  double baseHeight = ws::FlightManager::getInstance().getBaseHeight();
  for (const QVector3D &p : points) {
    vertexData << p.x() << p.y() << p.z() + mfFlightHight + baseHeight;
  }

  m_pointVAO.bind();
  m_pointVBO.bind();
  m_pointVBO.allocate(vertexData.constData(),
                      vertexData.size() * sizeof(float));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  // 获取 OpenGL 函数指针并设置点的大小
  QOpenGLFunctions_3_3_Core *functions =
      QOpenGLContext::currentContext()
          ->versionFunctions<QOpenGLFunctions_3_3_Core>();
  if (functions) {
    functions->glPointSize(10.0f);
  }

  // 绘制所有控制点
  mLineShader->setUniformValue("color",
                               QVector4D(1.0f, 0.0f, 0.0f, 1.0f)); // 红色
  glDrawArrays(GL_POINTS, 0, points.size());

  // 绘制选中的点（黄色）
  int selectedIndex = m_routePlanner->selectedPointIndex();
  if (selectedIndex >= 0 && selectedIndex < points.size()) {
    mLineShader->setUniformValue("color",
                                 QVector4D(1.0f, 1.0f, 0.0f, 1.0f)); // 黄色
    glDrawArrays(GL_POINTS, selectedIndex, 1);
  }

  // 绘制Home点（紫色）
  if (m_routePlanner && !m_routePlanner->ControlPoints().isNull()) {
    QVector3D ControlPoints = m_routePlanner->ControlPoints();
    QVector<float> homeVertex;
    homeVertex << ControlPoints.x() << ControlPoints.y() << ControlPoints.z();

    mLineShader->setUniformValue("color",
                                 QVector4D(0.5f, 0.0f, 0.5f, 1.0f)); // 紫色
    m_pointVBO.allocate(homeVertex.constData(),
                        homeVertex.size() * sizeof(float));
    glDrawArrays(GL_POINTS, 0, 1);
  }

  // 清理状态并恢复深度测试
  m_pointVBO.release();
  m_pointVAO.release();
  mLineShader->release();
  glEnable(GL_DEPTH_TEST);
}

// MyOpenGLWidget.cpp 绘制凸包实现
void MyOpenGLWidget::drawConvexHull() {
  if (!m_routePlanner || m_routePlanner->convexHull().size() < 2)
    return;

  mLineShader->bind();
  QMatrix4x4 view;
  view.translate(0, 0, mfDistance);
  QMatrix4x4 mvp = mProjection * view * mModelView;
  mLineShader->setUniformValue("mvp", mvp);
  mLineShader->setUniformValue("color",
                               QVector4D(0.0f, 1.0f, 0.0f, 1.0f)); // 绿色

  // 准备顶点数据
  QVector<QVector3D> hull = m_routePlanner->convexHull();
  QVector<float> vertexData;
  double baseHeight = ws::FlightManager::getInstance().getBaseHeight();
  for (const QVector3D &p : hull) {
    vertexData << p.x() << p.y() << p.z() + mfFlightHight + baseHeight;
  }

  // 设置顶点缓冲
  m_hullVAO.bind();
  m_hullVBO.bind();
  m_hullVBO.allocate(vertexData.constData(), vertexData.size() * sizeof(float));

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  // 绘制设置
  glLineWidth(2.0f);
  glDrawArrays(GL_LINE_LOOP, 0, hull.size());

  // 清理状态
  m_hullVAO.release();
  m_hullVBO.release();
  mLineShader->release();
}

void MyOpenGLWidget::drawRoutePath() {
  if (!m_routePlanner || m_routePlanner->routePath().isEmpty())
    return;

  QVector<QVector3D> route = m_routePlanner->routePath();
  if (route.size() < 2)
    return;
  QMatrix4x4 model = mModelView; // 获取当前模型矩阵
  mLineShader->setUniformValue("model", model);
  mLineShader->bind();
  QMatrix4x4 view;
  view.translate(0, 0, mfDistance);
  QMatrix4x4 mvp = mProjection * view * mModelView;
  mLineShader->setUniformValue("mvp", mvp);

  // 绘制主路径（实线）
  if (route.size() > 2) {
    QVector<QVector3D> mainRoute = route.mid(1, route.size() - 2);
    drawPathSection(mainRoute, QVector4D(0.0f, 0.0f, 1.0f, 1.0f), 2.0f, false);
  }

  // 绘制启程点连接线（虚线）
  if (route.size() >= 2) {
    QVector<QVector3D> startLine = {route[0], route[1]};
    QVector<QVector3D> endLine = {route[route.size() - 2], route.back()};
    drawPathSection(startLine, QVector4D(1.0f, 0.5f, 0.0f, 1.0f), 2.0f, true);
    drawPathSection(endLine, QVector4D(1.0f, 0.5f, 0.0f, 1.0f), 2.0f, true);
  }

  mLineShader->release();
}

void MyOpenGLWidget::drawPathSection(const QVector<QVector3D> &points,
                                     const QVector4D &color, float lineWidth,
                                     bool dashed) {
  if (points.size() < 2)
    return;

  // 设置绘制参数
  mLineShader->setUniformValue("color", color);
  glLineWidth(lineWidth);

  if (dashed) {
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(4, 0x00FF); // 虚线模式
  }

  // 准备顶点数据
  QVector<float> vertexData;
  for (const QVector3D &p : points) {
    vertexData << p.x() << p.y() << p.z() + mfFlightHight + 2.0f;
  }

  // 绘制线段
  m_routeVAO.bind();
  m_routeVBO.bind();
  m_routeVBO.allocate(vertexData.constData(),
                      vertexData.size() * sizeof(float));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glDrawArrays(GL_LINES, 0, points.size());

  // 清理状态
  if (dashed)
    glDisable(GL_LINE_STIPPLE);
  m_routeVBO.release();
  m_routeVAO.release();
}

void MyOpenGLWidget::addControlPoint(const QVector3D &point) {
  if (m_routePlanner) {
    // 直接在基准面高度上添加控制点（不再需要Z轴偏移）

    double baseHeight = ws::FlightManager::getInstance().getBaseHeight();
    QVector3D modelPoint =
        mModelView.inverted() * QVector3D(point.x(), point.y(), baseHeight);

    m_routePlanner->addControlPoint(modelPoint);
    update();
  } else {
    qDebug() << "错误：RoutePlanner 未初始化！";
  }
}
QVector3D MyOpenGLWidget::getSurfacePointFromMouse() {

  // 获取鼠标点击位置
  QPoint mousePos = this->mapFromGlobal(QCursor::pos());

  // 转换为标准化设备坐标
  float x = (2.0f * mousePos.x()) / width() - 1.0f;
  float y = 1.0f - (2.0f * mousePos.y()) / height();
  float z = 1.0f;

  // 计算射线方向
  QMatrix4x4 inverseProjection = mProjection.inverted();
  QVector4D rayEye = inverseProjection * QVector4D(x, y, -1.0f, 1.0f);
  rayEye.setZ(-1.0f);
  rayEye.setW(0.0f);

  QMatrix4x4 inverseView = mModelView.inverted();
  QVector4D rayWorld = inverseView * rayEye;
  QVector3D rayDir = rayWorld.toVector3D().normalized();

  // 计算射线起点（相机位置）
  QVector3D rayOrigin =
      QVector3D(inverseView(0, 3), inverseView(1, 3), inverseView(2, 3));

  return calculateRayIntersection(rayOrigin, rayDir);
}

QVector3D
MyOpenGLWidget::calculateRayIntersection(const QVector3D &rayOrigin,
                                         const QVector3D &rayDirection) {
  float closestDistance = FLT_MAX;
  QVector3D closestPoint;

  // 基准面高度
  double baseHeight = ws::FlightManager::getInstance().getBaseHeight();
  const float planeZ = baseHeight;

  // 组合视图和模型矩阵
  QMatrix4x4 modelViewMatrix = mViewMatrix * mModelView;

  for (ModelData *model : m_models) {
    for (int i = 0; i < model->vertices.size(); i += 3) {
      // 应用模型视图变换到顶点
      QVector3D v0 = modelViewMatrix * model->vertices[i].position;
      QVector3D v1 = modelViewMatrix * model->vertices[i + 1].position;
      QVector3D v2 = modelViewMatrix * model->vertices[i + 2].position;

      // 使用变换后的顶点进行相交检测
      QVector3D edge1 = v1 - v0;
      QVector3D edge2 = v2 - v0;
      QVector3D h = QVector3D::crossProduct(rayDirection, edge2);
      float a = QVector3D::dotProduct(edge1, h);

      if (fabs(a) < 0.0001f)
        continue;

      float f = 1.0f / a;
      QVector3D s = rayOrigin - v0;
      float u = f * QVector3D::dotProduct(s, h);

      if (u < 0.0f || u > 1.0f)
        continue;

      QVector3D q = QVector3D::crossProduct(s, edge1);
      float v = f * QVector3D::dotProduct(rayDirection, q);

      if (v < 0.0f || u + v > 1.0f)
        continue;

      float t = f * QVector3D::dotProduct(edge2, q);
      if (t > 0.0001f && t < closestDistance) {
        closestDistance = t;
        // 计算交点坐标
        QVector3D intersection = rayOrigin + rayDirection * t;

        // 投影到基准面 Z = planeZ
        intersection.setZ(planeZ);
        closestPoint = intersection;
      }
    }
  }

  return closestDistance < FLT_MAX ? closestPoint : QVector3D();
}
*/
void MyOpenGLWidget::handleMouseMove(QMouseEvent *event) {
/*
  QPoint screenPos = event->pos(); // 从 QMouseEvent 获取鼠标位置
  if (m_routePlanner->selectedPointIndex() >= 0) {
    QVector3D surfacePoint = getSurfacePointFromMouse();
    m_routePlanner->m_controlPoints[m_routePlanner->selectedPointIndex()] =
        surfacePoint;
    m_routePlanner->handleMouseMove(event);
  }
*/
}

void MyOpenGLWidget::keyPressEvent(QKeyEvent *event) {
  /*
  if (event->key() == Qt::Key_Space) {
    m_cameraFollowAircraft = !m_cameraFollowAircraft;
    qDebug() << "  m_cameraFollowAircraft=" << m_cameraFollowAircraft;
    update();
  } else {
    float step = 5.0f; // 调整平移步长
    switch (event->key()) {
    case Qt::Key_Left:
      m_viewTranslation += QVector3D(-step, 0, 0);
      update();
      break;
    case Qt::Key_Right:
      m_viewTranslation += QVector3D(step, 0, 0);
      update();
      break;
    case Qt::Key_Up:
      m_viewTranslation += QVector3D(0, step, 0);
      update();
      break;
    case Qt::Key_Down:
      m_viewTranslation += QVector3D(0, -step, 0);
      update();
      break;
    default:
      QOpenGLWidget::keyPressEvent(event);
      return;
    }
  }
  */
}
