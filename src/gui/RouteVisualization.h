
#ifndef ROUTE_VISUALIZATION_H
#define ROUTE_VISUALIZATION_H

#include <QObject>
#include <QVector>
#include <QVector3D>
#include <QPolygonF>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QRectF>
#include "../core/RiskEventPlanner.h"
#include "../core/TriangulationHandler.h"


struct VisualizationStyle {
   
    QColor pathColor;           
    int pathWidth;             
    Qt::PenStyle pathStyle;    
    
  
    QColor riskPointColor;     
    int riskPointSize;          
    QColor riskPointBorderColor; 
    int riskPointBorderWidth;   
 
    QColor startPointColor;     
    int startPointSize;        
    
  
    QColor flightZoneColor;    
    QColor flightZoneBorderColor; 
    int flightZoneBorderWidth;  
    int flightZoneAlpha;       
    
    
    QColor triangulationColor; 
    int triangulationWidth;     
    int triangulationAlpha;    
    bool showTriangulation;    
    
    
    VisualizationStyle() {
        pathColor = QColor(255, 0, 0);        
        pathWidth = 3;
        pathStyle = Qt::SolidLine;
        
        riskPointColor = QColor(255, 255, 0);  
        riskPointSize = 8;
        riskPointBorderColor = QColor(0, 0, 0);
        riskPointBorderWidth = 2;
        
        startPointColor = QColor(0, 255, 0); 
        
        flightZoneColor = QColor(0, 255, 0); 
        flightZoneBorderColor = QColor(0, 150, 0); 
        flightZoneBorderWidth = 2;
        flightZoneAlpha = 50;  
        
        triangulationColor = QColor(100, 100, 100); 
        triangulationWidth = 1;
        triangulationAlpha = 100;
        showTriangulation = true;
    }
};

/**
 * @brief 
 */
struct ViewTransform {
    QRectF worldBounds; 
    QRectF viewBounds; 
    double scale;  
    QPointF offset;  
    
    ViewTransform() : scale(1.0), offset(0, 0) {}
};


class RouteVisualization : public QObject {
    Q_OBJECT

public:
    explicit RouteVisualization(QObject *parent = nullptr);
    ~RouteVisualization();

    void setPlanningResult(const PlanningResult& result);
    void setVisualizationStyle(const VisualizationStyle& style);
    const VisualizationStyle& getVisualizationStyle() const { return mStyle; }
    void render(QPainter& painter, const QRectF& viewRect);
    QRectF calculateOptimalViewBounds() const;
    void setViewTransform(const ViewTransform& transform);
    QPointF worldToView(const QVector3D& worldPoint) const;
    QVector3D viewToWorld(const QPointF& viewPoint) const;
    QRectF getDataBounds() const { return mDataBounds; }
    bool hasData() const;
    bool exportToImage(const QString& filePath, const QSize& size = QSize(1920, 1080));
    void clear();

signals:

    void renderCompleted();

    void dataUpdated();

private:
    PlanningResult mResult;         
    VisualizationStyle mStyle;     
    ViewTransform mViewTransform;  
    QRectF mDataBounds;           

    void drawFlightZones(QPainter& painter);

 
    void drawTriangulation(QPainter& painter);

    void drawRiskEventPoints(QPainter& painter);

    void drawOptimalPath(QPainter& painter);

    void drawStartPoint(QPainter& painter);


    void drawLegend(QPainter& painter, const QRectF& legendRect);

    void calculateDataBounds();


    void setupPainter(QPainter& painter, const QColor& color, int width = 1, 
                     Qt::PenStyle style = Qt::SolidLine, int alpha = 255);


    QBrush createBrush(const QColor& color, int alpha = 255);


    QPolygonF worldPolygonToView(const QPolygonF& worldPolygon) const;
};

#endif // ROUTE_VISUALIZATION_H 