#ifndef CANVAS_H
#define CANVAS_H
#include <QStackedWidget>
#include <QLabel>
#include <memory>
#include "../core/RoutePlanner.h"
#include "OpenGLCanvas.h"

class Canvas : public QStackedWidget{
    Q_OBJECT

public:
    Canvas(QStackedWidget *parent = nullptr);
    ~Canvas();
    void init3DWidget();
    void init2DWidget();
    void resetView();

public slots:
    void switchTo2D();
    void switchTo3D();

private:
    QLabel *mpImageLabel;
    OpenGLCanvas *mpOpenGLWidget;
    std::unique_ptr<RoutePlanner> mpRoutePlanner;
};

#endif // CANVAS_H
