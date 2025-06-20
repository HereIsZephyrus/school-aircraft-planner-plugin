#include "ShapefileHandler.h"
#include "../log/QgisDebug.h"
#include <QFileInfo>
#include <QApplication>
#include <qgspolygon.h>
#include <qgslinestring.h>
#include <qgspoint.h>
#include <qgsproviderregistry.h>
#include <qgsapplication.h>
#include <qgsrectangle.h>
#include <qgswkbtypes.h>
#include <qgspointxy.h>

ShapefileHandler::ShapefileHandler() {
    if (!QgsApplication::instance()) {
        logMessage("QGIS application not initialized", Qgis::MessageLevel::Critical);
    }
    
    QgsProviderRegistry* registry = QgsProviderRegistry::instance();
    if (!registry) {
        logMessage("Cannot access QGIS provider registry", Qgis::MessageLevel::Critical);
    } else {
        QStringList providers = registry->providerList();
        logMessage(QString("Available QGIS providers: %1").arg(providers.join(", ")), Qgis::MessageLevel::Info);
        
        if (!providers.contains("ogr")) {
            logMessage("OGR provider not available - this may cause shapefile loading to fail", Qgis::MessageLevel::Warning);
        } else {
            logMessage("OGR provider is available", Qgis::MessageLevel::Info);
        }
    }
}

ShapefileHandler::~ShapefileHandler() {
    clear();
}

bool ShapefileHandler::loadFlightZonePolygon(const QString& filePath) {
    logMessage(QString("attempting to load flight zone file: %1").arg(filePath), Qgis::MessageLevel::Info);
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        logMessage(QString("file not found: %1").arg(filePath), Qgis::MessageLevel::Critical);
        return false;
    }

    QgsVectorLayer* layer = new QgsVectorLayer(filePath, "flight_zone", "ogr");
    if (!layer || !layer->isValid()) {
        QString errorMsg = layer ? layer->error().message() : "failed to create layer";
        logMessage(QString("cannot load flight zone file: %1, error: %2").arg(filePath).arg(errorMsg), Qgis::MessageLevel::Critical);
        if (layer) delete layer;
        return false;
    }

    logMessage(QString("successfully created layer, feature count: %1").arg(layer->featureCount()), Qgis::MessageLevel::Info);

    mFlightZonePolygons.clear();
    mFlightZoneBounds = QRectF();

    QgsFeatureIterator features = layer->getFeatures();
    QgsFeature feature;
    QRectF bounds;
    bool firstFeature = true;

    while (features.nextFeature(feature)) {
        QgsGeometry geometry = feature.geometry();
        if (geometry.isNull()) continue;

        QPolygonF polygon = geometryToPolygon(geometry);
        if (!polygon.isEmpty()) {
            mFlightZonePolygons.append(polygon);
            
            QRectF polyBounds = polygon.boundingRect();
            if (firstFeature) {
                bounds = polyBounds;
                firstFeature = false;
            } else {
                bounds = bounds.united(polyBounds);
            }
        }
    }

    mFlightZoneBounds = bounds;
    delete layer;

    logMessage(QString("successfully loaded %1 flight zone polygons").arg(mFlightZonePolygons.size()), 
               Qgis::MessageLevel::Success);
    return !mFlightZonePolygons.isEmpty();
}

bool ShapefileHandler::loadRiskEventPoints(const QString& filePath) {
    logMessage(QString("attempting to load risk event points file: %1").arg(filePath), Qgis::MessageLevel::Info);
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        logMessage(QString("file not found: %1").arg(filePath), Qgis::MessageLevel::Critical);
        return false;
    }

    QgsVectorLayer* layer = new QgsVectorLayer(filePath, "risk_points", "ogr");
    if (!layer || !layer->isValid()) {
        QString errorMsg = layer ? layer->error().message() : "failed to create layer";
        logMessage(QString("cannot load risk points file: %1, error: %2").arg(filePath).arg(errorMsg), Qgis::MessageLevel::Critical);
        if (layer) delete layer;
        return false;
    }

    logMessage(QString("successfully created risk points layer, feature count: %1").arg(layer->featureCount()), Qgis::MessageLevel::Info);

    mRiskEventPoints.clear();

    QgsFeatureIterator features = layer->getFeatures();
    QgsFeature feature;
    int featureCount = 0;

    while (features.nextFeature(feature)) {
        featureCount++;
        
        QgsGeometry geometry = feature.geometry();
        if (geometry.isNull()) {
            logMessage(QString("Feature %1: null geometry").arg(featureCount), Qgis::MessageLevel::Warning);
            continue;
        }

        try {
            QgsPointXY point = geometry.asPoint();
            QVector3D point3D(point.x(), point.y(), 0.0);
            mRiskEventPoints.append(point3D);
        } catch (...) {
            logMessage(QString("Feature %1: failed to convert to point").arg(featureCount), Qgis::MessageLevel::Warning);
            continue;
        }
    }

    delete layer;

    logMessage(QString("successfully loaded %1 risk event points").arg(mRiskEventPoints.size()), 
               Qgis::MessageLevel::Success);
    return !mRiskEventPoints.isEmpty();
}

bool ShapefileHandler::loadFishnetLines(const QString& filePath) {
    logMessage(QString("attempting to load fishnet lines file: %1").arg(filePath), Qgis::MessageLevel::Info);
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        logMessage(QString("file not found: %1").arg(filePath), Qgis::MessageLevel::Critical);
        return false;
    }

    QgsVectorLayer* layer = new QgsVectorLayer(filePath, "fishnet_lines", "ogr");
    if (!layer || !layer->isValid()) {
        QString errorMsg = layer ? layer->error().message() : "failed to create layer";
        logMessage(QString("cannot load fishnet lines file: %1, error: %2").arg(filePath).arg(errorMsg), Qgis::MessageLevel::Critical);
        if (layer) delete layer;
        return false;
    }

    logMessage(QString("successfully created fishnet layer, feature count: %1").arg(layer->featureCount()), Qgis::MessageLevel::Info);

    mFishnetLines.clear();

    QgsFeatureIterator features = layer->getFeatures();
    QgsFeature feature;
    int featureCount = 0;
    int processedLines = 0;

    while (features.nextFeature(feature)) {
        featureCount++;
        QgsGeometry geometry = feature.geometry();
        if (geometry.isNull()) {
            logMessage(QString("Feature %1: geometry is null").arg(featureCount), Qgis::MessageLevel::Warning);
            continue;
        }

        if (featureCount <= 3) {
            logMessage(QString("Feature %1: geometry type = %2").arg(featureCount).arg(static_cast<int>(geometry.type())), Qgis::MessageLevel::Info);
            QString wkt = geometry.asWkt();
            if (wkt.length() > 200) {
                wkt = wkt.left(200) + "..."; 
            }
            logMessage(QString("Feature %1: WKT = %2").arg(featureCount).arg(wkt), Qgis::MessageLevel::Info);
        }

        try {
            QgsPolylineXY line = geometry.asPolyline();
            if (featureCount <= 3) {
                logMessage(QString("Feature %1: asPolyline succeeded, points = %2").arg(featureCount).arg(line.size()), Qgis::MessageLevel::Info);
            }
            if (line.size() >= 2) {
                for (int i = 0; i < line.size() - 1; ++i) {
                    QVector3D startPoint(line[i].x(), line[i].y(), 0.0);
                    QVector3D endPoint(line[i+1].x(), line[i+1].y(), 0.0);
                    mFishnetLines.append(qMakePair(startPoint, endPoint));
                    processedLines++;
                }
                continue; 
            } else if (featureCount <= 3) {
                logMessage(QString("Feature %1: line has only %2 points, trying multiline").arg(featureCount).arg(line.size()), Qgis::MessageLevel::Info);
            }
        } catch (...) {
            if (featureCount <= 3) {
                logMessage(QString("Feature %1: asPolyline failed, trying multiline").arg(featureCount), Qgis::MessageLevel::Info);
            }
        }
        
        try {
            QgsMultiPolylineXY multiLine = geometry.asMultiPolyline();
            if (featureCount <= 3) {
                logMessage(QString("Feature %1: asMultiPolyline succeeded, lines = %2").arg(featureCount).arg(multiLine.size()), Qgis::MessageLevel::Info);
            }
            for (const QgsPolylineXY& line : multiLine) {
                if (line.size() >= 2) {
                    for (int i = 0; i < line.size() - 1; ++i) {
                        QVector3D startPoint(line[i].x(), line[i].y(), 0.0);
                        QVector3D endPoint(line[i+1].x(), line[i+1].y(), 0.0);
                        mFishnetLines.append(qMakePair(startPoint, endPoint));
                        processedLines++;
                    }
                }
            }
            if (!multiLine.isEmpty()) {
                continue; 
            }
        } catch (...) {
            if (featureCount <= 3) {
                logMessage(QString("Feature %1: asMultiPolyline failed, trying polygon").arg(featureCount), Qgis::MessageLevel::Info);
            }
        }
        
        try {
            QgsPolygonXY polygon = geometry.asPolygon();
            if (featureCount <= 3) {
                logMessage(QString("Feature %1: asPolygon succeeded, rings = %2").arg(featureCount).arg(polygon.size()), Qgis::MessageLevel::Info);
            }
            if (!polygon.isEmpty() && !polygon.first().isEmpty()) {
                QgsPolylineXY outerRing = polygon.first();
                if (featureCount <= 3) {
                    logMessage(QString("Feature %1: polygon outer ring points = %2").arg(featureCount).arg(outerRing.size()), Qgis::MessageLevel::Info);
                }
                for (int i = 0; i < outerRing.size() - 1; ++i) {
                    QVector3D startPoint(outerRing[i].x(), outerRing[i].y(), 0.0);
                    QVector3D endPoint(outerRing[i+1].x(), outerRing[i+1].y(), 0.0);
                    mFishnetLines.append(qMakePair(startPoint, endPoint));
                    processedLines++;
                }
                continue; 
            } else if (featureCount <= 3) {
                logMessage(QString("Feature %1: polygon is empty").arg(featureCount), Qgis::MessageLevel::Warning);
            }
        } catch (...) {
            if (featureCount <= 3) {
                logMessage(QString("Feature %1: asPolygon also failed, skipping").arg(featureCount), Qgis::MessageLevel::Warning);
            }
            continue;
        }
    }

    delete layer;

    logMessage(QString("successfully processed %1 features and extracted %2 line segments").arg(featureCount).arg(processedLines), 
               Qgis::MessageLevel::Info);
    logMessage(QString("successfully loaded %1 fishnet lines").arg(mFishnetLines.size()), 
               Qgis::MessageLevel::Success);
    
    if (mFishnetLines.isEmpty()) {
        logMessage("Warning: No fishnet lines were extracted from the shapefile", Qgis::MessageLevel::Warning);
    }
    
    return !mFishnetLines.isEmpty();
}

bool ShapefileHandler::loadRiskLineEvents(const QString& filePath) {
    logMessage(QString("attempting to load risk line events file: %1").arg(filePath), Qgis::MessageLevel::Info);

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        logMessage(QString("file not found: %1").arg(filePath), Qgis::MessageLevel::Critical);
        return false;
    }

    QgsVectorLayer* layer = new QgsVectorLayer(filePath, "risk_lines", "ogr");
    if (!layer || !layer->isValid()) {
        QString errorMsg = layer ? layer->error().message() : "failed to create layer";
        logMessage(QString("cannot load risk lines file: %1, error: %2").arg(filePath).arg(errorMsg), Qgis::MessageLevel::Critical);
        if (layer) delete layer;
        return false;
    }

    logMessage(QString("successfully created risk lines layer, feature count: %1").arg(layer->featureCount()), Qgis::MessageLevel::Info);

    mRiskLineEvents.clear();

    QgsFeatureIterator features = layer->getFeatures();
    QgsFeature feature;
    int featureCount = 0;
    int processedLines = 0;

    while (features.nextFeature(feature)) {
        featureCount++;
        
        QgsGeometry geometry = feature.geometry();
        if (geometry.isNull()) {
            logMessage(QString("Feature %1: null geometry").arg(featureCount), Qgis::MessageLevel::Warning);
            continue;
        }

        try {
            QgsPolylineXY line = geometry.asPolyline();
            if (featureCount <= 3) {
                logMessage(QString("Feature %1: asPolyline succeeded, points = %2").arg(featureCount).arg(line.size()), Qgis::MessageLevel::Info);
            }
            if (line.size() >= 2) {
                QVector3D startPoint(line.first().x(), line.first().y(), 0.0);
                QVector3D endPoint(line.last().x(), line.last().y(), 0.0);
                mRiskLineEvents.append(qMakePair(startPoint, endPoint));
                processedLines++;
                continue;
            }
        } catch (...) {
            if (featureCount <= 3) {
                logMessage(QString("Feature %1: asPolyline failed, trying multiline").arg(featureCount), Qgis::MessageLevel::Info);
            }
        }
        
        try {
            QgsMultiPolylineXY multiLine = geometry.asMultiPolyline();
            if (featureCount <= 3) {
                logMessage(QString("Feature %1: asMultiPolyline succeeded, lines = %2").arg(featureCount).arg(multiLine.size()), Qgis::MessageLevel::Info);
            }
            for (const QgsPolylineXY& line : multiLine) {
                if (line.size() >= 2) {
                    QVector3D startPoint(line.first().x(), line.first().y(), 0.0);
                    QVector3D endPoint(line.last().x(), line.last().y(), 0.0);
                    mRiskLineEvents.append(qMakePair(startPoint, endPoint));
                    processedLines++;
                }
            }
        } catch (...) {
            logMessage(QString("Feature %1: asMultiPolyline also failed, skipping").arg(featureCount), Qgis::MessageLevel::Warning);
        }
    }

    delete layer;

    logMessage(QString("successfully processed %1 features and extracted %2 line events").arg(featureCount).arg(processedLines), 
               Qgis::MessageLevel::Info);
    logMessage(QString("successfully loaded %1 risk line events").arg(mRiskLineEvents.size()), 
               Qgis::MessageLevel::Success);
    
    return !mRiskLineEvents.isEmpty();
}

bool ShapefileHandler::isPointInFlightZone(const QVector3D& point) const {
    QPointF qpoint(point.x(), point.y());
    
    for (const QPolygonF& polygon : mFlightZonePolygons) {
        if (polygon.containsPoint(qpoint, Qt::OddEvenFill)) {
            return true;
        }
    }
    return false;
}

QRectF ShapefileHandler::getFlightZoneBounds() const {
    return mFlightZoneBounds;
}

void ShapefileHandler::clear() {
    mFlightZonePolygons.clear();
    mRiskEventPoints.clear();
    mRiskLineEvents.clear();
    mFishnetLines.clear();
    mFlightZoneBounds = QRectF();
}

QPolygonF ShapefileHandler::geometryToPolygon(const QgsGeometry& geometry) {
    QPolygonF polygon;
    
    if (geometry.isNull()) {
        logMessage("geometry is null", Qgis::MessageLevel::Warning);
        return polygon;
    }
    
    logMessage("attempting to convert geometry to polygon", Qgis::MessageLevel::Info);
    
    try {
        QgsRectangle bbox = geometry.boundingBox();
        if (bbox.isNull()) {
            logMessage("geometry has null bounding box", Qgis::MessageLevel::Warning);
            return polygon;
        }
        
        try {
            QgsPolygonXY singlePolygon = geometry.asPolygon();
            if (!singlePolygon.isEmpty() && !singlePolygon.first().isEmpty()) {
                logMessage("successfully converted as single polygon", Qgis::MessageLevel::Info);
                QgsPolylineXY outerRing = singlePolygon.first(); 
                for (const QgsPointXY& point : outerRing) {
                    polygon.append(QPointF(point.x(), point.y()));
                }
                logMessage(QString("extracted %1 points from single polygon").arg(polygon.size()), Qgis::MessageLevel::Info);
                return polygon;
            }
        } catch (...) {
        }
        
        try {
            QgsMultiPolygonXY multiPolygon = geometry.asMultiPolygon();
            if (!multiPolygon.isEmpty() && !multiPolygon.first().isEmpty() && !multiPolygon.first().first().isEmpty()) {
                logMessage("successfully converted as multi polygon", Qgis::MessageLevel::Info);
                QgsPolygonXY firstPolygon = multiPolygon.first();
                QgsPolylineXY outerRing = firstPolygon.first(); 
                for (const QgsPointXY& point : outerRing) {
                    polygon.append(QPointF(point.x(), point.y()));
                }
                logMessage(QString("extracted %1 points from multi polygon").arg(polygon.size()), Qgis::MessageLevel::Info);
                return polygon;
            }
        } catch (...) {
            }
        
        logMessage("failed to extract polygon points, using bounding box as fallback", Qgis::MessageLevel::Warning);
        polygon.append(QPointF(bbox.xMinimum(), bbox.yMinimum()));
        polygon.append(QPointF(bbox.xMaximum(), bbox.yMinimum()));
        polygon.append(QPointF(bbox.xMaximum(), bbox.yMaximum()));
        polygon.append(QPointF(bbox.xMinimum(), bbox.yMaximum()));
                polygon.append(QPointF(bbox.xMinimum(), bbox.yMinimum())); 
        
        logMessage(QString("created bounding box polygon with %1 points").arg(polygon.size()), Qgis::MessageLevel::Info);
        
    } catch (const std::exception& e) {
        logMessage(QString("exception in geometry conversion: %1").arg(e.what()), Qgis::MessageLevel::Critical);
        polygon.clear();
    } catch (...) {
        logMessage("unknown exception in geometry conversion", Qgis::MessageLevel::Critical);
        polygon.clear();
    }

    return polygon;
}

QVector3D ShapefileHandler::qgsPointToVector3D(const QgsPoint& point) {
    return QVector3D(static_cast<float>(point.x()), 
                     static_cast<float>(point.y()), 
                     static_cast<float>(point.z()));
}

QVector<QVector3D> ShapefileHandler::getRiskEventPoints() const {
    return mRiskEventPoints;
}

QVector<QPair<QVector3D, QVector3D>> ShapefileHandler::getRiskLineEvents() const {
    return mRiskLineEvents;
} 