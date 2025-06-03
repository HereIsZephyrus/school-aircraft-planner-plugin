#include "OpenGLCanvas.h"
#include "../core/RoutePlanner.h"
#include "../core/SharedContextManager.h"
#include "../log/QgisDebug.h"
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMouseEvent>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QOffscreenSurface>
#include <QTextStream>
#include <QTransform>
#include <QWheelEvent>
#include <memory>
#include <qgis.h>
#include <qgsapplication.h>
#include <qtimer.h>
#include <qvector4d.h>
#include <qthread.h>
#include "../core/WorkspaceState.h"

OpenGLCanvas::OpenGLCanvas(QWidget *parent) : QOpenGLWidget(parent) {
  QSurfaceFormat format;
  format.setVersion(4, 1);
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setDepthBufferSize(24);
  format.setStencilBufferSize(8);
  format.setSamples(4);
  format.setOption(QSurfaceFormat::DebugContext);
  setFormat(format);
  setFocusPolicy(Qt::StrongFocus);
  setFocus();

  // set timer
  updateTimer = std::make_unique<QTimer>();
  updateTimer->setInterval(16);
  updateTimer->moveToThread(QApplication::instance()->thread());
  connect(updateTimer.get(), &QTimer::timeout, this,
          QOverload<>::of(&QOpenGLWidget::update), Qt::QueuedConnection);
  updateTimer->start();

  // set camera
  double width = static_cast<double>(this->width());
  double height = static_cast<double>(this->height());
  Camera::getInstance().setAspectRatio(width / height);
}

OpenGLCanvas::~OpenGLCanvas() {
  logMessage("ready to destroy OpenGLCanvas", Qgis::MessageLevel::Info);
  
  if (updateTimer) {
    updateTimer->stop();
    updateTimer = nullptr;
  }
  
  if (mpScene) {
    mpScene->cleanupResources();
    mpScene = nullptr;
  }
  
  logMessage("OpenGLCanvas destroyed", Qgis::MessageLevel::Success);
}

void OpenGLCanvas::initializeGL() {
  if (!context()->isValid()) {
    logMessage("Invalid OpenGL context", Qgis::MessageLevel::Critical);
    return;
  }
  initializeOpenGLFunctions();

  QString version = QString::fromUtf8((const char *)glGetString(GL_VERSION));
  logMessage("OpenGL Version: " + version, Qgis::MessageLevel::Info);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Initialize shared context manager with our surface
  if (!SharedContextManager::getInstance().initialize(context())) {
    logMessage("Failed to initialize shared context manager", Qgis::MessageLevel::Critical);
    return;
  }

  mpScene = std::make_unique<OpenGLScene>(context());
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

  if (mpScene) {
    mpScene->paintScene(view, projection);
  }
  
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    logMessage(QString("OpenGL error in paintGL: %1").arg(err), 
               Qgis::MessageLevel::Critical);
  }
}

OpenGLScene::OpenGLScene(QOpenGLContext* context) {
    this->context = context;
    context->makeCurrent(context->surface());
    basePlaneWidget = std::make_shared<gl::BasePlane>();
    const QString objFilePath = "/mnt/repo/comprehensive3S/test/Tile_+000_+000.obj";
    //modelWidget = std::make_shared<gl::Model>(objFilePath);
    context->doneCurrent();
    logMessage("OpenGLScene initialized", Qgis::MessageLevel::Success);
}

OpenGLScene::~OpenGLScene() {
    cleanupResources();
    routes.clear();
}

void OpenGLScene::cleanupResources() {
    if (context->makeCurrent(context->surface())) {
        if (modelWidget) {
            modelWidget->cleanupTextures();
        }
        context->doneCurrent();
    }
}

void OpenGLScene::paintScene(const QMatrix4x4 &view, const QMatrix4x4 &projection) {
    if (!QOpenGLContext::currentContext()) {
        logMessage("OpenGL context is not current", Qgis::MessageLevel::Critical);
        return;
    }
    if (basePlaneWidget) {
        basePlaneWidget->draw(view, projection);
    }
    if (modelWidget) {
        modelWidget->draw(view, projection);
    }
}

void OpenGLScene::loadModel(const QString &objFilePath) {
    logMessage("OpenGLScene::loadModel", Qgis::MessageLevel::Info);
    if (!QOpenGLContext::currentContext()) {
        logMessage("OpenGL context is not current", Qgis::MessageLevel::Critical);
        return;
    }
    // Clean up old resources before loading new model
    if (modelWidget)
      modelWidget->cleanupTextures();

    if (context->makeCurrent(context->surface())) {
        modelWidget = std::make_shared<gl::Model>(objFilePath);
        context->doneCurrent();
        logMessage("Model loaded", Qgis::MessageLevel::Success);
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
    ws::WindowManager::getInstance().keyPressEvent(event);
    update();
}

void OpenGLCanvas::keyReleaseEvent(QKeyEvent *event) {
    ws::WindowManager::getInstance().keyReleaseEvent(event);
    update();
}

void OpenGLCanvas::loadModel(const QString &objFilePath) {
    logMessage("OpenGLCanvas::loadModel", Qgis::MessageLevel::Info);
    makeCurrent();
    mpScene->loadModel(objFilePath);
    doneCurrent();
}
