#ifndef TRIANGULATION_HANDLER_H
#define TRIANGULATION_HANDLER_H

#include <QVector>
#include <QVector3D>
#include <QPolygonF>
#include <QRectF>
#include <qgsgeometry.h>
#include <qgspoint.h>
#include <qgswkbtypes.h>
#include <qgsfeature.h>

class QgsTriangulation;

struct Triangle {
    QVector3D p1, p2, p3;  
    QVector3D center;      
    
    Triangle() = default;
    Triangle(const QVector3D& point1, const QVector3D& point2, const QVector3D& point3)
        : p1(point1), p2(point2), p3(point3) {
        center = (p1 + p2 + p3) / 3.0f;
    }
};


class TriangulationHandler {
public:
    TriangulationHandler();
    ~TriangulationHandler();


    QVector<QVector3D> generateUniformPoints(const QRectF& bounds, 
                                             double spacing,
                                             const QVector<QPolygonF>& flightZonePolygons);


    bool performDelaunayTriangulation(const QVector<QVector3D>& points);


    QVector<Triangle> clipTriangulationWithFlightZone(const QVector<QPolygonF>& flightZonePolygons);


    const QVector<Triangle>& getTriangles() const { return mTriangles; }


    const QVector<Triangle>& getClippedTriangles() const { return mClippedTriangles; }


    QVector<QPair<QVector3D, QVector3D>> getTriangulationEdges() const;


    static bool triangleIntersectsPolygon(const Triangle& triangle, const QPolygonF& polygon);


    static bool triangleInPolygon(const Triangle& triangle, const QPolygonF& polygon);


private:
    QVector<QVector3D> mPoints;           // 输入点集
    QVector<Triangle> mTriangles;         // 原始三角形集合
    QVector<QVector3D> mPoints;           
    QVector<Triangle> mTriangles;         
    QVector<Triangle> mClippedTriangles;  
   
    bool isPointInAnyPolygon(const QVector3D& point, const QVector<QPolygonF>& polygons);

    
    void performSimpleTriangulation(const QVector<QVector3D>& points);
};

#endif // TRIANGULATION_HANDLER_H 