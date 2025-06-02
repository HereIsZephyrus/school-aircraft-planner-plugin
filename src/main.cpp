#include "MainWindow.h"
#include "opengl/Camera.h"
#include "qgsapplication.h"
#include "log/QgisDebug.h"
#ifdef Q_OS_WIN
#define QGIS_PATH "D:/OSGEO4~1/apps/qgis"
#elif defined(Q_OS_MACOS)
#define QGIS_PATH "/Applications/QGIS.app/Contents/MacOS/QGIS"
#else
#define QGIS_PATH "/usr/bin/qgis"
#endif

int main(int argc, char *argv[]) {
  logMessage("Starting application...", Qgis::MessageLevel::Info);
  
  QgsApplication app(argc, argv, true);
  logMessage("QgsApplication created", Qgis::MessageLevel::Info);

  QgsApplication::setPrefixPath(QGIS_PATH, true);
  logMessage("QGIS prefix path set", Qgis::MessageLevel::Info);
  
  QgsApplication::initQgis();
  logMessage("QGIS initialized", Qgis::MessageLevel::Info);

  MainWindow& w = MainWindow::getInstance();
  logMessage("MainWindow instance created", Qgis::MessageLevel::Info);
  
  w.show();
  logMessage("MainWindow shown", Qgis::MessageLevel::Info);

  int exitCode = app.exec();
  logMessage("Application event loop ended", Qgis::MessageLevel::Info);

  w.close();
  logMessage("MainWindow closed", Qgis::MessageLevel::Info);

  w.release();

  QgsApplication::exitQgis();
  logMessage("QGIS exited", Qgis::MessageLevel::Info);

  return exitCode;
}
