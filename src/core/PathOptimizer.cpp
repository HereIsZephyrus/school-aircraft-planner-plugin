
#include "PathOptimizer.h"
#include "../log/QgisDebug.h"
#include <QtMath>
#include <QLineF>
#include <QRandomGenerator>
#include <algorithm>
#include <limits>
#include <random>

PathOptimizer::PathOptimizer() {
}

PathOptimizer::~PathOptimizer() {
    clear();
}

void PathOptimizer::setRiskEventPoints(const QVector<QVector3D>& riskPoints) {
    mRiskEventPoints = riskPoints;
    
    mAllNodes.clear();
    for (int i = 0; i < riskPoints.size(); ++i) {
        mAllNodes.append(PathNode(riskPoints[i], i, true));
    }
    
    buildDistanceMatrix();
    logMessage(QString("set %1 risk event points").arg(riskPoints.size()), 
               Qgis::MessageLevel::Info);
}

void PathOptimizer::setTriangulationData(const QVector<Triangle>& triangles) {
    mTriangles = triangles;
    logMessage(QString("set %1 triangles constraints").arg(triangles.size()), 
               Qgis::MessageLevel::Info);
}

void PathOptimizer::setFlightZoneConstraints(const QVector<QPolygonF>& flightZones) {
    mFlightZones = flightZones;
    logMessage(QString("set %1 flight zone constraints").arg(flightZones.size()), 
               Qgis::MessageLevel::Info);
}

QVector<PathNode> PathOptimizer::generateOptimalPath(const QVector3D& startPoint, const QString& algorithm) {
    if (mRiskEventPoints.isEmpty()) {
        logMessage("no risk event points data", Qgis::MessageLevel::Warning);
        return QVector<PathNode>();
    }

    QVector<PathNode> path;

    if (algorithm == "NearestNeighbor") {
        path = nearestNeighborTSP(startPoint);
        path = twoOptImprovement(path);
    } else if (algorithm == "GeneticAlgorithm") {
        path = geneticAlgorithmTSP(startPoint);
    } else {
        path = nearestNeighborTSP(startPoint);
    }

    double totalLength = calculatePathLength(path);
    logMessage(QString("generate path completed, total length: %1").arg(totalLength, 0, 'f', 2), 
               Qgis::MessageLevel::Success);

    return path;
}

QVector<PathNode> PathOptimizer::nearestNeighborTSP(const QVector3D& startPoint) {
    QVector<PathNode> path;
    QVector<bool> visited(mRiskEventPoints.size(), false);

    path.append(PathNode(startPoint, -1, false));

    int currentIndex = -1;
    double minDistance = std::numeric_limits<double>::max();
    
    for (int i = 0; i < mRiskEventPoints.size(); ++i) {
        double dist = calculateDistance(startPoint, mRiskEventPoints[i]);
        if (dist < minDistance) {
            minDistance = dist;
            currentIndex = i;
        }
    }

    if (currentIndex == -1) return path;

    while (true) {
        visited[currentIndex] = true;
        path.append(PathNode(mRiskEventPoints[currentIndex], currentIndex, true));

        int nextIndex = -1;
        minDistance = std::numeric_limits<double>::max();

        for (int i = 0; i < mRiskEventPoints.size(); ++i) {
            if (!visited[i]) {
                double dist = calculateDistance(mRiskEventPoints[currentIndex], mRiskEventPoints[i]);
                if (dist < minDistance) {
                    minDistance = dist;
                    nextIndex = i;
                }
            }
        }

        if (nextIndex == -1) break; 
        currentIndex = nextIndex;
    }

    path.append(PathNode(startPoint, -1, false));

    return path;
}

QVector<PathNode> PathOptimizer::twoOptImprovement(const QVector<PathNode>& path) {
    if (path.size() < 4) return path; 

    QVector<PathNode> improvedPath = path;
    bool improved = true;

    while (improved) {
        improved = false;
        
        for (int i = 1; i < improvedPath.size() - 2; ++i) {
            for (int j = i + 1; j < improvedPath.size() - 1; ++j) {                 
                double currentLength = calculateDistance(improvedPath[i].position, improvedPath[i+1].position) +
                                     calculateDistance(improvedPath[j].position, improvedPath[j+1].position);
                
                double newLength = calculateDistance(improvedPath[i].position, improvedPath[j].position) +
                                 calculateDistance(improvedPath[i+1].position, improvedPath[j+1].position);

                if (newLength < currentLength) {
                    std::reverse(improvedPath.begin() + i + 1, improvedPath.begin() + j + 1);
                    improved = true;
                }
            }
        }
    }

    return improvedPath;
}

QVector<PathNode> PathOptimizer::geneticAlgorithmTSP(const QVector3D& startPoint, int generations, int populationSize) {
    if (mRiskEventPoints.size() < 2) {
        return nearestNeighborTSP(startPoint);
    }

    QVector<QVector<int>> population;
    for (int i = 0; i < populationSize; ++i) {
        population.append(generateRandomPath(0)); 
    }

    QVector<int> bestPath;
    double bestFitness = std::numeric_limits<double>::max();

    for (int generation = 0; generation < generations; ++generation) {
        QVector<double> fitness(populationSize);
        for (int i = 0; i < populationSize; ++i) {
            fitness[i] = 0.0;
            for (int j = 0; j < population[i].size() - 1; ++j) {
                int from = population[i][j];
                int to = population[i][j + 1];
                if (from < mDistanceMatrix.size() && to < mDistanceMatrix[from].size()) {
                    fitness[i] += mDistanceMatrix[from][to];
                }
            }
            
            if (fitness[i] < bestFitness) {
                bestFitness = fitness[i];
                bestPath = population[i];
            }
        }

        QVector<QVector<int>> newPopulation;
        for (int i = 0; i < populationSize; ++i) {
            int parent1 = QRandomGenerator::global()->bounded(populationSize);
            int parent2 = QRandomGenerator::global()->bounded(populationSize);
            if (fitness[parent2] < fitness[parent1]) {
                std::swap(parent1, parent2);
            }

            QVector<int> offspring = crossoverPaths(population[parent1], population[parent2]);
            
            offspring = mutatePath(offspring);
            
            newPopulation.append(offspring);
        }

        population = newPopulation;
    }

    QVector<PathNode> result;
    result.append(PathNode(startPoint, -1, false));
    
    for (int index : bestPath) {
        if (index < mRiskEventPoints.size()) {
            result.append(PathNode(mRiskEventPoints[index], index, true));
        }
    }
    
    result.append(PathNode(startPoint, -1, false));
    
    return result;
}

bool PathOptimizer::hasDirectPath(const QVector3D& from, const QVector3D& to) {
    return lineIntersectsFlightZone(from, to);
}

QVector<QVector3D> PathOptimizer::findConstrainedPath(const QVector3D& from, const QVector3D& to) {
    if (hasDirectPath(from, to)) {
        return QVector<QVector3D>();
    }

    return aStarPathfinding(from, to);
}

double PathOptimizer::calculatePathLength(const QVector<PathNode>& path) {
    if (path.size() < 2) return 0.0;

    double totalLength = 0.0;
    for (int i = 0; i < path.size() - 1; ++i) {
        totalLength += calculateDistance(path[i].position, path[i+1].position);
    }
    return totalLength;
}

void PathOptimizer::clear() {
    mRiskEventPoints.clear();
    mTriangles.clear();
    mFlightZones.clear();
    mDistanceMatrix.clear();
    mAllNodes.clear();
}

double PathOptimizer::calculateDistance(const QVector3D& p1, const QVector3D& p2) {
    return qSqrt(qPow(p1.x() - p2.x(), 2) + qPow(p1.y() - p2.y(), 2) + qPow(p1.z() - p2.z(), 2));
}

void PathOptimizer::buildDistanceMatrix() {
    int size = mRiskEventPoints.size();
    mDistanceMatrix.clear();
    mDistanceMatrix.resize(size);

    for (int i = 0; i < size; ++i) {
        mDistanceMatrix[i].resize(size);
        for (int j = 0; j < size; ++j) {
            if (i == j) {
                mDistanceMatrix[i][j] = 0.0;
            } else {
                mDistanceMatrix[i][j] = calculateDistance(mRiskEventPoints[i], mRiskEventPoints[j]);
            }
        }
    }
}

bool PathOptimizer::lineIntersectsFlightZone(const QVector3D& from, const QVector3D& to) {
    QLineF line(from.x(), from.y(), to.x(), to.y());
    
    for (const QPolygonF& zone : mFlightZones) {    
        if (zone.containsPoint(QPointF(from.x(), from.y()), Qt::OddEvenFill) &&
            zone.containsPoint(QPointF(to.x(), to.y()), Qt::OddEvenFill)) {
            
            bool intersects = false;
            for (int i = 0; i < zone.size() - 1; ++i) {
                QLineF edge(zone[i], zone[i + 1]);
                QPointF intersection;
                if (line.intersects(edge, &intersection) == QLineF::BoundedIntersection) {
                    intersects = true;
                    break;
                }
            }
            
            if (!intersects) {
                return true; 
            }
        }
    }
    return false;
}

QVector<QVector3D> PathOptimizer::aStarPathfinding(const QVector3D& start, const QVector3D& goal) {
    QVector<QVector3D> path;
    
    QVector3D midPoint = (start + goal) / 2.0f;
    path.append(midPoint);
    
    return path;
}

QVector<Triangle> PathOptimizer::getAdjacentTriangles(const Triangle& triangle) {
    QVector<Triangle> adjacent;
    
    for (const Triangle& other : mTriangles) {
        if (&other == &triangle) continue;
        
        int sharedVertices = 0;
        if (triangle.p1 == other.p1 || triangle.p1 == other.p2 || triangle.p1 == other.p3) sharedVertices++;
        if (triangle.p2 == other.p1 || triangle.p2 == other.p2 || triangle.p2 == other.p3) sharedVertices++;
        if (triangle.p3 == other.p1 || triangle.p3 == other.p2 || triangle.p3 == other.p3) sharedVertices++;
        
        if (sharedVertices >= 2) {
            adjacent.append(other);
        }
    }
    
    return adjacent;
}

QVector<int> PathOptimizer::generateRandomPath(int startIndex) {
    QVector<int> path;
    QVector<int> indices;
    
    for (int i = 0; i < mRiskEventPoints.size(); ++i) {
        if (i != startIndex) {
            indices.append(i);
        }
    }
    
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(indices.begin(), indices.end(), g);
    
    path.append(startIndex);
    path.append(indices);
    path.append(startIndex); 
    
    return path;
}

QVector<int> PathOptimizer::crossoverPaths(const QVector<int>& parent1, const QVector<int>& parent2) {
    if (parent1.size() != parent2.size() || parent1.size() < 4) {
        return parent1;
    }
    
    int size = parent1.size() - 2; 
    int crossPoint1 = QRandomGenerator::global()->bounded(1, size);
    int crossPoint2 = QRandomGenerator::global()->bounded(crossPoint1 + 1, size + 1);
    
    QVector<int> offspring = parent1;
    
    for (int i = crossPoint1; i < crossPoint2; ++i) {
        offspring[i] = parent2[i];
    }
    
    return offspring;
}

QVector<int> PathOptimizer::mutatePath(const QVector<int>& path, double mutationRate) {
    QVector<int> mutated = path;
    
    if (QRandomGenerator::global()->generateDouble() < mutationRate && path.size() > 3) {
        int pos1 = QRandomGenerator::global()->bounded(1, path.size() - 2);
        int pos2 = QRandomGenerator::global()->bounded(1, path.size() - 2);
        if (pos1 != pos2) {
            std::swap(mutated[pos1], mutated[pos2]);
        }
    }
    
    return mutated;
} 