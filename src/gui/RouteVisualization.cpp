
#include "RouteVisualization.h"
#include "../log/QgisDebug.h"
#include <QImage>
#include <QtMath>

RouteVisualization::RouteVisualization(QObject *parent)
    : QObject(parent) {
   
    mStyle = VisualizationStyle(); 
}

RouteVisualization::~RouteVisualization() {
    clear();
}

void RouteVisualization::setPlanningResult(const PlanningResult& result) {
    mResult = result;
    calculateDataBounds();
    emit dataUpdated();
    
    logMessage(QString("route visualization data updated - path nodes: %1, risk points: %2, triangles: %3")
               .arg(result.optimalPath.size())
               .arg(result.riskEventPoints.size())
               .arg(result.triangulationTriangles.size()),
               Qgis::MessageLevel::Info);
}

void RouteVisualization::setVisualizationStyle(const VisualizationStyle& style) {
    mStyle = style;
}

void RouteVisualization::render(QPainter& painter, const QRectF& viewRect) {
    if (!hasData()) {
        logMessage("no data to render", Qgis::MessageLevel::Warning);
        return;
    }

    mViewTransform.viewBounds = viewRect;
    mViewTransform.worldBounds = mDataBounds;

    double scaleX = viewRect.width() / mDataBounds.width();
    double scaleY = viewRect.height() / mDataBounds.height();
    mViewTransform.scale = qMin(scaleX, scaleY) * 0.9; // 留出10%的边距
    

    QPointF dataCenter = mDataBounds.center();
    QPointF viewCenter = viewRect.center();
    mViewTransform.offset = viewCenter - QPointF(dataCenter.x() * mViewTransform.scale, 
                                                 dataCenter.y() * mViewTransform.scale);

    painter.setRenderHint(QPainter::Antialiasing, true);
    

    if (mStyle.showTriangulation) {
        drawTriangulation(painter);
    }
    drawFlightZones(painter);
    drawOptimalPath(painter);
    drawRiskEventPoints(painter);
    drawStartPoint(painter);

    QRectF legendRect(viewRect.right() - 200, viewRect.top() + 10, 190, 150);
    drawLegend(painter, legendRect);
    
    emit renderCompleted();
}

QRectF RouteVisualization::calculateOptimalViewBounds() const {
    if (!hasData()) {
        return QRectF(0, 0, 1000, 1000);
    }

    double margin = 0.1;
    double width = mDataBounds.width();
    double height = mDataBounds.height();
    double marginX = width * margin;
    double marginY = height * margin;
    
    return QRectF(mDataBounds.x() - marginX,
                  mDataBounds.y() - marginY,
                  width + 2 * marginX,
                  height + 2 * marginY);
}

void RouteVisualization::setViewTransform(const ViewTransform& transform) {
    mViewTransform = transform;
}

QPointF RouteVisualization::worldToView(const QVector3D& worldPoint) const {
    return QPointF(worldPoint.x() * mViewTransform.scale + mViewTransform.offset.x(),
                   worldPoint.y() * mViewTransform.scale + mViewTransform.offset.y());
}

QVector3D RouteVisualization::viewToWorld(const QPointF& viewPoint) const {
    return QVector3D(static_cast<float>((viewPoint.x() - mViewTransform.offset.x()) / mViewTransform.scale),
                     static_cast<float>((viewPoint.y() - mViewTransform.offset.y()) / mViewTransform.scale),
                     0.0f);
}

bool RouteVisualization::hasData() const {
    return mResult.success && 
           (!mResult.optimalPath.isEmpty() || !mResult.riskEventPoints.isEmpty());
}

bool RouteVisualization::exportToImage(const QString& filePath, const QSize& size) {
    if (!hasData()) {
        logMessage("no data to export", Qgis::MessageLevel::Warning);
        return false;
    }

    QImage image(size, QImage::Format_ARGB32);
    image.fill(Qt::white);
    
    QPainter painter(&image);
    render(painter, QRectF(0, 0, size.width(), size.height()));
    
    bool success = image.save(filePath);
    if (success) {
        logMessage(QString("route visualization exported to: %1").arg(filePath), 
                   Qgis::MessageLevel::Success);
    } else {
        logMessage(QString("route visualization export failed: %1").arg(filePath), 
                   Qgis::MessageLevel::Critical);
    }
    
    return success;
}

void RouteVisualization::clear() {
    mResult = PlanningResult();
    mDataBounds = QRectF();
    mViewTransform = ViewTransform();
}

void RouteVisualization::drawFlightZones(QPainter& painter) {
    if (mResult.flightZones.isEmpty()) return;
    
    // 设置画刷和画笔
    QColor fillColor = mStyle.flightZoneColor;
    fillColor.setAlpha(mStyle.flightZoneAlpha);
    painter.setBrush(createBrush(fillColor, mStyle.flightZoneAlpha));
    setupPainter(painter, mStyle.flightZoneBorderColor, mStyle.flightZoneBorderWidth);
    
    for (const QPolygonF& zone : mResult.flightZones) {
        QPolygonF viewPolygon = worldPolygonToView(zone);
        painter.drawPolygon(viewPolygon);
    }
}

void RouteVisualization::drawTriangulation(QPainter& painter) {
    if (mResult.triangulationTriangles.isEmpty()) return;
    
    setupPainter(painter, mStyle.triangulationColor, mStyle.triangulationWidth, 
                Qt::SolidLine, mStyle.triangulationAlpha);
    painter.setBrush(Qt::NoBrush);
    
    for (const Triangle& triangle : mResult.triangulationTriangles) {
        QPolygonF trianglePolygon;
        trianglePolygon << QPointF(triangle.p1.x(), triangle.p1.y())
                       << QPointF(triangle.p2.x(), triangle.p2.y())
                       << QPointF(triangle.p3.x(), triangle.p3.y());
        
        QPolygonF viewPolygon = worldPolygonToView(trianglePolygon);
        painter.drawPolygon(viewPolygon);
    }
}

void RouteVisualization::drawRiskEventPoints(QPainter& painter) {
    if (mResult.riskEventPoints.isEmpty()) return;
    

    painter.setBrush(createBrush(mStyle.riskPointColor));
    setupPainter(painter, mStyle.riskPointBorderColor, mStyle.riskPointBorderWidth);
    
    for (const QVector3D& point : mResult.riskEventPoints) {
        QPointF viewPoint = worldToView(point);
        painter.drawEllipse(viewPoint, mStyle.riskPointSize/2, mStyle.riskPointSize/2);
    }
}

void RouteVisualization::drawOptimalPath(QPainter& painter) {
    if (mResult.optimalPath.size() < 2) return;
    
    setupPainter(painter, mStyle.pathColor, mStyle.pathWidth, mStyle.pathStyle);
    painter.setBrush(Qt::NoBrush);
    
    QPainterPath path;
    bool firstPoint = true;
    
    for (const PathNode& node : mResult.optimalPath) {
        QPointF viewPoint = worldToView(node.position);
        
        if (firstPoint) {
            path.moveTo(viewPoint);
            firstPoint = false;
        } else {
            path.lineTo(viewPoint);
        }
    }
    
    painter.drawPath(path);
}

void RouteVisualization::drawStartPoint(QPainter& painter) {
    if (mResult.optimalPath.isEmpty()) return;
    
    const PathNode& startNode = mResult.optimalPath.first();
    if (startNode.isRiskEvent) return;
    
    
    painter.setBrush(createBrush(mStyle.startPointColor));
    setupPainter(painter, QColor(0, 0, 0), 2); 
    
    
    QPointF viewPoint = worldToView(startNode.position);
    painter.drawEllipse(viewPoint, mStyle.startPointSize/2, mStyle.startPointSize/2);
}

void RouteVisualization::drawLegend(QPainter& painter, const QRectF& legendRect) {

    painter.fillRect(legendRect, QColor(255, 255, 255, 200));
    painter.setPen(QPen(QColor(0, 0, 0), 1));
    painter.drawRect(legendRect);
    

    double itemHeight = 20;
    double startY = legendRect.top() + 10;
    double iconX = legendRect.left() + 10;
    double textX = iconX + 30;
    
    painter.setFont(QFont("Arial", 10));
    

    painter.fillRect(QRectF(iconX, startY, 20, 15), createBrush(mStyle.flightZoneColor, mStyle.flightZoneAlpha));
    painter.drawText(QPointF(textX, startY + 12), "flight zone");
    startY += itemHeight;
    
    setupPainter(painter, mStyle.pathColor, mStyle.pathWidth);
    painter.drawLine(QPointF(iconX, startY + 7), QPointF(iconX + 20, startY + 7));
    painter.drawText(QPointF(textX, startY + 12), "optimal path");
    startY += itemHeight;

    painter.setBrush(createBrush(mStyle.riskPointColor));
    painter.setPen(QPen(mStyle.riskPointBorderColor, 1));
    painter.drawEllipse(QRectF(iconX + 5, startY + 2, 10, 10));
    painter.drawText(QPointF(textX, startY + 12), "risk event point");
    startY += itemHeight;
    
    painter.setBrush(createBrush(mStyle.startPointColor));
    painter.setPen(QPen(QColor(0, 0, 0), 1));
    painter.drawEllipse(QRectF(iconX + 3, startY, 14, 14));
    painter.drawText(QPointF(textX, startY + 12), "start point");
    startY += itemHeight;

    if (mStyle.showTriangulation) {
        setupPainter(painter, mStyle.triangulationColor, 1, Qt::SolidLine, mStyle.triangulationAlpha);
        painter.setBrush(Qt::NoBrush);
        QPolygonF miniTriangle;
        miniTriangle << QPointF(iconX, startY + 10) << QPointF(iconX + 15, startY + 10) << QPointF(iconX + 7, startY);
        painter.drawPolygon(miniTriangle);
        painter.drawText(QPointF(textX, startY + 12), "triangulation");
    }
}

void RouteVisualization::calculateDataBounds() {
    if (!mResult.success) {
        mDataBounds = QRectF();
        return;
    }
    
    QVector<QPointF> allPoints;

    for (const PathNode& node : mResult.optimalPath) {
        allPoints.append(QPointF(node.position.x(), node.position.y()));
    }

    for (const QVector3D& point : mResult.riskEventPoints) {
        allPoints.append(QPointF(point.x(), point.y()));
    }

    for (const QPolygonF& zone : mResult.flightZones) {
        for (const QPointF& point : zone) {
            allPoints.append(point);
        }
    }
    
    for (const Triangle& triangle : mResult.triangulationTriangles) {
        allPoints.append(QPointF(triangle.p1.x(), triangle.p1.y()));
        allPoints.append(QPointF(triangle.p2.x(), triangle.p2.y()));
        allPoints.append(QPointF(triangle.p3.x(), triangle.p3.y()));
    }
    
    if (allPoints.isEmpty()) {
        mDataBounds = QRectF(0, 0, 1000, 1000);
        return;
    }
    

    double minX = allPoints.first().x();
    double maxX = minX;
    double minY = allPoints.first().y();
    double maxY = minY;
    
    for (const QPointF& point : allPoints) {
        minX = qMin(minX, point.x());
        maxX = qMax(maxX, point.x());
        minY = qMin(minY, point.y());
        maxY = qMax(maxY, point.y());
    }
    
    mDataBounds = QRectF(minX, minY, maxX - minX, maxY - minY);
    

    if (mDataBounds.width() < 1.0) {
        mDataBounds.setWidth(1.0);
    }
    if (mDataBounds.height() < 1.0) {
        mDataBounds.setHeight(1.0);
    }
}

void RouteVisualization::setupPainter(QPainter& painter, const QColor& color, int width, 
                                     Qt::PenStyle style, int alpha) {
    QColor penColor = color;
    if (alpha < 255) {
        penColor.setAlpha(alpha);
    }
    painter.setPen(QPen(penColor, width, style));
}

QBrush RouteVisualization::createBrush(const QColor& color, int alpha) {
    QColor brushColor = color;
    if (alpha < 255) {
        brushColor.setAlpha(alpha);
    }
    return QBrush(brushColor);
}

QPolygonF RouteVisualization::worldPolygonToView(const QPolygonF& worldPolygon) const {
    QPolygonF viewPolygon;
    for (const QPointF& worldPoint : worldPolygon) {
        QVector3D world3D(static_cast<float>(worldPoint.x()), static_cast<float>(worldPoint.y()), 0.0f);
        viewPolygon.append(worldToView(world3D));
    }
    return viewPolygon;
} 