#include "Canvas.h"
#include "../core/WorkspaceState.h"
#include "../log/QgisDebug.h"
#include "OpenGLCanvas.h"
#include <QStandardPaths>
#include "qgsmaptoolpan.h"

Canvas::Canvas(QWidget *parent) : QStackedWidget(parent) {
  init2DWidget();
  init3DWidget();
  setCurrentWidget(mpOpenGLWidget);
  connect(mpOpenGLWidget, &OpenGLCanvas::refreash3DParms, this, &Canvas::refreashParms);
  logMessage("create canvas", Qgis::MessageLevel::Success);
}

// switch to 3D
void Canvas::switchTo3D() {
  wsp::WindowManager::getInstance().setCurrentCanvas(wsp::CanvasType::ThreeD);
  setCurrentWidget(mpOpenGLWidget);
  logMessage("switch to 3D model view", Qgis::MessageLevel::Success);
}
// switch to 2D
void Canvas::switchTo2D() {
  wsp::WindowManager::getInstance().setCurrentCanvas(wsp::CanvasType::TwoD);
  // setCurrentWidget(mpImageLabel); // switch to 2D map view
  setCurrentWidget(mpMapCanvas);
  logMessage("switch to 2D map view", Qgis::MessageLevel::Success);
}

void Canvas::refreshQgsMapCanvas() {
    mpMapCanvas->setLayers(QgsProject::instance()->layerTreeRoot()->checkedLayers());
    mpMapCanvas->refresh();
}

void Canvas::viewReset() {
  Camera::getInstance().resetView();
  logMessage("reset view", Qgis::MessageLevel::Success);
}

void Canvas::init3DWidget() {
  mpOpenGLWidget = new OpenGLCanvas(this);
  addWidget(mpOpenGLWidget);
  logMessage("create 3D view widget", Qgis::MessageLevel::Success);
}
void Canvas::init2DWidget() {
  //  Created by INMIDA
  mpMapCanvas = new QgsMapCanvas(this);
  mpMapCanvas->enableAntiAliasing(true);
  connect(LayerTreeWidget::getInstance(),&LayerTreeWidget::refreshQgsMapCanvas,this,&Canvas::refreshQgsMapCanvas);
  // default pantool
  QgsMapToolPan *panTool = new QgsMapToolPan(mpMapCanvas);
  mpMapCanvas->setMapTool(panTool);
  mpMapCanvas->setCanvasColor(QColor(25,25,25));
  LayerTreeWidget::getInstance()->setMapCanvasBridge(new QgsLayerTreeMapCanvasBridge(QgsProject::instance()->layerTreeRoot(),mpMapCanvas));
  // load init data
  QString appDir = QCoreApplication::applicationDirPath();
  QString realPath = appDir + "/resources/map/project.qgs";
  if (QgsProject::instance()->read(realPath)) {
      QgsProject::instance()->setFileName(realPath);
      logMessage("Project loaded from: " + realPath, Qgis::MessageLevel::Success);
      emit refreshQgsMapCanvas();
  } else {
      logMessage("Failed to load project from: " + realPath, Qgis::MessageLevel::Critical);
  }
  addWidget(mpMapCanvas);
}

Canvas::~Canvas() {
    delete mpOpenGLWidget;
    delete mpMapCanvas;
    logMessage("Canvas destroyed", Qgis::MessageLevel::Success);
}
