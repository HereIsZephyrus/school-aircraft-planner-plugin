/****************************************************************************
File:MyOpenGLWidget.h
Author:wkj
Date:2025.3.13
****************************************************************************/
#pragma once
#include "../log/qgis_debug.h"
#include "../opengl/Primitive.h"
#include "../core/RoutePlanner.h"
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
  QVector3D getSurfacePointFromMouse();
 
protected:
  std::shared_ptr<gl::Model> modelWidget;
  std::shared_ptr<gl::BasePlane> basePlaneWidget;
  std::shared_ptr<gl::ControlPoints> ControlPointsWidget;
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int w, int h) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;
  std::unique_ptr<QTimer> updateTimer;

private:
  void initCanvas();
  std::shared_ptr<gl::BasePlane> initBasePlane();
  
private:
  QPoint mLastMousePos;
public slots:
  void handleMouseMove(QMouseEvent *event);
  void loadModel(const QString& objFilePath);
signals:
  void glInitialized();
};
 /*
  void addControlPoint(const QVector3D &point);
  void setRoutePlanner(RoutePlanner *planner);
  void drawPathSection(const QVector<QVector3D> &points, const QVector4D &color,
                       float lineWidth, bool dashed);
  */
  //void drawControlPoints();
  //void drawConvexHull();
  //void drawRoutePath();
  //void drawBasePlane();

  //RoutePlanner *m_routePlanner = nullptr;
  //QVector<QVector3D> m_currentRoute;