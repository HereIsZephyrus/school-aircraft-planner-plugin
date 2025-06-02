/****************************************************************************
File:OpenGLCanvas.h
Author:wkj
Date:2025.3.13
****************************************************************************/
#ifndef Canvas_H
#define Canvas_H
//#include <GL/gl.h>
#include "../opengl/Camera.h"
#include "../opengl/Primitive.h"
#include "../core/RoutePlanner.h"
#include <QObject>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QOffscreenSurface>
#include <QVector2D>
#include <QVector3D>
#include <QVector>
#include <QtMath>
#include <cfloat>
#include <memory>

class OpenGLScene;
class OpenGLCanvas : public QOpenGLWidget, protected QOpenGLFunctions {
public:
  OpenGLCanvas(QWidget *parent = nullptr);
  ~OpenGLCanvas();
  QVector3D getSurfacePointFromMouse();
  QOpenGLContext* getSharedContext() const { return mSharedContext; }

protected:
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int w, int h) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;
  std::unique_ptr<QTimer> updateTimer;
  std::unique_ptr<OpenGLScene> mpScene;

private:
  QPoint mLastMousePos;
  QOpenGLContext* mSharedContext;
  void initializeSharedContext();
  std::shared_ptr<gl::BasePlane> basePlaneWidget;

public slots:
  void handleMouseMove(QMouseEvent *event);
  void loadModel(const QString &objFilePath);
};

class OpenGLScene {
public:
  OpenGLScene(QOpenGLContext* sharedContext = nullptr);
  ~OpenGLScene();
  void paintScene(const QMatrix4x4 &view, const QMatrix4x4 &projection);
  std::shared_ptr<gl::BasePlane> initBasePlane();
  void loadModel(const QString &objFilePath);
  void setSharedContext(QOpenGLContext* context){mSharedContext = context;}
  void cleanupResources();

protected:
  std::shared_ptr<gl::Model> modelWidget;
  std::shared_ptr<gl::BasePlane> basePlaneWidget;
  QVector<std::shared_ptr<Route>> routes;

private:
  QOpenGLContext* mSharedContext;
  QOffscreenSurface* mSurface;
};
#endif // Canvas_H