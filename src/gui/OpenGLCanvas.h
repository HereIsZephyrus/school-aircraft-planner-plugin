/****************************************************************************
File:OpenGLCanvas.h
Author:wkj
Date:2025.3.13
****************************************************************************/
#ifndef OPENGLCANVAS_H
#define OPENGLCANVAS_H
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
    Q_OBJECT

public:
    explicit OpenGLCanvas(QWidget *parent = nullptr);
    ~OpenGLCanvas();
    QVector3D getSurfacePointFromMouse();

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

public slots:
    void loadModel(const QString &objFilePath);
};

class OpenGLScene {
public:
    OpenGLScene(QOpenGLContext* context);
    ~OpenGLScene();

    void paintScene(const QMatrix4x4 &view, const QMatrix4x4 &projection);
    void loadModel(const QString &objFilePath);
    void cleanupResources();

protected:
    std::shared_ptr<gl::Model> modelWidget;
    std::shared_ptr<gl::BasePlane> basePlaneWidget;
    QVector<std::shared_ptr<Route>> routes;
    QOpenGLContext* context;
};

#endif // OPENGLCANVAS_H