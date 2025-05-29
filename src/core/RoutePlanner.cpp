#include "RoutePlanner.h"
#include <QPolygonF>
#include <QVector3D>
#include <QStack>
#include <algorithm>
#include<qdebug.h>
#include <QtCore>

#include <QtGui/QTransform>
RoutePlanner::RoutePlanner(QObject* parent)
    : QObject(parent)
{
}
// enter route planning mode
void RoutePlanner::enterRoutePlanningMode()
{
 
    m_controlPoints.clear();
    m_convexHull.clear();
    m_flightPath.clear();
    mCreateRoute = true;
    emit requestRedraw();
    qDebug() << "mCreateRoute = true";
}
// add control point
void RoutePlanner::addControlPoint(const QVector3D& point)
{
    if (mCreateRoute == true) {
        m_controlPoints.append(point);
        updateConvexHull();
        generateFlightPath();
        emit requestRedraw();
    }
}
// set editing mode
void RoutePlanner::setEditingMode(bool enabled)
{
    m_editingMode = enabled;
    emit requestRedraw();
}

// handle mouse move event
void RoutePlanner::handleMouseMove(QMouseEvent* event)
{
        updateConvexHull();
        generateFlightPath();
        emit requestRedraw();
 
}
// calculate convex hull
QVector<QVector3D> RoutePlanner::calculateConvexHull()
{
    // use Graham scan to calculate convex hull
    if (m_controlPoints.size() < 3)
        return {};

    QVector<QVector3D> sortedPoints = m_controlPoints;
    std::sort(sortedPoints.begin(), sortedPoints.end(), [](const QVector3D& a, const QVector3D& b) {
        return a.x() < b.x() || (a.x() == b.x() && a.y() < b.y());
        });

    QStack<QVector3D> lower;
    for (const QVector3D& p : sortedPoints) {
        while (lower.size() >= 2 && QVector3D::crossProduct(lower.top() - lower.at(lower.size() - 2), p - lower.top()).z() <= 0) {
            lower.pop();
        }
        lower.push(p);
    }

    QStack<QVector3D> upper;
    for (int i = sortedPoints.size() - 1; i >= 0; --i) {
        while (upper.size() >= 2 && QVector3D::crossProduct(upper.top() - upper.at(upper.size() - 2), sortedPoints[i] - upper.top()).z() <= 0) {
            upper.pop();
        }
        upper.push(sortedPoints[i]);
    }

    upper.pop(); // remove duplicate
    lower.pop(); // remove duplicate

    QVector<QVector3D> convexHull;
    while (!lower.isEmpty()) {
        convexHull.append(lower.pop());
    }
    while (!upper.isEmpty()) {
        convexHull.append(upper.pop());
    }

    return convexHull;
}

QVector<QVector3D> RoutePlanner::generateScanLines(float spacing)
{
    QVector<QVector3D> scanLines;
    if (m_convexHull.isEmpty())
        return scanLines;

    QPolygonF convexPolygon;
    for (const QVector3D& point : m_convexHull) {
        convexPolygon.append(QPointF(point.x(), point.y()));
    }

    QVector<QVector3D> scanPaths;
    float minX = convexPolygon.boundingRect().left();
    float maxX = convexPolygon.boundingRect().right();
    float currentX = minX;

    while (currentX <= maxX) {
        QVector<QVector3D> path;
        for (int i = 0; i < convexPolygon.size(); ++i) {
            if (convexPolygon[i].x() >= currentX && convexPolygon[i].x() <= currentX + spacing) {
                path.append(QVector3D(convexPolygon[i].x(), convexPolygon[i].y(), mdCurrentHeight));
            }
        }
        scanPaths.append(path);
        currentX += spacing;
    }
    for (auto& point : scanLines) {
        point.setZ(mdCurrentHeight);
    }
    return scanPaths;
}
// update convex hull
void RoutePlanner::updateConvexHull()
{
    m_convexHull = calculateConvexHull();
    generateFlightPath();
}
// screen to world
QVector3D RoutePlanner::screenToWorld(const QPoint& screenPos)
{
    // first rotate the screen to world
    return QVector3D(screenPos.x(), screenPos.y(), 0.0f);
}
// check if it is editing mode
bool RoutePlanner::isEditing() {
    if (m_editingMode)
        return true;
    else return false;
}

void RoutePlanner::setSelectedPoint(int index) {
    mnSelectedPoint = index;
    emit dataUpdated();
}

void RoutePlanner::removeSelectedPoint() {
    if (mnSelectedPoint >= 0 && mnSelectedPoint < m_controlPoints.size()) {
        m_controlPoints.remove(mnSelectedPoint);
        mnSelectedPoint = -1;
        updateConvexHull();
        generateFlightPath();
        emit dataUpdated();
    }
}

void RoutePlanner::setHomePoint(const QVector3D& point) {
    m_homePoint = point;
    emit dataUpdated();
}

QVector3D RoutePlanner::homePoint() const {
    return m_homePoint;
}

void RoutePlanner::setSettingHomePointMode(bool enabled) {
    m_settingHomePointMode = enabled;
}

bool RoutePlanner::isSettingHomePointMode() const {
    return m_settingHomePointMode;
}

void RoutePlanner::generateFlightPath() {
    m_flightPath.clear();

    if (m_convexHull.isEmpty() || m_homePoint.isNull()) return;

    // route planning mode generate flight path
    QVector<QVector3D> basePath;
    switch (m_pattern) {
    case SCAN_LINES: basePath = generateScanLines(); break;
    case SPIRAL:     basePath = generateSpiral();    break;
    }

    // optimize path
    QVector<QVector3D> optimized = optimizePath(basePath);

    // add home point
    if (!optimized.isEmpty()) {
        m_flightPath.append(m_homePoint);
        m_flightPath += optimized;
        m_flightPath.append(m_homePoint);
    }

}
// generate scan lines
QVector<QVector3D> RoutePlanner::generateScanLines() {
    QVector<QVector3D> path;
    if (m_controlPoints.size() < 3) return path;

    QPolygonF hullPoly;
    for (const auto& p : m_convexHull) hullPoly << QPointF(p.x(), p.y());

    // 1. 平移凸多边形到原点
    QPointF center = hullPoly.boundingRect().center();
    QTransform translateToOrigin = QTransform::fromTranslate(-center.x(), -center.y());
    QTransform rotationTransform;
    float rotationAngle = calculateOptimalRotation(hullPoly);
    rotationTransform.rotate(-rotationAngle);

    // ��ϱ任��ƽ��->��ת->ƽ�ƻ�ȥ
    QTransform fullTransform = translateToOrigin * rotationTransform * translateToOrigin.inverted();

    QPolygonF rotatedHull = fullTransform.map(hullPoly);
    QRectF rotatedBBox = rotatedHull.boundingRect();

    float effectiveSpacing = m_scanSpacing;
    float currentY = rotatedBBox.top();
    bool reverseDirection = false;
    QVector3D lastEndPoint;

    while (currentY <= rotatedBBox.bottom()) {
        QLineF scanLine(rotatedBBox.left() - 10, currentY,
            rotatedBBox.right() + 10, currentY);

        QVector<QPointF> intersects = calculateIntersections(scanLine, rotatedHull);
        if (intersects.size() >= 2) {
            if (reverseDirection) std::reverse(intersects.begin(), intersects.end());

            // 3. ��ȷת�����겢����Zֵ
            QTransform inverse = fullTransform.inverted();
            QPointF start2D = inverse.map(intersects.first());
            QPointF end2D = inverse.map(intersects.last());

            QVector3D start(start2D.x(), start2D.y(), mdCurrentHeight); // �����߶�
            QVector3D end(end2D.x(), end2D.y(), mdCurrentHeight);
        if (!path.isEmpty()) {
            // ����ת��·������һ���յ� -> ��ǰ���
            path.append(lastEndPoint);
            path.append(start);
        }

        // ����ɨ����·��
        path.append(start);
        path.append(end);
        lastEndPoint = end;

        reverseDirection = !reverseDirection;

        }

        currentY += effectiveSpacing; // ʹ�ò�����ļ��
    }
    return path;
}


float RoutePlanner::calculateOptimalRotation(const QPolygonF& hull) {
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

// ����ɨ������͹���Ľ���
QVector<QPointF> RoutePlanner::calculateIntersections(const QLineF& scanLine, const QPolygonF& rotatedHull) {
    QVector<QPointF> intersects;
    for (int i = 0; i < rotatedHull.size(); ++i) {
        QLineF edge(rotatedHull[i], rotatedHull[(i + 1) % rotatedHull.size()]);
        QPointF intersect;
        if (scanLine.intersects(edge, &intersect) == QLineF::BoundedIntersection)
            intersects.append(intersect);
    }
    std::sort(intersects.begin(), intersects.end(),
        [](const QPointF& a, const QPointF& b) { return a.x() < b.x(); });
    return intersects;
}

QVector<QVector3D> RoutePlanner::optimizePath(const QVector<QVector3D>& path) {
    return path; // ֱ�ӷ���ԭ·�����������κ��Ż�
}


bool RoutePlanner::lineInConvexHull(const QLineF& line) {
    QPolygonF hull;
    for (const auto& p : m_convexHull)
        hull << QPointF(p.x(), p.y());

    // ���ټ��˵�
    if (!hull.containsPoint(line.p1(), Qt::OddEvenFill) ||
        !hull.containsPoint(line.p2(), Qt::OddEvenFill)) {
        return false;
    }

    // ��ϸ����߶���͹���߽�Ľ���
    for (int i = 0; i < hull.size(); ++i) {
        QLineF edge(hull[i], hull[(i + 1) % hull.size()]);
        QPointF intersect;
        if (line.intersects(edge, &intersect) == QLineF::BoundedIntersection) {
            return false; // ���ֱ߽罻��˵���߶δ���
        }
    }
    return true;
}
// �Ľ������������㷨
QVector<QVector3D> RoutePlanner::generateSpiral() {
    QVector<QVector3D> spiralPath;
    if (m_convexHull.size() < 3) return spiralPath;

    // ����͹������
    QVector3D centroid(0, 0, 0);
    for (const QVector3D& p : m_convexHull) centroid += p;
    centroid /= m_convexHull.size();
    centroid.setZ(mdCurrentHeight);

    // ��������
    const float step = m_scanSpacing * 0.8f; // ���������ܶ�
    const float angleStep = 0.1f;            // �Ƕ�����
    float radius = 0.0f;
    float theta = 0.0f;

    // ����������
    QVector3D lastValidPoint = centroid;
    while (radius < 1000.0f) { // ��ȫ��ֹ����
        radius += step * angleStep / (2 * M_PI);
        theta += angleStep;

        // ��������������
        float x = centroid.x() + radius * cos(theta);
        float y = centroid.y() + radius * sin(theta);
        QVector3D currentPoint(x, y, mdCurrentHeight);

        // ����߶��Ƿ���͹����
        QLineF pathSegment(lastValidPoint.toPointF(), currentPoint.toPointF());
        if (lineInConvexHull(pathSegment)) {
            spiralPath.append(currentPoint);
            lastValidPoint = currentPoint;
        }
        else {
            // ����Ӧ�����뾶
            radius -= step * 0.5f;
        }

        // ��ֹ�������뾶��������������
        float maxDistance = 0.0f;
        for (const QVector3D& p : m_convexHull) {
            maxDistance = qMax(maxDistance, p.distanceToPoint(centroid));
        }
        if (radius > maxDistance * 2) break;
    }

    return spiralPath;
}
// �����������жϵ��Ƿ���͹����
bool RoutePlanner::pointInConvexHull(const QVector3D& point) {
    QPolygonF hull;
    for (const auto& p : m_convexHull)
        hull << QPointF(p.x(), p.y());

    return hull.containsPoint(QPointF(point.x(), point.y()), Qt::OddEvenFill);
}

void RoutePlanner::setScanSpacing(double spacing) {
    if (m_scanSpacing != spacing && spacing > 0) {
        m_scanSpacing = spacing;
        generateFlightPath(); // ��������·��
       qDebug() << "m_scanSpacing ="<< m_scanSpacing;
        emit requestRedraw();
    }
}