#include "OpenGLCanvas.h"
#include "../core/RoutePlanner.h"
#include "../log/QgisDebug.h"
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMouseEvent>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QTextStream>
#include <QTransform>
#include <QWheelEvent>
#include <memory>
#include <qgis.h>
#include <qgsapplication.h>
#include <qtimer.h>
#include <qvector4d.h>

OpenGLCanvas::OpenGLCanvas(QWidget *parent) : QOpenGLWidget(parent) {
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
  connect(updateTimer.get(), &QTimer::timeout, this,
          QOverload<>::of(&QOpenGLWidget::update));
  updateTimer->start();
}

OpenGLCanvas::~OpenGLCanvas() {
  logMessage("ready to destroy OpenGLCanvas", Qgis::MessageLevel::Info);
  
  if (updateTimer) {
    updateTimer->stop();
    updateTimer = nullptr;
  }
  
  QOpenGLContext* ctx = context();
  if (!ctx) {
    logMessage("OpenGL context is null", Qgis::MessageLevel::Critical);
    return;
  }
  
  if (!ctx->isValid()) {
    logMessage("OpenGL context is not valid", Qgis::MessageLevel::Critical);
    return;
  }
  
  logMessage("OpenGL context is valid", Qgis::MessageLevel::Info);
  
  try {
    makeCurrent();
    logMessage("OpenGLCanvas::makeCurrent", Qgis::MessageLevel::Info);
    mpScene = nullptr;
    doneCurrent();
    logMessage("OpenGLCanvas::doneCurrent", Qgis::MessageLevel::Info);
  } catch (const std::exception& e) {
    logMessage("Error during OpenGL cleanup: " + QString(e.what()), Qgis::MessageLevel::Critical);
  }
  
  logMessage("OpenGLCanvas destroyed", Qgis::MessageLevel::Success);
}

void OpenGLCanvas::initializeGL() {
  if (!context()->isValid()) {
    logMessage("Invalid OpenGL context", Qgis::MessageLevel::Critical);
    return;
  }
  initializeOpenGLFunctions();
  // logMessage("initialize opengl functions", Qgis::MessageLevel::Info);

  QString version = QString::fromUtf8((const char *)glGetString(GL_VERSION));
  logMessage("OpenGL Version: " + version, Qgis::MessageLevel::Info);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  mpScene = std::make_unique<OpenGLScene>();
  logMessage("OpenGL scene initialized", Qgis::MessageLevel::Success);
}

void OpenGLCanvas::resizeGL(int w, int h) {
  double aspectRatio = static_cast<double>(w) / static_cast<double>(h);
  Camera::getInstance().setAspectRatio(aspectRatio);
}

void OpenGLCanvas::paintGL() {
  if (!isValid()) {
    logMessage("OpenGLCanvas is not valid", Qgis::MessageLevel::Critical);
    return;
  }
  if (!isVisible())
    return;
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  Camera &camera = Camera::getInstance();
  QMatrix4x4 view = camera.viewMatrix();
  QMatrix4x4 projection = camera.projectionMatrix();

  mpScene->paintScene(view, projection);
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    logMessage(QString("OpenGL error in paintGL: %1").arg(err), 
               Qgis::MessageLevel::Critical);
  }
}

OpenGLScene::OpenGLScene() {
  basePlaneWidget = std::make_shared<gl::BasePlane>();
}
OpenGLScene::~OpenGLScene() {
  basePlaneWidget = nullptr;
  modelWidget = nullptr;
  routes.clear();
}
void OpenGLScene::paintScene(const QMatrix4x4 &view,
                             const QMatrix4x4 &projection) {
  if (basePlaneWidget){
    logMessage("basePlaneWidget", Qgis::MessageLevel::Info);
    basePlaneWidget->draw(view, projection);
  }
  if (modelWidget){
    logMessage("modelWidget", Qgis::MessageLevel::Info);
    modelWidget->draw(view, projection);
  }
}

void OpenGLCanvas::mousePressEvent(QMouseEvent *event) {
    mLastMousePos = event->pos();
}

void OpenGLCanvas::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        QPoint delta = event->pos() - mLastMousePos;
        Camera::getInstance().handleMouseMove(delta);
        mLastMousePos = event->pos();
        update();
    }
}

void OpenGLCanvas::wheelEvent(QWheelEvent *event) {
    Camera::getInstance().handleMouseWheel(event->angleDelta().y() / 120);
    update();
}

void OpenGLCanvas::keyPressEvent(QKeyEvent *event) {
    float step = 1.0f;
    switch (event->key()) {
    case Qt::Key_W:
        Camera::getInstance().moveForward(step);
        break;
    case Qt::Key_S:
        Camera::getInstance().moveBackward(step);
        break;
    case Qt::Key_A:
        Camera::getInstance().moveLeft(step);
        break;
    case Qt::Key_D:
        Camera::getInstance().moveRight(step);
        break;
    case Qt::Key_Q:
        Camera::getInstance().moveUp(step);
        break;
    case Qt::Key_E:
        Camera::getInstance().moveDown(step);
        break;
    case Qt::Key_R:
        Camera::getInstance().resetView();
        break;
    default:
        QOpenGLWidget::keyPressEvent(event);
        return;
    }
    update();
}

void OpenGLCanvas::loadModel(const QString &objFilePath) {
  mpScene->loadModel(objFilePath);
}
void OpenGLScene::loadModel(const QString &objFilePath) {
  modelWidget = std::make_shared<gl::Model>(objFilePath);
  logMessage("Model loaded", Qgis::MessageLevel::Success);
}
