
#include "RiskEventPlanner.h"
#include "../log/QgisDebug.h"
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDateTime>
#include <QTextStream>
#include <QtMath>
#include <cmath>


#include <qgslinestring.h>
#include <qgsgeometry.h>
#include <qgsfeature.h>
#include <qgsfield.h>

RiskEventPlanner::RiskEventPlanner(QObject *parent)
    : QObject(parent)
    , mShapefileHandler(new ShapefileHandler())
    , mTriangulationHandler(new TriangulationHandler())
    , mPathOptimizer(new PathOptimizer()) {
    
    logMessage("risk event path planner initialized", Qgis::MessageLevel::Info);
}

RiskEventPlanner::~RiskEventPlanner() {
    delete mShapefileHandler;
    delete mTriangulationHandler;
    delete mPathOptimizer;
}

PlanningResult RiskEventPlanner::executePlanning(const PlanningParameters& params) {

    mResult = PlanningResult();
    mParams = params;


    auto validation = validateParameters(params);
    if (!validation.first) {
        mResult.success = false;
        mResult.errorMessage = validation.second;
        return mResult;
    }

    logMessage("start executing risk event path planner", Qgis::MessageLevel::Info);

    try {

        emit progressUpdated(0, "loading data files...");
        if (!loadDataFiles()) {
            mResult.success = false;
            mResult.errorMessage = "data file loading failed";
            emit planningFailed(mResult.errorMessage);
            return mResult;
        }
        emit progressUpdated(25, "data files loaded");


        emit progressUpdated(25, "generating triangulation...");
        if (!generateTriangulation()) {
            mResult.success = false;
            mResult.errorMessage = "triangulation generation failed";
            emit planningFailed(mResult.errorMessage);
            return mResult;
        }
        emit progressUpdated(50, "triangulation generated");


        emit progressUpdated(50, "optimizing path...");
        if (!optimizePath()) {
            mResult.success = false;
            mResult.errorMessage = "path optimization failed";
            emit planningFailed(mResult.errorMessage);
            return mResult;
        }
        emit progressUpdated(80, "path optimization completed");


        emit progressUpdated(80, "performing post-processing...");
        if (!postProcessing()) {
            mResult.success = false;
            mResult.errorMessage = "post-processing failed";
            emit planningFailed(mResult.errorMessage);
            return mResult;
        }
        emit progressUpdated(100, "planning completed");

        mResult.success = true;
        logMessage("risk event path planner completed", Qgis::MessageLevel::Success);
        emit planningCompleted(mResult);

    } catch (const std::exception& e) {
        mResult.success = false;
        mResult.errorMessage = QString("exception occurred during planning: %1").arg(e.what());
        logMessage(mResult.errorMessage, Qgis::MessageLevel::Critical);
        emit planningFailed(mResult.errorMessage);
    }

    return mResult;
}

void RiskEventPlanner::setPlanningParameters(const PlanningParameters& params) {
    mParams = params;
}

bool RiskEventPlanner::exportPathToShapefile(const QString& outputPath) {
    if (mResult.optimalPath.isEmpty()) {
        logMessage("no path data to export", Qgis::MessageLevel::Warning);
        return false;
    }

    try {

        QString csvPath = outputPath;
        csvPath.replace(".shp", ".csv");
        
        QFile csvFile(csvPath);
        if (csvFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream csvOut(&csvFile);
            csvOut.setCodec("UTF-8");
            

            csvOut << "node_id,x,y,z,type,distance_from_start\n";
            
            double totalDistance = 0.0;
            for (int i = 0; i < mResult.optimalPath.size(); ++i) {
                const PathNode& node = mResult.optimalPath[i];
                

                if (i > 0) {
                    const PathNode& prevNode = mResult.optimalPath[i - 1];
                    double segmentDistance = sqrt(
                        pow(node.position.x() - prevNode.position.x(), 2) +
                        pow(node.position.y() - prevNode.position.y(), 2)
                    );
                    totalDistance += segmentDistance;
                }
                
                csvOut << (i + 1) << ","
                      << QString::number(node.position.x(), 'f', 2) << ","
                      << QString::number(node.position.y(), 'f', 2) << ","
                      << QString::number(node.position.z(), 'f', 2) << ","
                      << (node.isRiskEvent ? "risk_event" : "path_point") << ","
                      << QString::number(totalDistance, 'f', 2) << "\n";
            }
            
            csvFile.close();
            logMessage(QString("path exported to CSV: %1").arg(csvPath), Qgis::MessageLevel::Success);
        }
        

        QString wktPath = outputPath;
        wktPath.replace(".shp", ".wkt");
        
        QFile wktFile(wktPath);
        if (wktFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream wktOut(&wktFile);
            wktOut.setCodec("UTF-8");
            

            wktOut << "LINESTRING (";
            for (int i = 0; i < mResult.optimalPath.size(); ++i) {
                const PathNode& node = mResult.optimalPath[i];
                if (i > 0) wktOut << ", ";
                wktOut << QString::number(node.position.x(), 'f', 2) << " "
                      << QString::number(node.position.y(), 'f', 2) << " "
                      << QString::number(node.position.z(), 'f', 2);
            }
            wktOut << ")\n";
            
            wktFile.close();
            logMessage(QString("path exported to WKT: %1").arg(wktPath), Qgis::MessageLevel::Success);
        }
        

        QString statsPath = outputPath;
        statsPath.replace(".shp", "_statistics.txt");
        
        QFile statsFile(statsPath);
        if (statsFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream statsOut(&statsFile);
            statsOut.setCodec("UTF-8");
            
            statsOut << "Path Planning Statistics\n";
            statsOut << "========================\n\n";
            statsOut << "Total path length: " << QString::number(mResult.totalPathLength, 'f', 2) << " meters\n";
            statsOut << "Total nodes: " << mResult.optimalPath.size() << "\n";
            statsOut << "Risk event points: " << mResult.riskEventCount << "\n";
            statsOut << "Flight zones: " << mResult.flightZones.size() << "\n";
            statsOut << "Algorithm: " << mParams.optimizationAlgorithm << "\n\n";
            

            if (!mResult.optimalPath.isEmpty()) {
                double minX = mResult.optimalPath[0].position.x();
                double maxX = minX, minY = mResult.optimalPath[0].position.y(), maxY = minY;
                
                for (const PathNode& node : mResult.optimalPath) {
                    minX = qMin(minX, static_cast<double>(node.position.x()));
                    maxX = qMax(maxX, static_cast<double>(node.position.x()));
                    minY = qMin(minY, static_cast<double>(node.position.y()));
                    maxY = qMax(maxY, static_cast<double>(node.position.y()));
                }
                
                statsOut << "Coordinate bounds:\n";
                statsOut << "  X: " << QString::number(minX, 'f', 2) << " to " << QString::number(maxX, 'f', 2) << "\n";
                statsOut << "  Y: " << QString::number(minY, 'f', 2) << " to " << QString::number(maxY, 'f', 2) << "\n";
            }
            
            statsFile.close();
            logMessage(QString("statistics exported to: %1").arg(statsPath), Qgis::MessageLevel::Success);
        }
        
        logMessage(QString("path data exported successfully to multiple formats"), Qgis::MessageLevel::Success);
        return true;
        
    } catch (const std::exception& e) {
        logMessage(QString("exception occurred during path export: %1").arg(e.what()), Qgis::MessageLevel::Critical);
        return false;
    } catch (...) {
        logMessage("unknown exception occurred during path export", Qgis::MessageLevel::Critical);
        return false;
    }
}

bool RiskEventPlanner::exportTriangulationToShapefile(const QString& outputPath) {
    if (mResult.triangulationTriangles.isEmpty()) {
        logMessage("no triangulation data to export", Qgis::MessageLevel::Warning);
        return false;
    }


    logMessage(QString("triangulation export function to be implemented: %1").arg(outputPath), Qgis::MessageLevel::Info);
    return true;
}

QString RiskEventPlanner::getStatistics() const {
    if (!mResult.success) {
        return QString("planning failed: %1").arg(mResult.errorMessage);
    }

    return QString(
        "=== path planning statistics ===\n"
        "risk event points count: %1\n"
        "triangles count: %2\n"
        "optimal path length: %3 meters\n"
        "path nodes count: %4\n"
        "flight zones count: %5\n"
        "optimization algorithm: %6"
    ).arg(mResult.riskEventCount)
     .arg(mResult.triangleCount)
     .arg(mResult.totalPathLength, 0, 'f', 2)
     .arg(mResult.optimalPath.size())
     .arg(mResult.flightZones.size())
     .arg(mParams.optimizationAlgorithm);
}

void RiskEventPlanner::clear() {
    mResult = PlanningResult();
    mParams = PlanningParameters();
    mShapefileHandler->clear();
    mTriangulationHandler->clear();
    mPathOptimizer->clear();
}

QPair<bool, QString> RiskEventPlanner::validateParameters(const PlanningParameters& params) {

    if (params.flightZoneShapefile.isEmpty()) {
        return qMakePair(false, QString("flight zone file not specified"));
    }

    if (params.riskEventShapefile.isEmpty()) {
        return qMakePair(false, QString("risk event points file not specified"));
    }

    if (params.outputPath.isEmpty()) {
        return qMakePair(false, QString("output path not specified"));
    }


    if (!QFileInfo::exists(params.flightZoneShapefile)) {
        return qMakePair(false, QString("flight zone file not found: %1").arg(params.flightZoneShapefile));
    }

    if (!QFileInfo::exists(params.riskEventShapefile)) {
        return qMakePair(false, QString("风险事件点文件不存在: %1").arg(params.riskEventShapefile));
    }


    if (params.triangulationSpacing <= 0) {
        return qMakePair(false, QString("三角网点间距必须大于0"));
    }

    return qMakePair(true, QString("参数验证通过"));
}

void RiskEventPlanner::asyncExecutePlanning(const PlanningParameters& params) {

    executePlanning(params);
}

bool RiskEventPlanner::loadDataFiles() {

    if (!mShapefileHandler->loadFlightZonePolygon(mParams.flightZoneShapefile)) {
        return false;
    }


    if (!mShapefileHandler->loadRiskEventPoints(mParams.riskEventShapefile)) {
        return false;
    }


    mResult.flightZones = mShapefileHandler->getFlightZonePolygons();
    mResult.riskEventPoints = mShapefileHandler->getRiskEventPoints();
    mResult.riskEventCount = mResult.riskEventPoints.size();

    return true;
}

bool RiskEventPlanner::generateTriangulation() {

    QRectF bounds = mShapefileHandler->getFlightZoneBounds();
    

    QVector<QVector3D> uniformPoints = mTriangulationHandler->generateUniformPoints(
        bounds, 
        mParams.triangulationSpacing, 
        mResult.flightZones
    );


    QVector<QVector3D> allPoints = uniformPoints;
    allPoints.append(mResult.riskEventPoints);


    if (!mTriangulationHandler->performDelaunayTriangulation(allPoints)) {
        return false;
    }


    mResult.triangulationTriangles = mTriangulationHandler->clipTriangulationWithFlightZone(mResult.flightZones);
    mResult.triangleCount = mResult.triangulationTriangles.size();

    return true;
}

bool RiskEventPlanner::optimizePath() {

    mPathOptimizer->setRiskEventPoints(mResult.riskEventPoints);
    mPathOptimizer->setTriangulationData(mResult.triangulationTriangles);
    mPathOptimizer->setFlightZoneConstraints(mResult.flightZones);


    mResult.optimalPath = mPathOptimizer->generateOptimalPath(
        mParams.startPoint, 
        mParams.optimizationAlgorithm
    );

    if (mResult.optimalPath.isEmpty()) {
        return false;
    }


    mResult.totalPathLength = mPathOptimizer->calculatePathLength(mResult.optimalPath);

    return true;
}

bool RiskEventPlanner::postProcessing() {

    if (!createOutputDirectory(mParams.outputPath)) {
        return false;
    }

                
    QString report = generateDetailedReport();
    

    QString reportPath = QDir(mParams.outputPath).filePath("planning_report.txt");
    QFile reportFile(reportPath);
    if (reportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&reportFile);
        out.setCodec("UTF-8");
        out << report;
        reportFile.close();
        logMessage(QString("planning report saved to: %1").arg(reportPath), Qgis::MessageLevel::Info);
    }

    return true;
}

bool RiskEventPlanner::createOutputDirectory(const QString& path) {
    QDir dir;
    if (!dir.exists(path)) {
        if (!dir.mkpath(path)) {
            logMessage(QString("cannot create output directory: %1").arg(path), Qgis::MessageLevel::Critical);
            return false;
        }
    }
    return true;
}

QString RiskEventPlanner::generateDetailedReport() const {
    QString report;
    QTextStream stream(&report);
    
    stream << "risk event path planner report\n";
    stream << "=" << QString("=").repeated(50) << "\n\n";
    
    stream << "planning time: " << QDateTime::currentDateTime().toString() << "\n\n";
    
    stream << "input parameters:\n";
    stream << "  flight zone file: " << mParams.flightZoneShapefile << "\n";
    stream << "  risk event points file: " << mParams.riskEventShapefile << "\n";
    stream << "  triangulation spacing: " << mParams.triangulationSpacing << " meters\n";
    stream << "  optimization algorithm: " << mParams.optimizationAlgorithm << "\n";
    stream << "  start point: (" << mParams.startPoint.x() << ", " << mParams.startPoint.y() << ")\n\n";
    
    stream << "planning results:\n";
    stream << "  risk event points count: " << mResult.riskEventCount << "\n";
    stream << "  triangles count: " << mResult.triangleCount << "\n";
    stream << "  optimal path length: " << QString::number(mResult.totalPathLength, 'f', 2) << " meters\n";
    stream << "  path nodes count: " << mResult.optimalPath.size() << "\n";
    stream << "  flight zones count: " << mResult.flightZones.size() << "\n\n";
    
    stream << "path details:\n";
    for (int i = 0; i < mResult.optimalPath.size(); ++i) {
        const PathNode& node = mResult.optimalPath[i];
        stream << QString("  node %1: (%2, %3) %4\n")
                  .arg(i + 1)
                  .arg(node.position.x(), 0, 'f', 2)
                  .arg(node.position.y(), 0, 'f', 2)
                  .arg(node.isRiskEvent ? "[risk event point]" : "[path point]");
    }
    
    return report;
} 
