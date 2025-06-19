#ifndef RESCUEROUTEPLANNER_H
#define RESCUEROUTEPLANNER_H

#include <qgsapplication.h>

#include <qgspointxy.h>
#include <qgsprocessing.h>
#include <qgsprocessingregistry.h>
#include <qgsprocessingparameters.h>
#include <qgsprocessingcontext.h>
#include <qgsprocessingmodelalgorithm.h>
#include <qgsprocessingfeedback.h>
#include <qgsprocessingalgrunnertask.h>
#include <qgsprocessingutils.h>

// Async support
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>

// geometry support
#include <qgsvectorlayer.h>
#include <qgsgeometry.h>
#include <qgslinestring.h>
#include <qgsfeature.h>
#include <qgsspatialindex.h>
#include <qgsgeometryutils.h>
#include <qgsfeatureiterator.h>
#include <qgsrectangle.h>

#include <qgslinesymbol.h>
#include <qgssinglesymbolrenderer.h>
#include "../log/QgisDebug.h"

class QgsProcessingManager : public QObject {
    Q_OBJECT
    QgsProcessingManager(){
        this->mpContext = new QgsProcessingContext();
        this->mpFeedback = new QgsProcessingFeedback();
        this->mpContext->setProject(QgsProject::instance());
        this->mpContext->setFeedback(this->mpFeedback);
    };
    ~QgsProcessingManager() {
        if (mpContext != nullptr){
            delete mpContext;
        };
        if (mpFeedback != nullptr){
            delete mpFeedback;
        }
    };

    QgsProcessingContext* mpContext;
    QgsProcessingFeedback* mpFeedback = nullptr;
    QMap<QString,QgsMapLayer*> mvResultLayers;
public:
    static QgsProcessingManager* getInstance(){
        static QgsProcessingManager instance;
        return &instance;
    };
    QFuture<QString> executeAlgorithm(const QString& algorithmId, const QVariantMap& parameters);

    QgsMapLayer* getResultLayer(const QString& resultName) const {
        return mvResultLayers.value(resultName,nullptr);
    };

    void clearResults(){
        mvResultLayers.clear();
        emit resultLayersChanged();
    };

    void flushContext() {
        if (mpContext != nullptr){
            delete mpContext;
        };
        mpContext = new QgsProcessingContext();
        mpContext->setProject(QgsProject::instance());
        mpContext->setFeedback(this->mpFeedback);
    };
    QgsProcessingContext* getContext() const { return this->mpContext; };
    QgsProcessingFeedback* getFeedback() const { return this->mpFeedback; };
signals:
    void algorithmExecuted(bool success, const QString& message);
    void resultLayersChanged();
private:
    void saveResultLayers(const QString& resultId, QgsMapLayer* layer){
        this->mvResultLayers.insert(resultId, layer);
    };
};


class RescueRoutePlanner : public QObject {
    Q_OBJECT
    QString msNetWorkLayerName = "road";
    QString mspoliceStationLayerName = "保安室";
    QgsVectorLayer* mpNetWorkLayer;
    QList<QgsPointXY> mvPointList;
    QList<QString> mvTempResult;
private:
    RescueRoutePlanner(QObject* parent = nullptr) : QObject(parent) {
        mpNetWorkLayer = qobject_cast<QgsVectorLayer*>(
            QgsProject::instance()->mapLayersByName(msNetWorkLayerName).value(0));
        QgsVectorLayer *policeStationsLayer = qobject_cast<QgsVectorLayer*>(
            QgsProject::instance()->mapLayersByName(mspoliceStationLayerName).value(0));

        if (!mpNetWorkLayer || !mpNetWorkLayer->isValid()) {
            logMessage("Road network layer not found or invalid!",Qgis::MessageLevel::Critical);
            return;
        }
        if (!policeStationsLayer || !policeStationsLayer->isValid()) {
            logMessage("Police stations layer not found or invalid!",Qgis::MessageLevel::Critical);
            return;
        }

        QgsFeature feature;
        QgsFeatureIterator it = policeStationsLayer->getFeatures();
        while(it.nextFeature(feature)){
            QgsGeometry geom = feature.geometry();
            if (geom.type() == Qgis::GeometryType::Point){
                mvPointList.append(geom.asPoint());
            }
        }
        logMessage("08h2:load network " + mpNetWorkLayer->name() + ".." + mpNetWorkLayer->sourceName());
        logMessage("08h2:find "+QString::number(mvPointList.size())+" stations");
    }
public:
    static RescueRoutePlanner* getInstance(){
        static RescueRoutePlanner* instance = nullptr;
        if (!instance) {
            instance = new RescueRoutePlanner();
        }
        return instance;
    };
    RescueRoutePlanner(const RescueRoutePlanner&) = delete;
    RescueRoutePlanner& operator=(const RescueRoutePlanner&) = delete;
public slots:
    void generateResueRoutePlan(const QgsPoint& accidentPoint);
private slots:
    void handleResult(const QList<QString>& resultPath);
private:
    QFuture<QString> generateShortestPath(const QgsPointXY& startPoint,const QgsPointXY& endPoint);
};

#endif // RESCUEROUTEPLANNER_H


