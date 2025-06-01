#include "RoutePlanner.h"
#include "../core/WorkspaceState.h"
#include <QPolygonF>
#include <QStack>
#include <QVector3D>
#include <QtCore>
#include <algorithm>
#include <qdebug.h>
#include <QtGui/QTransform>

RoutePlanner::RoutePlanner()
    : mPattern(FlightPattern::SCANLINE), mScanSpacing(10.0f),
      mTurnRadius(5.0f) {}

Route::Route(FlightPattern pattern, float turnRadius, float scanSpacing,
             std::shared_ptr<gl::ControlPoints> controlPoints,
             std::shared_ptr<gl::ConvexHull> convexHull,
             std::shared_ptr<gl::RoutePath> path,
             std::shared_ptr<gl::HomePoint> homePoint) {
  mPattern = pattern;
  mTurnRadius = turnRadius;
  mScanSpacing = scanSpacing;
  this->controlPoints = controlPoints;
  this->convexHull = convexHull;
  this->path = path;
  this->homePoint = homePoint;
}

bool RoutePlanner::pointInConvexHull(const QVector3D &point,
                                     const QVector<QVector3D> &convexHull) {
  QPolygonF hull;
  for (const auto &p : convexHull)
    hull << QPointF(p.x(), p.y());

  return hull.containsPoint(QPointF(point.x(), point.y()), Qt::OddEvenFill);
}

bool RoutePlanner::lineInConvexHull(const QLineF &line,
                                    const QVector<QVector3D> &convexHull) {
  QPolygonF hull;
  for (const auto &p : convexHull)
    hull << QPointF(p.x(), p.y());

  // check if the line is in the convex hull
  if (!hull.containsPoint(line.p1(), Qt::OddEvenFill) ||
      !hull.containsPoint(line.p2(), Qt::OddEvenFill)) {
    return false;
  }

  // check if the line intersects with the convex hull
  for (int i = 0; i < hull.size(); ++i) {
    QLineF edge(hull[i], hull[(i + 1) % hull.size()]);
    QPointF intersect;
    if (line.intersects(edge, &intersect) == QLineF::BoundedIntersection) {
      return false; // the line intersects with the convex hull
    }
  }
  return true;
}

QVector3D RoutePlanner::getControlPoint() {
  return QVector3D(0, 0, 0);
}

void RoutePlanner::generateControlPoints(
    QVector<QVector3D> &controlPointsLocation) {
  mDrawMode = RouteDrawMode::CREATING_CONTROL_POINTS;
  while (mDrawMode == RouteDrawMode::CREATING_CONTROL_POINTS) {
    controlPointsLocation.append(getControlPoint());
  }
}

void RoutePlanner::generateConvexHull(QVector<QVector3D> &controlPointsLocation,
                                      QVector<QVector3D> &convexHullLocation) {
  mDrawMode = RouteDrawMode::CREATING_CONVEX_HULL;

  // use Graham scan to calculate convex hull
  if (controlPointsLocation.size() < 3)
    return;

  QVector<QVector3D> sortedPoints = controlPointsLocation;
  std::sort(sortedPoints.begin(), sortedPoints.end(),
            [](const QVector3D &a, const QVector3D &b) {
              return a.x() < b.x() || (a.x() == b.x() && a.y() < b.y());
            });

  QStack<QVector3D> lower;
  for (const QVector3D &p : sortedPoints) {
    while (lower.size() >= 2 &&
           QVector3D::crossProduct(lower.top() - lower.at(lower.size() - 2),
                                   p - lower.top())
                   .z() <= 0)
      lower.pop();
    lower.push(p);
  }

  QStack<QVector3D> upper;
  for (int i = sortedPoints.size() - 1; i >= 0; --i) {
    while (upper.size() >= 2 &&
           QVector3D::crossProduct(upper.top() - upper.at(upper.size() - 2),
                                   sortedPoints[i] - upper.top())
                   .z() <= 0)
      upper.pop();
    upper.push(sortedPoints[i]);
  }

  upper.pop(); // remove duplicate
  lower.pop(); // remove duplicate

  while (!lower.isEmpty())
    convexHullLocation.append(lower.pop());
  while (!upper.isEmpty())
    convexHullLocation.append(upper.pop());
}

void RoutePlanner::generateRoutePath(const QVector<QVector3D> &controlPointsLocation,
                                     const QVector3D &homePointLocation,
                                     const QVector<QVector3D> &convexHullLocation,
                                     FlightPattern pattern,
                                     QVector<QVector3D> &routePathLocation) {
  mDrawMode = RouteDrawMode::CREATING_ROUTE_PATH;
  switch (pattern) {
  case FlightPattern::SCANLINE:
    generateScanLinePath(homePointLocation, convexHullLocation,
                         routePathLocation);
    break;
  case FlightPattern::SPIRAL:
    generateSpiralPath(homePointLocation, convexHullLocation,
                       routePathLocation);
    break;
  case FlightPattern::TOUR:
    generateTourPath(controlPointsLocation, homePointLocation,
                     convexHullLocation, routePathLocation);
    break;
  }
}

QVector3D RoutePlanner::generateHomePoint() {
  mDrawMode = RouteDrawMode::CREATING_HOME_POINT;
  while (mDrawMode == RouteDrawMode::CREATING_HOME_POINT) {
    return getControlPoint();
  }
  return QVector3D(0, 0, 0);
}

void RoutePlanner::createRoute() {
  QVector<QVector3D> controlPointsLocation;
  generateControlPoints(controlPointsLocation);
  std::shared_ptr<gl::ControlPoints> controlPoints =
      std::make_shared<gl::ControlPoints>(controlPointsLocation);

  QVector3D homePointLocation;
  homePointLocation = generateHomePoint();
  std::shared_ptr<gl::HomePoint> homePoint =
      std::make_shared<gl::HomePoint>(QVector<QVector3D>{homePointLocation});

  QVector<QVector3D> convexHullLocation;
  generateConvexHull(controlPointsLocation, convexHullLocation);
  std::shared_ptr<gl::ConvexHull> convexHull =
      std::make_shared<gl::ConvexHull>(convexHullLocation);

  QVector<QVector3D> routePathLocation;
  generateRoutePath(controlPointsLocation, homePointLocation,
                    convexHullLocation, mPattern, routePathLocation);

  std::shared_ptr<gl::RoutePath> routePath =
      std::make_shared<gl::RoutePath>(routePathLocation);
  mRoutes.append(std::make_shared<Route>(mPattern, mTurnRadius, mScanSpacing,
                                         controlPoints, convexHull, routePath,
                                         homePoint));
  mDrawMode = RouteDrawMode::AVAILABLE;
}

void RoutePlanner::generateScanLinePath(
    const QVector3D &homePointLocation,
    const QVector<QVector3D> &convexHullLocation,
    QVector<QVector3D> &routePathLocation) {

  float currentHeight = ws::FlightManager::getInstance().getCurrentHeight();
  if (convexHullLocation.size() < 3)
    return;

  QPolygonF hullPoly;
  for (const auto &p : convexHullLocation)
    hullPoly << QPointF(p.x(), p.y());

  // 1. translate convex hull to origin
  QPointF center = hullPoly.boundingRect().center();
  QTransform translateToOrigin =
      QTransform::fromTranslate(-center.x(), -center.y());
  QTransform rotationTransform;
  float rotationAngle = calculateOptimalRotation(hullPoly);
  rotationTransform.rotate(-rotationAngle);

  // 2. rotate and translate convex hull to origin
  QTransform fullTransform =
      translateToOrigin * rotationTransform * translateToOrigin.inverted();

  QPolygonF rotatedHull = fullTransform.map(hullPoly);
  QRectF rotatedBBox = rotatedHull.boundingRect();

  float effectiveSpacing = mScanSpacing;
  float currentY = rotatedBBox.top();
  bool reverseDirection = false;
  QVector3D lastEndPoint;

  while (currentY <= rotatedBBox.bottom()) {
    QLineF scanLine(rotatedBBox.left() - 10, currentY, rotatedBBox.right() + 10,
                    currentY);

    QVector<QPointF> intersects = calculateIntersections(scanLine, rotatedHull);
    if (intersects.size() >= 2) {
      if (reverseDirection)
        std::reverse(intersects.begin(), intersects.end());

      // 3. calculate the start and end point of the scan line
      QTransform inverse = fullTransform.inverted();
      QPointF start2D = inverse.map(intersects.first());
      QPointF end2D = inverse.map(intersects.last());

      QVector3D start(start2D.x(), start2D.y(), currentHeight); // start point
      QVector3D end(end2D.x(), end2D.y(), currentHeight);
      if (!routePathLocation.isEmpty()) {
        // add the last end point to the path
        routePathLocation.append(lastEndPoint);
        routePathLocation.append(start);
      }

      // add the start and end point to the path
      routePathLocation.append(start);
      routePathLocation.append(end);
      lastEndPoint = end;

      reverseDirection = !reverseDirection;
    }

    currentY += effectiveSpacing; // update the current y
  }
}

void RoutePlanner::generateSpiralPath(
    const QVector3D &homePointLocation,
    const QVector<QVector3D> &convexHullLocation,
    QVector<QVector3D> &routePathLocation) {
  float currentHeight = ws::FlightManager::getInstance().getCurrentHeight();
  if (convexHullLocation.size() < 3)
    return;

  // calculate the centroid of the convex hull
  QVector3D centroid(0, 0, 0);
  for (const QVector3D &p : convexHullLocation)
    centroid += p;
  centroid /= convexHullLocation.size();
  centroid.setZ(currentHeight); // set the z value to the current height

  // calculate the step and angle step
  const float step = mScanSpacing * 0.8f; // step
  const float angleStep = 0.1f;           // angle step
  float radius = 0.0f;
  float theta = 0.0f;

  // calculate the spiral path
  QVector3D lastValidPoint = centroid;
  while (radius < 1000.0f) { // stop when the radius is greater than 1000
    radius += step * angleStep / (2 * M_PI);
    theta += angleStep;

    // calculate the current point
    float x = centroid.x() + radius * cos(theta);
    float y = centroid.y() + radius * sin(theta);
    QVector3D currentPoint(x, y, currentHeight);

    // check if the path segment is in the convex hull
    QLineF pathSegment(lastValidPoint.toPointF(), currentPoint.toPointF());
    if (lineInConvexHull(pathSegment, convexHullLocation)) {
      routePathLocation.append(currentPoint);
      lastValidPoint = currentPoint;
    } else {
      // update the radius
      radius -= step * 0.5f;
    }

    // stop when the radius is greater than the maximum distance
    float maxDistance = 0.0f;
    for (const QVector3D &p : convexHullLocation)
      maxDistance = qMax(maxDistance, p.distanceToPoint(centroid));
    if (radius >
        maxDistance *
            2) // stop when the radius is greater than the maximum distance
      break;
  }
}

void RoutePlanner::generateTourPath(
    const QVector<QVector3D> &controlPointsLocation,
    const QVector3D &homePointLocation,
    const QVector<QVector3D> &convexHullLocation,
    QVector<QVector3D> &routePathLocation) {
  if (convexHullLocation.size() < 3)
    return;
}

float RoutePlanner::calculateOptimalRotation(const QPolygonF &hull) {
  float minArea = FLT_MAX;
  float optimalAngle = 0.0f;

  for (float angle = 0; angle < 180; angle += 5) {
    QTransform t;
    t.rotate(angle);
    QRectF rect = t.map(hull).boundingRect();
    float area = rect.width() * rect.height();
    if (area < minArea) {
      minArea = area;
      optimalAngle = angle;
    }
  }
  return optimalAngle;
}

QVector<QPointF> RoutePlanner::calculateIntersections(const QLineF &scanLine, const QPolygonF &rotatedHull) {
  QVector<QPointF> intersects;
  for (int i = 0; i < rotatedHull.size(); ++i) {
    QLineF edge(rotatedHull[i], rotatedHull[(i + 1) % rotatedHull.size()]);
    QPointF intersect;
    if (scanLine.intersects(edge, &intersect) == QLineF::BoundedIntersection)
      intersects.append(intersect);
  }
  std::sort(intersects.begin(), intersects.end(),
            [](const QPointF &a, const QPointF &b) { return a.x() < b.x(); });
  return intersects;
}

QVector<QVector3D> RoutePlanner::optimizePath(const QVector<QVector3D> &path) {
  return path;
}