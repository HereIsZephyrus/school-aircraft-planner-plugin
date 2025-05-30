#include "Canvas.h"

Canvas::Canvas(QStackedWidget *parent) : QStackedWidget(parent){

}

// switch to 3D
void Canvas::switchTo3D() {
  if (!ws::WindowManager::getInstance().get3DMapInited()) {
    logMessage("3D map not initialized", Qgis::MessageLevel::Info);
    init3DWidget();
    logMessage("3D map initialized", Qgis::MessageLevel::Success);
  }
  mpStackedWidget->setCurrentWidget(
      mpOpenGLWidget.get()); // switch to 3D model view
  ws::WindowManager::getInstance().setCurrentCanvas(ws::CanvasType::ThreeD);
  logMessage("switch to 3D model view", Qgis::MessageLevel::Success);
}
// switch to 2D
void Canvas::switchTo2D() {
  if (!ws::WindowManager::getInstance().get2DMapInited()) {
    logMessage("2D map not initialized", Qgis::MessageLevel::Info);
    init2DWidget();
    logMessage("2D map initialized", Qgis::MessageLevel::Success);
  }
  logMessage("switch to 2D map view", Qgis::MessageLevel::Info);
  mpStackedWidget->setCurrentWidget(mpImageLabel); // switch to 2D map view
  ws::WindowManager::getInstance().setCurrentCanvas(ws::CanvasType::TwoD);
  logMessage("switch to 2D map view", Qgis::MessageLevel::Success);
}

void Canvas::resetView() {
  Camera::getInstance().resetView();
  logMessage("reset view", Qgis::MessageLevel::Success);
}


void Canvas::init3DWidget() {
  mpRoutePlanner = std::make_unique<RoutePlanner>();
  mpOpenGLWidget = new OpenGLCanvas(this);

  // connect(mpOpenGLWidget.get(), &OpenGLCanvas::glInitialized, this,
  // &MainWindow::switchTo3D);

  // logMessage("canvas initialized", Qgis::MessageLevel::Success);

  // pass RoutePlanner instance to OpenGLWidget
  // mpOpenGLWidget->setRoutePlanner(mpRoutePlanner.get());
}
void Canvas::init2DWidget() {
  // create QLabel to display local image
  mpImageLabel = new QLabel(this);
  QPixmap mapImage(
      ":/schoolcore/map/capture.png"); // use resource path to load image
  if (mapImage.isNull()) {
    logMessage("failed to load local map image", Qgis::MessageLevel::Critical);
    return;
  }
  mpImageLabel->setPixmap(mapImage);
  mpImageLabel->setScaledContents(
      true); // let image adapt to label size, keep ratio
  mpImageLabel->setSizePolicy(QSizePolicy::Ignored,
                              QSizePolicy::Ignored); // set size policy
  logMessage("create QLabel to display local map image",
             Qgis::MessageLevel::Success);
}