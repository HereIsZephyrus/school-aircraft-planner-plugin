
#ifndef RISK_EVENT_PLANNER_H
#define RISK_EVENT_PLANNER_H

#include <QObject>
#include <QVector>
#include <QVector3D>
#include <QString>
#include <QPolygonF>
#include "ShapefileHandler.h"
#include "TriangulationHandler.h"
#include "PathOptimizer.h"


struct PlanningParameters {
    QString flightZoneShapefile;       
    QString riskEventShapefile;        
    QString outputPath;                
    QVector3D startPoint;              
    double triangulationSpacing;       
    QString optimizationAlgorithm;     
    
    PlanningParameters() : 
        startPoint(558856.516f, 3371566.848f, 50.0f),
        triangulationSpacing(10.0), 
        optimizationAlgorithm("NearestNeighbor") {}
};


struct PlanningResult {
    bool success;                                   
    QString errorMessage;                           
    QVector<PathNode> optimalPath;                  
    QVector<Triangle> triangulationTriangles;      
    QVector<QPolygonF> flightZones;                 
    QVector<QVector3D> riskEventPoints;             
    double totalPathLength;                         
    int triangleCount;                              
    int riskEventCount;                             
    
    PlanningResult() : success(false), totalPathLength(0.0), triangleCount(0), riskEventCount(0) {}
};


class RiskEventPlanner : public QObject {
    Q_OBJECT

public:
    explicit RiskEventPlanner(QObject *parent = nullptr);
    ~RiskEventPlanner();


    PlanningResult executePlanning(const PlanningParameters& params);


    void setPlanningParameters(const PlanningParameters& params);


    const PlanningResult& getPlanningResult() const { return mResult; }


    bool exportPathToShapefile(const QString& outputPath);


    bool exportTriangulationToShapefile(const QString& outputPath);


    QString getStatistics() const;


    void clear();


    QPair<bool, QString> validateParameters(const PlanningParameters& params);

public slots:

    void asyncExecutePlanning(const PlanningParameters& params);

signals:

    void progressUpdated(int percentage, const QString& message);


    void planningCompleted(const PlanningResult& result);


    void planningFailed(const QString& errorMessage);

private:
    PlanningParameters mParams;         
    PlanningResult mResult;             
    
    ShapefileHandler* mShapefileHandler;     
    TriangulationHandler* mTriangulationHandler; 
    PathOptimizer* mPathOptimizer;           



    bool loadDataFiles();


    bool generateTriangulation();


    bool optimizePath();


    bool postProcessing();


    bool createOutputDirectory(const QString& path);


    QString generateDetailedReport() const;
};

#endif // RISK_EVENT_PLANNER_H 