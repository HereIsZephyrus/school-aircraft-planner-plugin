#ifndef CANVAS_H
#define CANVAS_H
#include "../core/RoutePlanner.h"
#include "OpenGLCanvas.h"
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <memory>
#include <qobjectdefs.h>
#include <qgsmapcanvas.h>

#include "../gui/LayerTreeWidget.h"

class Canvas : public QStackedWidget {
  Q_OBJECT

public:
  Canvas(QWidget *parent = nullptr);
  ~Canvas();
  OpenGLCanvas *getOpenGLWidget() const { return mpOpenGLWidget; }
  QgsMapCanvas* getMapCanvas() const { return mpMapCanvas; }

public slots:
  void switchTo2D();
  void switchTo3D();
  void viewReset();
  void refreshQgsMapCanvas();

private:
  OpenGLCanvas *mpOpenGLWidget;
  QgsMapCanvas* mpMapCanvas;

  QPushButton *mpBtnReset;
  QPushButton *mpBtnSwitchTo3D;
  QPushButton *mpBtnSwitchTo2D;

  void init3DWidget();
  void init2DWidget();

signals:
  void refreashParms();
};

#endif // CANVAS_H
