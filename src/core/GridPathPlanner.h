

#ifndef GRID_PATH_PLANNER_H
#define GRID_PATH_PLANNER_H

#include <QObject>
#include <QVector>
#include <QVector3D>
#include <QString>
#include <QPolygonF>
#include <QLineF>
#include <QSet>
#include <QHash>
#include <QPair>
#include "ShapefileHandler.h"


struct GridNode {
    QVector3D position;         
    int id;                     
    bool isRiskPoint;          
    bool isObstacle;           
    QVector<int> neighbors;     
    
    GridNode() : id(-1), isRiskPoint(false), isObstacle(false) {}
    GridNode(const QVector3D& pos, int nodeId) 
        : position(pos), id(nodeId), isRiskPoint(false), isObstacle(false) {}
};



struct PathPoint {
    QVector3D position;        
    int riskPointId;          
    bool isRiskPoint;         
    double distanceFromStart;  
    int lineSegmentId;         
    
    PathPoint() : riskPointId(-1), isRiskPoint(false), distanceFromStart(0.0), lineSegmentId(-1) {}
    PathPoint(const QVector3D& pos) 
        : position(pos), riskPointId(-1), isRiskPoint(false), distanceFromStart(0.0), lineSegmentId(-1) {}
};



struct RiskLineEvent {
    int lineId;                   
    QVector3D startPoint;         
    QVector3D endPoint;           
    QVector<int> gridNodeIds;     
    bool isCompleted;             
    
    RiskLineEvent() : lineId(-1), isCompleted(false) {}
    RiskLineEvent(int id, const QVector3D& start, const QVector3D& end) 
        : lineId(id), startPoint(start), endPoint(end), isCompleted(false) {}
};



struct GridPlanningParameters {
    QString fishnetShapefile;       
    QString riskPointsShapefile;     
    QString outputPath;             
    QVector3D startPoint;           
    QString algorithm;               
    double maxRiskPointDistance;     
    
    GridPlanningParameters() : 
        startPoint(558856.516f, 3371566.848f, 50.0f),
        algorithm("TSP_Dijkstra"),
        maxRiskPointDistance(50.0) {}
};


struct LinePlanningParameters {
    QString fishnetShapefile;       
    QString riskLinesShapefile;     
    QString outputPath;             
    QVector3D startPoint;           
    QString algorithm;               
    double maxRiskLineDistance;      
};


struct AreaPlanningParameters {
    QString fishnetShapefile;       
    QString outputPath;             
    QVector3D startPoint;           
    QString algorithm;               
    double coverageThreshold;        
    int maxIterations;               
    double gridSpacing;              
    
    AreaPlanningParameters() : 
        startPoint(558856.516f, 3371566.848f, 50.0f),
        algorithm("Area_Spiral"),
        coverageThreshold(0.95),
        maxIterations(1000),
        gridSpacing(10.0) {}
};


struct GridPlanningResult {
    bool success;                   
    QString errorMessage;           
    QString algorithm;               
    QVector<GridNode> gridNodes;     
    QVector<QVector3D> riskPoints;   
    QVector<PathPoint> optimalPath;  
    int totalRiskPoints;             
    int visitedRiskPoints;           
    double totalPathLength;          
};


struct LinePlanningResult {
    bool success;                   
    QString errorMessage;           
    QString algorithm;               
    QVector<GridNode> gridNodes;     
    QVector<RiskLineEvent> riskLines; 
    QVector<PathPoint> optimalPath;  
    int totalRiskLines;              
    int completedRiskLines;          
    double totalPathLength;          
};


struct AreaPlanningResult {
    bool success;                   
    QString errorMessage;           
    QString algorithm;               
    QVector<GridNode> gridNodes;     
    QVector<PathPoint> optimalPath;  
    double totalPathLength;          
    double coverageRate;             
    int totalGridCells;              
    int coveredGridCells;            
    QVector<QVector3D> uncoveredAreas; 
    double averagePathDensity;       
};


class GridPathPlanner : public QObject {
    Q_OBJECT

public:
    explicit GridPathPlanner(QObject *parent = nullptr);
    ~GridPathPlanner();


    GridPlanningResult executePlanning(const GridPlanningParameters& params);


    LinePlanningResult executeLinePlanning(const LinePlanningParameters& params);


    AreaPlanningResult executeAreaPlanning(const AreaPlanningParameters& params);


    QPair<bool, QString> validateParameters(const GridPlanningParameters& params);


    bool analyzeAreaGridRegularity();


    QPair<bool, QString> validateLineParameters(const LinePlanningParameters& params);


    QPair<bool, QString> validateAreaParameters(const AreaPlanningParameters& params);


    bool exportPathToFiles(const GridPlanningResult& result, const QString& outputPath);


    bool exportLinePathToFiles(const LinePlanningResult& result, const QString& outputPath);


    bool exportAreaPathToFiles(const AreaPlanningResult& result, const QString& outputPath);


    QString getStatistics(const GridPlanningResult& result) const;


    QString getLineStatistics(const LinePlanningResult& result) const;


    QString getAreaStatistics(const AreaPlanningResult& result) const;

public slots:

    void asyncExecutePlanning(const GridPlanningParameters& params);


    void asyncExecuteLinePlanning(const LinePlanningParameters& params);


    void asyncExecuteAreaPlanning(const AreaPlanningParameters& params);

signals:
    
    void progressUpdated(int percentage, const QString& message);

    
    void planningCompleted(const GridPlanningResult& result);

    
    void linePlanningCompleted(const LinePlanningResult& result);

    
    void areaPlanningCompleted(const AreaPlanningResult& result);

    
    void planningFailed(const QString& errorMessage);

private:
    ShapefileHandler* mShapefileHandler;  
    
    QVector<GridNode> mGridNodes;         
    QVector<QVector3D> mRiskPoints;       
    QHash<int, int> mNodeIdToIndex;       
    QVector<QVector<double>> mDistanceMatrix; 
    
    QVector<GridNode> mLineGridNodes;     
    QVector<RiskLineEvent> mRiskLines;    
    QHash<int, int> mLineNodeIdToIndex;   
    
    QVector<GridNode> mAreaGridNodes;     
    QHash<int, int> mAreaNodeIdToIndex;   


    bool buildGridFromFishnet(const QString& shapefile);


    bool loadRiskPoints(const QString& shapefile);


    void associateRiskPointsToGrid(double maxDistance);


    QVector<int> dijkstraPath(int startNodeId, int endNodeId);


    QVector<int> aStarPath(int startNodeId, int endNodeId);


    QVector<PathPoint> solveTSPWithDijkstra(int startNodeId);


    QVector<PathPoint> solveOptimizedGreedyTSP(int startNodeId, const QVector<int>& riskNodeIds);

     
    QVector<int> nearestNeighborTSP(int startNodeId, const QVector<int>& riskNodeIds);

  
    QVector<int> twoOptTSP(const QVector<int>& tour, const QVector<int>& riskNodeIds);


    void buildRiskPointDistanceMatrix(const QVector<int>& riskNodeIds);


    double calculateDistance(const QVector3D& p1, const QVector3D& p2) const;


    double heuristicDistance(int nodeId1, int nodeId2) const;


    int findNearestGridNode(const QVector3D& position) const;


    QVector<int> getRiskNodeIds() const;


    QVector<PathPoint> convertToPathPoints(const QVector<int>& nodeIds) const;


    void clear();


    QSet<int> getConnectedComponent(int startNodeId);


    QVector<QSet<int>> findConnectedComponents();


    int repairGridConnectivity();


    bool loadRiskLines(const QString& shapefile);


    void associateRiskLinesToGrid(double maxDistance);


    QVector<int> findGridPathForLine(const RiskLineEvent& riskLine);


    QVector<PathPoint> solveLineTSP(int startNodeId);


    QVector<PathPoint> solveOptimizedLineGreedy(int startNodeId, const QVector<RiskLineEvent>& riskLines);


    QVector<PathPoint> solveAreaSpiralCoverage(int startNodeId, double coverageThreshold);


    QVector<PathPoint> solveAreaGridCoverage(int startNodeId, double coverageThreshold);


    double calculateCoverageRate(const QSet<int>& visitedNodes) const;


    QVector<QVector3D> findUncoveredAreas(const QSet<int>& visitedNodes) const;


    QVector<GridNode> buildCoverageGrid(double gridSpacing);


    void buildGridConnections();


    int findNextSpiralNode(int currentNodeId, const QSet<int>& visitedNodes, const QSet<int>& availableNodes);


    int findNextGridNode(int currentNodeId, const QSet<int>& visitedNodes);


    int findNextSpiralNodeImproved(int currentNodeId, const QSet<int>& visitedNodes, const QSet<int>& availableNodes);
    int findNextGridNodeImproved(int currentNodeId, const QSet<int>& visitedNodes, const QSet<int>& availableNodes, bool movingRight);
    int findNearestUnvisitedNode(int currentNodeId, const QSet<int>& visitedNodes, const QSet<int>& availableNodes);
    void fillMissingNodes(QVector<PathPoint>& coveragePath, QSet<int>& visitedNodes, const QSet<int>& availableNodes, double coverageThreshold);
    void buildGridStructure();
    QVector<QVector<int>> organizeNodesIntoRows();
    

    bool buildAreaGridFromFishnet(const QString& shapefile);
    void buildAreaGridConnections();
    int findNearestAreaGridNode(const QVector3D& position) const;
    double calculateAreaCoverageRate(const QSet<int>& visitedNodes) const;
    QVector<QVector3D> findAreaUncoveredAreas(const QSet<int>& visitedNodes) const;
    

    int findNextAreaSpiralNodeImproved(int currentNodeId, const QSet<int>& visitedNodes, const QSet<int>& availableNodes);
    int findNextAreaGridNodeImproved(int currentNodeId, const QSet<int>& visitedNodes, const QSet<int>& availableNodes, bool movingRight);
    int findNearestAreaUnvisitedNode(int currentNodeId, const QSet<int>& visitedNodes, const QSet<int>& availableNodes);
    void fillAreaMissingNodes(QVector<PathPoint>& coveragePath, QSet<int>& visitedNodes, const QSet<int>& availableNodes, double coverageThreshold);
    void buildAreaGridStructure();
    QVector<QVector<int>> organizeAreaNodesIntoRows();
    bool isAreaNodeReachable(int fromNodeId, int toNodeId, const QSet<int>& visitedNodes);
};

#endif // GRID_PATH_PLANNER_H 