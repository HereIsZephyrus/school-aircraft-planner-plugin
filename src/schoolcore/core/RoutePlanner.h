/****************************************************************************
File:RoutePlanner.h
Author:wkj
Date:2025.3.13
****************************************************************************/
#pragma once

#include <QLineF>
#include <QObject>
#include <QPoint>
#include <QPolygonF>
#include <QTransform>
#include <QVector3D>
#include <QVector>
#include <QMouseEvent>
#include <qtransform.h>
#include <cfloat>

enum FlightPattern { SCAN_LINES, SPIRAL };
class RoutePlanner : public QObject {
  Q_OBJECT
public:
  void setFlightPattern(FlightPattern pattern);
  void setTurnRadius(float radius);

private:
  // check if the point is in the convex hull
  bool pointInConvexHull(const QVector3D &point);
  // check if the line is in the convex hull
  bool lineInConvexHull(const QLineF &line);
  FlightPattern m_pattern = SCAN_LINES;
  float m_scanSpacing = 10.0f; // scan line spacing
  float m_turnRadius = 5.0f;   // turn radius
  QVector<QVector3D> generateScanLines();
  QVector<QVector3D> generateSpiral();
  QVector<QVector3D> optimizePath(const QVector<QVector3D> &path);

public:
  explicit RoutePlanner(QObject *parent = nullptr);

  // calculate convex hull
  QVector<QVector3D> calculateConvexHull();
  // generate flight path
  void generateFlightPath();
  QVector<QVector3D> generateScanLines(float spacing);
  void dataUpdated(){};
  // control points
  const QVector<QVector3D> &controlPoints() const { return m_controlPoints; }
  // convex hull
  const QVector<QVector3D> &convexHull() const { return m_convexHull; }
  // route path
  const QVector<QVector3D> &routePath() const { return m_flightPath; }
  bool m_editingMode = false;          // edit mode
  bool mCreateRoute = false;           // create route mode
  bool m_isAddingControlPoint = false; // add control point mode
  bool isAddingControlPoint() const {
    return m_isAddingControlPoint;
  } // add control point mode
  int selectedPointIndex() const { return mnSelectedPoint; }
  void setSelectedPoint(int index);
  void removeSelectedPoint();
  void exitAddingMode() { m_editingMode = false; }
  QVector<QVector3D> m_controlPoints; // control points
  void setHomePoint(const QVector3D &point);
  QVector3D homePoint() const;
  void setSettingHomePointMode(bool enabled);
  bool isSettingHomePointMode() const;
  bool m_settingHomePointMode = false;
  float calculateOptimalRotation(const QPolygonF &hull);
  QVector<QPointF> calculateIntersections(const QLineF &scanLine,
                                          const QPolygonF &rotatedHull);

private:
  QVector3D m_homePoint;
public slots:
  void enterRoutePlanningMode();
  void addControlPoint(const QVector3D &point);
  void setEditingMode(bool enabled);
  void handleMouseMove(QMouseEvent *event);
  bool isEditing();
  void setScanSpacing(double spacing);
signals:
  void requestRedraw();

private:
  QVector<QVector3D> m_convexHull; // convex hull
  QVector<QVector3D> m_flightPath; // flight path
  QVector<QVector3D> m_edges;      // edges of convex hull
  double mdCurrentHeight = 50.0;   // current height
  int mnSelectedPoint = -1;        // current selected point
  // screen to world
  QVector3D screenToWorld(const QPoint &screenPos);

  void updateConvexHull(); // update convex hull
};
