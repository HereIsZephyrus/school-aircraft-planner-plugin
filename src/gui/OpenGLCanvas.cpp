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

void OpenGLCanvas::initializeSharedContext() {
  if (!context()) {
    logMessage("Cannot create shared context: main context is null", Qgis::MessageLevel::Critical);
    return;
  }

  QSurfaceFormat format;
  format.setVersion(4, 1);
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setDepthBufferSize(24);
  format.setStencilBufferSize(8);
  format.setSamples(4);
  
  mSharedContext = new QOpenGLContext();
  mSharedContext->setFormat(format);
  mSharedContext->setShareContext(context());
  
  if (!mSharedContext->create()) {
    logMessage("Failed to create shared context", Qgis::MessageLevel::Critical);
    delete mSharedContext;
    mSharedContext = nullptr;
    return;
  }

  if (mSharedContext->shareContext() != context()) {
    logMessage("Context sharing failed", Qgis::MessageLevel::Critical);
    delete mSharedContext;
    mSharedContext = nullptr;
    return;
  }
}

OpenGLCanvas::~OpenGLCanvas() {
  logMessage("ready to destroy OpenGLCanvas", Qgis::MessageLevel::Info);
  
  if (updateTimer) {
    updateTimer->stop();
    updateTimer = nullptr;
  }
  
  // 确保在正确的上下文中清理资源
  if (mpScene) {
    mpScene->cleanupResources();
  }
  
  if (mSharedContext) {
    mSharedContext->doneCurrent();
    delete mSharedContext;
    mSharedContext = nullptr;
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

  QString version = QString::fromUtf8((const char *)glGetString(GL_VERSION));
  logMessage("OpenGL Version: " + version, Qgis::MessageLevel::Info);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  basePlaneWidget = std::make_shared<gl::BasePlane>();
  initializeSharedContext();
  
  if (!mSharedContext) {
    logMessage("Failed to initialize shared context", Qgis::MessageLevel::Critical);
    return;
  }

  mpScene = std::make_unique<OpenGLScene>(mSharedContext);
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

  if (basePlaneWidget)
    basePlaneWidget->draw(view, projection);

  if (mpScene) {
    mpScene->paintScene(view, projection);
  }
  
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    logMessage(QString("OpenGL error in paintGL: %1").arg(err), 
               Qgis::MessageLevel::Critical);
  }
}

OpenGLScene::OpenGLScene(QOpenGLContext* sharedContext) : mSharedContext(sharedContext) {
  if (!mSharedContext) {
    logMessage("Shared context is null in OpenGLScene constructor", Qgis::MessageLevel::Critical);
    return;
  }
  logMessage("Shared context is valid in OpenGLScene constructor", Qgis::MessageLevel::Info);

  mSurface = new QOffscreenSurface();
  mSurface->setFormat(mSharedContext->format());
  mSurface->create();

  if (!mSurface->isValid()) {
    logMessage("Failed to create offscreen surface", Qgis::MessageLevel::Critical);
    delete mSurface;
    mSurface = nullptr;
    return;
  }

  mSharedContext->makeCurrent(mSurface);
  basePlaneWidget = std::make_shared<gl::BasePlane>();
  mSharedContext->doneCurrent();
}

OpenGLScene::~OpenGLScene() {
  cleanupResources();
  
  if (mSurface) {
    delete mSurface;
    mSurface = nullptr;
  }
  
  routes.clear();
}

void OpenGLScene::cleanupResources() {
  if (!mSharedContext || !mSurface) {
    return;
  }

  mSharedContext->makeCurrent(mSurface);
  
  if (modelWidget) {
    modelWidget->cleanupTextures();
  }
  
  basePlaneWidget = nullptr;
  modelWidget = nullptr;
  
  mSharedContext->doneCurrent();
}

void OpenGLScene::paintScene(const QMatrix4x4 &view, const QMatrix4x4 &projection) {
  if (!mSharedContext || !mSurface) {
    logMessage("Shared context or surface is null", Qgis::MessageLevel::Critical);
    return;
  }

  mSharedContext->makeCurrent(mSurface);
  if (basePlaneWidget)
    basePlaneWidget->draw(view, projection);
  if (modelWidget)
    modelWidget->draw(view, projection);
  mSharedContext->doneCurrent();
}

void OpenGLScene::loadModel(const QString &objFilePath) {
  logMessage("OpenGLScene::loadModel", Qgis::MessageLevel::Info);
  if (!mSharedContext || !mSurface) {
    logMessage("Shared context or surface is null", Qgis::MessageLevel::Critical);
    return;
  }

  // 在加载新模型前清理旧资源
  cleanupResources();

  mSharedContext->makeCurrent(mSurface);
  modelWidget = std::make_shared<gl::Model>(objFilePath);
  mSharedContext->doneCurrent();
  logMessage("Model loaded", Qgis::MessageLevel::Success);
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
