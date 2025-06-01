#include "Canvas.h"
#include "../log/QgisDebug.h"

Canvas::Canvas(QWidget *parent) : QStackedWidget(parent){
  logMessage("create canvas", Qgis::MessageLevel::Success);
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

void Canvas::viewReset() {
  Camera::getInstance().resetView();
  logMessage("reset view", Qgis::MessageLevel::Success);
}

void Canvas::loadModel(const QString &objFilePath) {mpOpenGLWidget->loadModel(objFilePath);}

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


/*
void MainWindow::setTianDiTuMap(double lat, double lon, int zoom) {
    QString key = "3e6c3b63b9529d502fc08c5850dfa5d5";

    QString htmlContent =
      QString("<!DOCTYPE html>"
              "<html>"
              "<head>"
              "<title>TianDiTu Map</title>"
              "<link rel='stylesheet' "
              "href='https://unpkg.com/leaflet@1.7.1/dist/leaflet.css'/>"
              "<script "
              "src='https://unpkg.com/leaflet@1.7.1/dist/leaflet.js'></script>"
              "<style>"
              "#map { height: 100%%; width: 100%%; }"
              "body { margin: 0; padding: 0; }"
              "</style>"
              "</head>"
              "<body>"
              "<div id='map'></div>"
              "<script>"
              "var map = L.map('map', {"
              "   crs: L.CRS.EPSG4326," // use EPSG4326 coordinate system
              "   center: [%4, %3],"    // latitude, longitude
              "   zoom: %2"
              "});"
              ""
              "L.tileLayer("
              "   "
              "'http://t0.tianditu.gov.cn/img_c/"
              "wmts?tk=%1&layer=img&style=default&tilematrixset=wgs84&Service="
              "WMTS&Request=GetTile&Version=1.0.0&Format=image/"
              "jpeg&TileMatrix=%2&TileRow=%3&TileCol=%4"
              "    "
              "Request=GetTile&Version=1.0.0&Format=image/"
              "jpeg&TileMatrix={z}&TileRow={y}&TileCol={x}',"
              "   {"
              "       tms: true,"              // enable TMS coordinate system
              "       maxZoom: 18,"            // maximum zoom level
              "       tileSize: 256,"          // tile size
              "       attribution: 'TianDiTu'" // attribution
              "   }"
              ").addTo(map);"
              "</script>"
              "</body>"
              "</html>")
          .arg(key)
          .arg(lat)
          .arg(lon)
          .arg(zoom);

  // mpWebView->setHtml(htmlContent);
  logMessage("set TianDiTu map", Qgis::MessageLevel::Success);
}
*/


void MainWindow::createCanvas() {
  mpOpenGLWidget = std::make_unique<OpenGLCanvas>(this);
  // ================= middle area =================
  mpStackedWidget = new QStackedWidget(this);
  setCentralWidget(mpStackedWidget);
  mpStackedWidget->setObjectName("mpStackedWidget");
  mpStackedWidget->addWidget(mpOpenGLWidget.get()); // add 3D view
  mpStackedWidget->addWidget(mpImageLabel);         // add 2D view
  mpStackedWidget->setCurrentWidget(
      mpOpenGLWidget.get()); // set 3D view as default

  logMessage("create stacked widget", Qgis::MessageLevel::Info);
}