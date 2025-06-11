#ifndef CANVAS_H
#define CANVAS_H
#include "../core/RoutePlanner.h"
#include "OpenGLCanvas.h"
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <memory>
#include <qobjectdefs.h>
class Canvas : public QStackedWidget {
  Q_OBJECT

public:
  Canvas(QWidget *parent = nullptr);
  ~Canvas();
  OpenGLCanvas *getOpenGLWidget() const { return mpOpenGLWidget; }
  QLabel *getImageLabel() const { return mpImageLabel; }    

public slots:
  void switchTo2D();
  void switchTo3D();
  void viewReset();

private:
  QLabel *mpImageLabel;
  OpenGLCanvas *mpOpenGLWidget;

  QPushButton *mpBtnReset;
  QPushButton *mpBtnSwitchTo3D;
  QPushButton *mpBtnSwitchTo2D;

  void init3DWidget();
  void init2DWidget();
  // void setTianDiTuMap(double lat, double lon, int zoom);

signals:
  void refreashParms();
};

#endif // CANVAS_H
