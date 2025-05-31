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
  Canvas(QStackedWidget *parent = nullptr);
  ~Canvas();
  void init3DWidget();
  void init2DWidget();
  void resetView();

private slots:
  void switchTo2D();
  void switchTo3D();
  void loadModel(const QString &objFilePath);

private:
  QLabel *mpImageLabel;
  OpenGLCanvas *mpOpenGLWidget;
  std::unique_ptr<RoutePlanner> mpRoutePlanner;

  QPushButton *mpBtnReset;
  QPushButton *mpBtnSwitchTo3D;
  QPushButton *mpBtnSwitchTo2D;
  QPushButton *mpBtnManualMode;
  QPushButton *mpBtnAutoMode;
  // void setTianDiTuMap(double lat, double lon, int zoom);
};

#endif // CANVAS_H
