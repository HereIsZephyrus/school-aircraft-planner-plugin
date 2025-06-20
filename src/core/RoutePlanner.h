
#ifndef ROUTE_PLANNER_H
#define ROUTE_PLANNER_H
#include <QLineF>
#include <QMouseEvent>
#include <QObject>
#include <QPoint>
#include <QPolygonF>
#include <QTransform>
#include <QVector3D>
#include <QVector>
#include <cfloat>
#include <memory>
#include <qtransform.h>
#include <qvector3d.h>
#include "../opengl/Primitive.h"

enum class FlightPattern : unsigned char {
  SCANLINE,
  SPIRAL,
  TOUR,
};

enum class RouteDrawMode : unsigned char {
  AVAILABLE,
  ADDDING_CONTROL_POINTS,
  CREATING_CONTROL_POINTS,
  CREATING_CONVEX_HULL,
  CREATING_ROUTE_PATH,
  CREATING_HOME_POINT,
  EDITING_ROUTE_PATH,
  EDITING_CONTROL_POINTS,
  EDITING_CONVEX_HULL,
  EDITING_HOME_POINT,
  PREVIEWING_ROUTE,
};

class RoutePlanner;

class Route {
  friend class RoutePlanner;
public:
  explicit Route(FlightPattern pattern, float turnRadius, float scanSpacing,
                 std::shared_ptr<gl::ControlPoints> controlPoints,
                 std::shared_ptr<gl::ConvexHull> convexHull,
                 std::shared_ptr<gl::RoutePath> path,
                 std::shared_ptr<gl::SinglePoint> homePoint);
  ~Route() = default;

  void draw(const QMatrix4x4 &view, const QMatrix4x4 &projection);

private:
  FlightPattern mPattern;
  float mTurnRadius;
  float mScanSpacing;
  std::shared_ptr<gl::RoutePath> path;
  std::shared_ptr<gl::SinglePoint> homePoint;
  std::shared_ptr<gl::ControlPoints> controlPoints;
  std::shared_ptr<gl::ConvexHull> convexHull;
};

class RoutePlanner : public QObject {
  Q_OBJECT
  
private:
  RoutePlanner();

public:
  ~RoutePlanner() {
    mRoutes.clear();
  }
  static RoutePlanner &getInstance() {
    static RoutePlanner instance;
    return instance;
  }
  RoutePlanner(const RoutePlanner &) = delete;
  RoutePlanner &operator=(const RoutePlanner &) = delete;
  FlightPattern flightPattern() const { return mPattern; }
  RouteDrawMode drawMode() const { return mDrawMode; }
  float turnRadius() const { return mTurnRadius; }
  float scanSpacing() const { return mScanSpacing; }
  void setFlightPattern(FlightPattern pattern) { mPattern = pattern; }
  void setTurnRadius(float radius) { mTurnRadius = radius; }
  void setScanSpacing(float spacing) { mScanSpacing = spacing; }
  RouteDrawMode getDrawMode() const { return mDrawMode; }
  void setDrawMode(RouteDrawMode mode) { mDrawMode = mode; }
  QVector3D getHomePoint() const { return mRoutes[mRouteIndex]->homePoint->getPoint();}
  const QVector<QVector3D>& getRoutePath() {return mRoutes[mRouteIndex]->path->getRoutePath();}

  std::shared_ptr<gl::ControlPoints> constructDrawingPoints();
  void drawRoutes(const QMatrix4x4 &view, const QMatrix4x4 &projection);
  void cleanRoutes();
  void setContext(QOpenGLContext* context);

public slots:
  void createRoute();
  void createControlPoint();
  void editRoute();
  void addControlPoint(QVector3D point);

private:
  QVector<std::shared_ptr<Route>> mRoutes;
  QVector<QVector3D> drawingPoint;
  FlightPattern mPattern;
  RouteDrawMode mDrawMode;
  QOpenGLContext* context;
  float mScanSpacing; // scan line spacing
  float mTurnRadius;  // turn radius
  int mRouteIndex;
  QVector3D getControlPoint();

  void generateRoutePath(const QVector<QVector3D> &controlPointsLocation,
                         const QVector3D &homePointLocation,
                         const QVector<QVector3D> &convexHullLocation,
                         FlightPattern pattern,
                         QVector<QVector3D> &routePathLocation);
                         
  void generateScanLinePath(const QVector3D &homePointLocation,
                            const QVector<QVector3D> &convexHullLocation,
                            QVector<QVector3D> &routePathLocation);

  void generateSpiralPath(const QVector3D &homePointLocation,
                          const QVector<QVector3D> &convexHullLocation,
                          QVector<QVector3D> &routePathLocation);

  void generateTourPath(const QVector<QVector3D> &controlPointsLocation,
                        const QVector3D &homePointLocation,
                        const QVector<QVector3D> &convexHullLocation,
                        QVector<QVector3D> &routePathLocation);

  void generateControlPoints(QVector<QVector3D> &controlPointsLocation);

  void generateConvexHull(QVector<QVector3D> &controlPointsLocation,
                          QVector<QVector3D> &convexHullLocation);

  QVector3D generateHomePoint();

  static bool pointInConvexHull(const QVector3D &point,
                                const QVector<QVector3D> &convexHull);

  static bool lineInConvexHull(const QLineF &line,
                               const QVector<QVector3D> &convexHull);

  QVector<QPointF> calculateIntersections(const QLineF &scanLine, const QPolygonF &rotatedHull);

  QVector<QVector3D> optimizePath(const QVector<QVector3D> &path);

  float calculateOptimalRotation(const QPolygonF &hull);
};
#endif