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
#include "../core/WorkspaceState.h"

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

  // Initialize shared context manager
  if (!SharedContextManager::getInstance().initialize(context())) {
    logMessage("Failed to initialize shared context manager", Qgis::MessageLevel::Critical);
    return;
  }

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

  if (mpScene) {
    mpScene->paintScene(view, projection);
  }
  
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    logMessage(QString("OpenGL error in paintGL: %1").arg(err), 
               Qgis::MessageLevel::Critical);
  }
}

OpenGLScene::OpenGLScene() {
    auto& sharedContextManager = SharedContextManager::getInstance();
    if (!sharedContextManager.isValid()) {
        logMessage("Shared context manager is not valid in OpenGLScene constructor", Qgis::MessageLevel::Critical);
        return;
    }

    if (sharedContextManager.makeCurrent()) {
        basePlaneWidget = std::make_shared<gl::BasePlane>();
        sharedContextManager.doneCurrent();
    }
}

OpenGLScene::~OpenGLScene() {
    cleanupResources();
    routes.clear();
}

void OpenGLScene::cleanupResources() {
    auto& sharedContextManager = SharedContextManager::getInstance();
    if (!sharedContextManager.isValid()) {
        return;
    }

    if (sharedContextManager.makeCurrent()) {
        if (modelWidget) {
            modelWidget->cleanupTextures();
        }
        sharedContextManager.doneCurrent();
    }
}

void OpenGLScene::paintScene(const QMatrix4x4 &view, const QMatrix4x4 &projection) {
    auto& sharedContextManager = SharedContextManager::getInstance();
    if (!sharedContextManager.isValid()) {
        logMessage("Shared context manager is not valid", Qgis::MessageLevel::Critical);
        return;
    }

    if (sharedContextManager.makeCurrent()) {
        if (basePlaneWidget) {
            basePlaneWidget->draw(view, projection);
        }
        if (modelWidget) {
            modelWidget->draw(view, projection);
        }
        sharedContextManager.doneCurrent();
    }
}

void OpenGLScene::loadModel(const QString &objFilePath) {
    logMessage("OpenGLScene::loadModel", Qgis::MessageLevel::Info);
    
    auto& sharedContextManager = SharedContextManager::getInstance();
    if (!sharedContextManager.isValid()) {
        logMessage("Shared context manager is not valid", Qgis::MessageLevel::Critical);
        return;
    }

    // Clean up old resources before loading new model
    cleanupResources();

    if (sharedContextManager.makeCurrent()) {
        modelWidget = std::make_shared<gl::Model>(objFilePath);
        sharedContextManager.doneCurrent();
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
