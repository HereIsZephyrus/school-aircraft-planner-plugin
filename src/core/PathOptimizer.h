
#ifndef PATH_OPTIMIZER_H
#define PATH_OPTIMIZER_H

#include <QVector>
#include <QVector3D>
#include <QPolygonF>
#include <QPair>
#include "TriangulationHandler.h"


struct PathNode {
    QVector3D position;     
    int riskEventId;        
    bool isRiskEvent;       
    
    PathNode() : riskEventId(-1), isRiskEvent(false) {}
    PathNode(const QVector3D& pos, int id = -1, bool isRisk = false) 
        : position(pos), riskEventId(id), isRiskEvent(isRisk) {}
};


class PathOptimizer {
public:
    PathOptimizer();
    ~PathOptimizer();


    void setRiskEventPoints(const QVector<QVector3D>& riskPoints);


    void setTriangulationData(const QVector<Triangle>& triangles);


    void setFlightZoneConstraints(const QVector<QPolygonF>& flightZones);


    QVector<PathNode> generateOptimalPath(const QVector3D& startPoint, 
                                         const QString& algorithm = "NearestNeighbor");


    QVector<PathNode> nearestNeighborTSP(const QVector3D& startPoint);


    QVector<PathNode> twoOptImprovement(const QVector<PathNode>& path);

  
    QVector<PathNode> geneticAlgorithmTSP(const QVector3D& startPoint, 
                                         int generations = 100, 
                                         int populationSize = 50);


    bool hasDirectPath(const QVector3D& from, const QVector3D& to);


    QVector<QVector3D> findConstrainedPath(const QVector3D& from, const QVector3D& to);


    double calculatePathLength(const QVector<PathNode>& path);


    const QVector<QVector<double>>& getDistanceMatrix() const { return mDistanceMatrix; }


    void clear();

private:
    QVector<QVector3D> mRiskEventPoints;     
    QVector<Triangle> mTriangles;             
    QVector<QPolygonF> mFlightZones;         
    QVector<QVector<double>> mDistanceMatrix; 
    QVector<PathNode> mAllNodes;              


    double calculateDistance(const QVector3D& p1, const QVector3D& p2);


    void buildDistanceMatrix();



    bool lineIntersectsFlightZone(const QVector3D& from, const QVector3D& to);


    QVector<QVector3D> aStarPathfinding(const QVector3D& start, const QVector3D& goal);


    QVector<Triangle> getAdjacentTriangles(const Triangle& triangle);


    QVector<int> generateRandomPath(int startIndex);


    QVector<int> crossoverPaths(const QVector<int>& parent1, const QVector<int>& parent2);

    
    QVector<int> mutatePath(const QVector<int>& path, double mutationRate = 0.1);
};

#endif // PATH_OPTIMIZER_H 