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

private:
  void setupOpenGLContext();
  void initCanvas();
  std::shared_ptr<gl::BasePlane> initBasePlane();
  
private:
  RoutePlanner *m_routePlanner = nullptr;
  QPoint mLastMousePos;
  float mfDistance = -5.0f;
  float mfFlightHight = 50.0f; //高
  QVector<QVector3D> m_currentRoute;
public slots:
  void updateFlightHeight(double height);
  void handleMouseMove(QMouseEvent *event);

  void generateFlightRoute(float height); // 生成航线
  void startSimulation(float speed);      // 开始模拟
  void pauseSimulation();                 // 暂停模拟
  void resumeSimulation();                // 继续模拟
  void returnToHome();                    // 自动返回
  void stopSimulation();
signals:
  void glInitialized();
};