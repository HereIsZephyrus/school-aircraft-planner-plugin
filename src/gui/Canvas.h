#ifndef CANVAS_H
#define CANVAS_H
#include "../core/RoutePlanner.h"
#include "OpenGLCanvas.h"
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <memory>
class Canvas : public QStackedWidget {
  Q_OBJECT

public:
  Canvas(QWidget *parent = nullptr);
  ~Canvas();

public slots:
  void loadModel(const QString &objFilePath);
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
};

#endif // CANVAS_H
