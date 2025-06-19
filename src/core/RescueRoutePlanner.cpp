#include "RescueRoutePlanner.h"

QFuture<QString> QgsProcessingManager::executeAlgorithm(const QString &algorithmId, const QVariantMap &parameters)
{
    auto task = [=]() -> QString {
        logMessage("08h2:execute alhorithm");
        try {
            std::unique_ptr<QgsProcessingAlgorithm> algorithm(QgsApplication::processingRegistry()->createAlgorithmById(
                    algorithmId,parameters
                ));
            if (!algorithm) {
                logMessage("08h2:algorithm couldn't found");
                return QString();
            }
            if (!algorithm->prepare(parameters,*mpContext,mpFeedback)){
                logMessage("08h2:algorithm prepare encounter a problem");
                return QString();
            }
            QVariantMap results = algorithm->run(parameters,*mpContext,mpFeedback);
            if (mpFeedback->isCanceled()){
                logMessage("08h2:algorithm execute canceled");
                return QString();
            }
            if (results.contains("OUTPUT")){
                QString layerId = results["OUTPUT"].toString();
                return layerId;
            } else {
                logMessage("08h2:no output");
                return QString();
            }
        } catch (const std::exception& e){
            logMessage("08h2:processing crack down with error:"+QString(e.what()));
            return QString();
        }
    };
    return QtConcurrent::run(task);
}

void RescueRoutePlanner::generateResueRoutePlan(const QgsPoint &accidentPoint)
{
    mvTempResult.clear();
    for (const QgsPointXY& station : mvPointList){
        QFuture<QString> temp = generateShortestPath(station,accidentPoint);

        auto watcher = new QFutureWatcher<QString>(this);
        watcher->setFuture(temp);

        connect(watcher, &QFutureWatcher<QString>::finished,this,[watcher,this]() {
            QString path = watcher->result();
            mvTempResult.append(path);
            if (mvTempResult.size() == mvPointList.size()){
                handleResult(mvTempResult);
            }
            watcher->deleteLater();
        });
        logMessage(QString("08h2:start thread,parameters(%1,%2)").arg(station.x()).arg(station.y()));
    };
}

void RescueRoutePlanner::handleResult(const QList<QString> &resultPath)
{
    // qgis shortestpathpoint2point result only contains one lineString object;
    QString min;
    double minDistance = DBL_MAX;
    for (const QString& tempId : resultPath){
        QgsMapLayer* layerTemp = QgsProcessingUtils::mapLayerFromString(tempId,*QgsProcessingManager::getInstance()->getContext());
        if (layerTemp) {
            QgsVectorLayer* vectorLayer = qobject_cast<QgsVectorLayer*>(layerTemp);
            if (vectorLayer && vectorLayer->isValid()) {
                QgsFeature feature;
                QgsFeatureIterator it = vectorLayer->getFeatures();

                if (it.nextFeature(feature)){
                    QgsGeometry geometry = feature.geometry();

                    QgsDistanceArea da;
                    da.setSourceCrs(vectorLayer->sourceCrs(), QgsProject::instance()->transformContext());
                    da.setEllipsoid(QgsProject::instance()->ellipsoid());

                    double length = 0.0;
                    if (geometry.type() == Qgis::GeometryType::Line) {
                        length = da.measureLength(geometry);
                    } else{
                        continue;
                    }
                    if (length < minDistance) {
                        minDistance = length;
                        min = tempId;
                    }
                }
            }
        }else{
            logMessage("layer invalid，id为:"+tempId);
            continue;
        }
    }
    if (!min.isEmpty()) {
        QgsMapLayer* layerTemp = QgsProcessingUtils::mapLayerFromString(min,*QgsProcessingManager::getInstance()->getContext());
        if (layerTemp) {
            QgsVectorLayer* vectorLayer = qobject_cast<QgsVectorLayer*>(layerTemp);
            if (vectorLayer && vectorLayer->isValid()) {
                QgsLineSymbol* lineSymbol = QgsLineSymbol::createSimple({
                    {"outline_color", "#00bcd4"},
                    {"outline_width", "2"}
                });
                QgsSingleSymbolRenderer* renderer = new QgsSingleSymbolRenderer(lineSymbol);\
                vectorLayer->setRenderer(renderer);
                QgsProject::instance()->addMapLayer(vectorLayer);
            }else {
                logMessage("08h2:no layer avaliable");
            }
        }
    }
}

QFuture<QString> RescueRoutePlanner::generateShortestPath(const QgsPointXY &startPoint, const QgsPointXY &endPoint)
{
    QString algorithmId = "native:shortestpathpointtopoint";
    QVariantMap conf;
    conf.insert(QStringLiteral("INPUT"),mpNetWorkLayer->name());
    conf.insert(QStringLiteral("START_POINT"),startPoint);
    conf.insert(QStringLiteral("END_POINT"),endPoint);
    conf.insert(QStringLiteral("OUTPUT"),"TEMPORARY_OUTPUT");
    return QgsProcessingManager::getInstance()->executeAlgorithm(algorithmId,conf);
}
