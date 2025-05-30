#include "mainwindow.h"
#include "QgsApplication.h"

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QApplication app(argc, argv, true); // 第三个参数 `GUIenabled = true`

    QgsApplication::setPrefixPath("D:/OSGEO4~1/apps/qgis", true);
    //QgsApplication::initQgis();


    MainWindow w;
    w.show();

    int exitCode = app.exec();

    //QgsApplication::exitQgis();

    return exitCode;
}
