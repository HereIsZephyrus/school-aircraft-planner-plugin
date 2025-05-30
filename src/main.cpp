#include "MainWindow.h"
#include "qgsapplication.h"
#ifdef Q_OS_WIN
#define QGIS_PATH "D:/OSGEO4~1/apps/qgis"
#elif defined(Q_OS_MACOS)
#define QGIS_PATH "/Applications/QGIS.app/Contents/MacOS/QGIS"
#else
#define QGIS_PATH "/usr/bin/qgis"
#endif

int main(int argc, char *argv[]) {
  QgsApplication app(argc, argv, true);

  QgsApplication::setPrefixPath(QGIS_PATH, true);
  QgsApplication::initQgis();

  //QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
  //QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

  MainWindow& w = MainWindow::getInstance();
  w.show();

  int exitCode = app.exec();

  QgsApplication::exitQgis();

  return exitCode;
}
