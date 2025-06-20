#ifndef SHAPEFILE_HANDLER_H
#define SHAPEFILE_HANDLER_H

#include <QVector>
#include <QVector3D>
#include <QPolygonF>
#include <QString>
#include <QPointF>


#include <qgsgeometry.h>
#include <qgspoint.h>
#include <qgspolygon.h>
#include <qgsvectorlayer.h>
#include <qgsfeature.h>
#include <qgsfeatureiterator.h>
#include <qgswkbtypes.h>
#include <qgsabstractgeometry.h>
#include <qgslinestring.h>


class ShapefileHandler {
public:
    ShapefileHandler();
    ~ShapefileHandler();


    bool loadFlightZonePolygon(const QString& filePath);


    bool loadRiskEventPoints(const QString& filePath);


    bool loadRiskLineEvents(const QString& filePath);


    bool loadFishnetLines(const QString& filePath);


    const QVector<QPolygonF>& getFlightZonePolygons() const { return mFlightZonePolygons; }


    QVector<QVector3D> getRiskEventPoints() const;


    QVector<QPair<QVector3D, QVector3D>> getRiskLineEvents() const;


    const QVector<QPair<QVector3D, QVector3D>>& getFishnetLines() const { return mFishnetLines; }


    bool isPointInFlightZone(const QVector3D& point) const;


    QRectF getFlightZoneBounds() const;


    void clear();

private:
    QVector<QPolygonF> mFlightZonePolygons; 
    QVector<QVector3D> mRiskEventPoints;     
    QVector<QPair<QVector3D, QVector3D>> mRiskLineEvents; 
    QVector<QPair<QVector3D, QVector3D>> mFishnetLines;   
    QRectF mFlightZoneBounds;                
    
    QPolygonF geometryToPolygon(const QgsGeometry& geometry);

    QVector3D qgsPointToVector3D(const QgsPoint& point);
};

#endif // SHAPEFILE_HANDLER_H 