/****************************************************************************
File:RoutePlanner.h
Author:wkj
Date:2025.3.13
****************************************************************************/
#ifndef ROUTE_PLANNER_H
#define ROUTE_PLANNER_H
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
#include <memory>

/*
class RoutePlanner : public QObject {
  Q_OBJECT
public:
  

private:
  
  QVector<QVector3D> generateScanLines();
  QVector<QVector3D> generateSpiral();
  QVector<QVector3D> optimizePath(const QVector<QVector3D> &path);

signals:
  void dataUpdated();

public:
  explicit RoutePlanner(QObject *parent = nullptr);

  // generate flight path
  void generateFlightPath();
  QVector<QVector3D> generateScanLines(float spacing);
  // control points
  const QVector<QVector3D> &controlPoints() const { return m_controlPoints; }
  // convex hull
  const QVector<QVector3D> &convexHull() const { return m_convexHull; }
  // route path
  const QVector<QVector3D> &routePath() const { return m_flightPath; }
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
*/

namespace gl{
class RoutePath;
class HomePoint;
class ControlPoints;
class ConvexHull;
};

enum class FlightPattern : unsigned char{
  SCANLINE,
  SPIRAL,
  TOUR,
};

enum class RouteDrawMode : unsigned char{
  AVAILABLE,
  CREATING_CONTROL_POINTS,
  CREATING_CONVEX_HULL,
  CREATING_ROUTE_PATH,
  CREATING_HOME_POINT,
  EDITING_ROUTE_PATH,
  EDITING_CONTROL_POINTS,
  EDITING_CONVEX_HULL,
  EDITING_HOME_POINT,
};

class Route{
public:
  explicit Route(FlightPattern pattern, float turnRadius, float scanSpacing, std::shared_ptr<gl::ControlPoints> controlPoints, std::shared_ptr<gl::ConvexHull> convexHull, std::shared_ptr<gl::RoutePath> path, std::shared_ptr<gl::HomePoint> homePoint);
  ~Route() = default;
  
private:
  FlightPattern mPattern;
  float mTurnRadius;
  float mScanSpacing;
  std::shared_ptr<gl::RoutePath> path;
  std::shared_ptr<gl::HomePoint> homePoint;
  std::shared_ptr<gl::ControlPoints> controlPoints;
  std::shared_ptr<gl::ConvexHull> convexHull;
};

class RoutePlanner{
private:
  RoutePlanner();
public:
  ~RoutePlanner(){
    for (auto &route : mRoutes) {
      route->path.reset();
      route->controlPoints.reset();
      route->convexHull.reset();
    }
  }
  static RoutePlanner& getInstance() {
    static RoutePlanner instance;
    return instance;
  }
  RoutePlanner(const RoutePlanner&) = delete;
  RoutePlanner& operator=(const RoutePlanner&) = delete;
  FlightPattern flightPattern() const {return mPattern;}
  RouteDrawMode drawMode() const {return mDrawMode;}
  float turnRadius() const {return mTurnRadius;}
  float scanSpacing() const {return mScanSpacing;}
  void setFlightPattern(FlightPattern pattern){mPattern = pattern;}
  void setTurnRadius(float radius){mTurnRadius = radius;}
  void setScanSpacing(float spacing){mScanSpacing = spacing;}
  void setDrawMode(RouteDrawMode mode){mDrawMode = mode;}

private slots:
  void createRoute();
  void editRoute();

private:
  QVector<std::shared_ptr<Route>> mRoutes;
  FlightPattern mPattern;
  RouteDrawMode mDrawMode;
  float mScanSpacing; // scan line spacing
  float mTurnRadius;   // turn radius
  void addControlPoint(const QVector3D &point);
  void generateScanLinePath(const QVector3D &homePointLocation, const QVector<QVector3D> &convexHullLocation, QVector<QVector3D> &routePathLocation);
  void generateSpiralPath(const QVector3D &homePointLocation, const QVector<QVector3D> &convexHullLocation, QVector<QVector3D> &routePathLocation);
  void generateTourPath(const QVector<QVector3D> &controlPointsLocation, const QVector3D &homePointLocation, const QVector<QVector3D> &convexHullLocation, QVector<QVector3D> &routePathLocation);
  void optimizePath(const QVector<QVector3D> &path);

  void generateControlPoints(QVector<QVector3D> &controlPointsLocation);
  void generateConvexHull(QVector<QVector3D> &controlPointsLocation, QVector<QVector3D> &convexHullLocation);
  void generateRoutePath(QVector<QVector3D> &controlPointsLocation, QVector<QVector3D> &convexHullLocation, FlightPattern pattern, QVector<QVector3D> &routePathLocation);
  QVector3D generateHomePoint();
  static bool pointInConvexHull(const QVector3D &point, const QVector<QVector3D> &convexHull);
  static bool lineInConvexHull(const QLineF &line, const QVector<QVector3D> &convexHull);
};
#endif