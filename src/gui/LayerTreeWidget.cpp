#include "LayerTreeWidget.h"
#include "../log/QgisDebug.h"
#include "../core/WorkspaceState.h"
#include <qgscoordinatetransform.h>
#include <qgsvectorlayer.h>
#include <qgsfeature.h>
#include <qgsgeometry.h>
#include <qgspoint.h>
#include <qgsmultipoint.h>
#include <qgslinestring.h>
#include <qgsmultilinestring.h>
#include <qgspolygon.h>
#include <qgsmultipolygon.h>
#include <qgssinglesymbolrenderer.h>
#include <qgsmarkersymbol.h>
#include <qgslinesymbol.h>
#include <qgsfillsymbol.h>

GLenum LayerNode::createVertices(QVector<QVector3D>& vertices){
    QgsVectorLayer* layer = dynamic_cast<QgsVectorLayer*>(mVectorLayer->layer());
    QgsCoordinateReferenceSystem layerCrs = layer->crs();
    wsp::WindowManager& windowManager = wsp::WindowManager::getInstance();
    QgsCoordinateTransform transform(layerCrs, windowManager.getTargetCrs(), QgsProject::instance());
    GLenum type;
    
    QgsFeature feature;
    QgsFeatureIterator iterator = layer->getFeatures();
    float baseZ = windowManager.getBaseDrawHeight();
    while (iterator.nextFeature(feature)) {
        QgsGeometry geometry = feature.geometry();
        if (geometry.isNull()) continue;
        
        Qgis::WkbType wkbType = geometry.wkbType();
        
        if (QgsWkbTypes::flatType(wkbType) == Qgis::WkbType::Point) {
            QgsPointXY transPoint = transform.transform(geometry.asPoint());
            QVector3D rawPoint = QVector3D(transPoint.x(), transPoint.y(), baseZ);
            vertices.append(windowManager.getModelTransform(rawPoint));
            type = GL_POINTS;
        } 
        else if (QgsWkbTypes::flatType(wkbType) == Qgis::WkbType::MultiPoint) {
            QgsMultiPointXY multiPoint = geometry.asMultiPoint();
            for (int i = 0; i < multiPoint.size(); ++i) {
                QgsPointXY transPoint = transform.transform(multiPoint[i]);
                QVector3D rawPoint = QVector3D(transPoint.x(), transPoint.y(), baseZ);
                vertices.append(windowManager.getModelTransform(rawPoint));
            }
            type = GL_POINTS;
        }
        else if (QgsWkbTypes::flatType(wkbType) == Qgis::WkbType::LineString) {
            QgsPolylineXY line = geometry.asPolyline();
            for (int i = 0; i < line.size(); ++i) {
                QgsPointXY transPoint = transform.transform(line[i]);
                QVector3D rawPoint = QVector3D(transPoint.x(), transPoint.y(), baseZ);
                vertices.append(windowManager.getModelTransform(rawPoint));
            }
            type = GL_LINE_STRIP;
        } 
        else if (QgsWkbTypes::flatType(wkbType) == Qgis::WkbType::MultiLineString) {
            QgsMultiPolylineXY multiLine = geometry.asMultiPolyline();
            for (int partIdx = 0; partIdx < multiLine.size(); ++partIdx) {
                QgsPolylineXY line = multiLine[partIdx];
                for (int i = 0; i < line.size(); ++i) {
                    QgsPointXY transPoint = transform.transform(line[i]);
                    QVector3D rawPoint = QVector3D(transPoint.x(), transPoint.y(), baseZ);
                    vertices.append(windowManager.getModelTransform(rawPoint));
                }
            }
            type = GL_LINE_STRIP;
        }
        else if (QgsWkbTypes::flatType(wkbType) == Qgis::WkbType::Polygon) {
            QgsPolygonXY polygon = geometry.asPolygon();
            for (int ringIdx = 0; ringIdx < polygon.size(); ++ringIdx) {
                QgsPolylineXY ring = polygon[ringIdx];
                for (int i = 0; i < ring.size(); ++i) {
                    QgsPointXY transPoint = transform.transform(ring[i]);
                    QVector3D rawPoint = QVector3D(transPoint.x(), transPoint.y(), baseZ);
                    vertices.append(windowManager.getModelTransform(rawPoint));
                }
            }
            type = GL_TRIANGLE_FAN;
        } 
        else if (QgsWkbTypes::flatType(wkbType) == Qgis::WkbType::MultiPolygon) {
            QgsMultiPolygonXY multiPolygon = geometry.asMultiPolygon();
            for (int partIdx = 0; partIdx < multiPolygon.size(); ++partIdx) {
                QgsPolygonXY polygon = multiPolygon[partIdx];
                for (int ringIdx = 0; ringIdx < polygon.size(); ++ringIdx) {
                    QgsPolylineXY ring = polygon[ringIdx];
                    for (int i = 0; i < ring.size(); ++i) {
                        QgsPointXY transPoint = transform.transform(ring[i]);
                        QVector3D rawPoint = QVector3D(transPoint.x(), transPoint.y(), baseZ);
                        vertices.append(windowManager.getModelTransform(rawPoint));
                    }
                }
            }
            type = GL_TRIANGLE_FAN;
        }
    }
    return type;
}
QColor LayerNode::createSymbol(){
    QgsVectorLayer* layer = dynamic_cast<QgsVectorLayer*>(mVectorLayer->layer());
    QgsFeatureRenderer *renderer = layer->renderer();
    if (renderer->type() == "singleSymbol") {
        QgsSingleSymbolRenderer *singleRenderer = static_cast<QgsSingleSymbolRenderer*>(renderer);
        QgsSymbol *symbol = singleRenderer->symbol();
        
        if (layer->geometryType() == Qgis::GeometryType::Point) {
            QgsMarkerSymbol *markerSymbol = static_cast<QgsMarkerSymbol*>(symbol);
            return markerSymbol->color();
        }
        else if (layer->geometryType() == Qgis::GeometryType::Line) {
            QgsLineSymbol *lineSymbol = static_cast<QgsLineSymbol*>(symbol);
            return lineSymbol->color();
        }
        else if (layer->geometryType() == Qgis::GeometryType::Polygon) {
            QgsFillSymbol *fillSymbol = static_cast<QgsFillSymbol*>(symbol);
            return fillSymbol->color();
        }
    }
    return QColor(Qt::white);
}

LayerNode::LayerNode(std::shared_ptr<QgsLayerTreeLayer> layerNode) : mVectorLayer(layerNode){
    QVector<QVector3D> vertices;
    GLenum primitiveType = createVertices(vertices);
    QColor color = createSymbol();
    float redColor = static_cast<float>(color.red()) / 255, greenColor = static_cast<float>(color.green()) / 255, blueColor = static_cast<float>(color.blue()) / 255, alphaColor = static_cast<float>(color.alpha()) / 255;
    mPrimitive = std::make_shared<gl::VectorPrimitive>(primitiveType, vertices, QVector4D(redColor, greenColor, blueColor, alphaColor / 2));
};

void LayerNode::draw(const QMatrix4x4 &view, const QMatrix4x4 &projection){
    if (mVectorLayer->isVisible())
        mPrimitive->draw(view,projection);
}
LayerNode::~LayerNode(){
    mPrimitive = nullptr;
}

LayerTreeWidget::LayerTreeWidget(QWidget *parent) : QgsLayerTreeView(parent) {
    setSelectionMode(QAbstractItemView::SingleSelection);
    setObjectName("layerTreeWidget");
    mLayerTree = QgsProject::instance()->layerTreeRoot();
    mLayerTreeModel = new QgsLayerTreeModel(mLayerTree,parent);
    mLayerTreeModel->setFlag(QgsLayerTreeModel::ShowLegendAsTree);
    mLayerTreeModel->setFlag(QgsLayerTreeModel::AllowNodeReorder);
    mLayerTreeModel->setFlag(QgsLayerTreeModel::AllowNodeRename);
    mLayerTreeModel->setFlag(QgsLayerTreeModel::AllowNodeChangeVisibility);
    this->setModel(mLayerTreeModel);
    this->setStyleSheet(
        "QgsLayerTreeView {\
            background-color: #2d2d2d;\
            color: #e0e0e0;\
            border: 1px solid #3a3a3a;\
            border-radius: 4px;\
        }"

        "QgsLayerTreeView::item {\
            height: 24px;\
            padding: 2px;\
        }"

        "QgsLayerTreeView::item:hover {\
            background-color: #3a3a3a;\
        }"

        "QgsLayerTreeView::item:selected {\
            background-color: rgba(0, 0, 0, 0.3);\
            border: 1px solid #5a5a5a;\
            color: #ffffff;\
        }"

        "QCheckBox {\
            spacing: 5px;\
        }"

        "QCheckBox::indicator {\
            width: 16px;\
            height: 16px;\
            border: 1px solid #5a5a5a;\
            border-radius: 3px;\
            background: #3d3d3d;\
        }"

        "QCheckBox::indicator:hover {\
            border: 1px solid #7a7a7a;\
            background: #4a4a4a;\
        }"

        "QCheckBox::indicator:checked {\
            background-color: #888888;\
            border: 1px solid #aaaaaa;\
            image: url(:/images/themes/default/mActionChecked.svg);\
        }"

        "QCheckBox::indicator:disabled {\
            background: #2d2d2d;\
            border: 1px solid #444444;\
        }"
    );
}

LayerTreeWidget::~LayerTreeWidget(){
    if (mLayerTree)
        mLayerTree = nullptr;
    if (mLayerTreeModel){
        //delete mLayerTreeModel;
        mLayerTreeModel = nullptr;
    }
}

void LayerTreeWidget::setContext(QOpenGLContext* context){
    this->context = context;
}

bool LayerTreeWidget::addVectorLayer(const QString& filePath){
    if (filePath.isEmpty()) {
        return false;
    }
    QgsVectorLayer* vectorLayer = new QgsVectorLayer(filePath, QFileInfo(filePath).baseName(), "ogr");
    if (!vectorLayer->isValid()) {
        return false;
    }
    QgsProject::instance()->addMapLayer(vectorLayer);
    return true;
}

bool LayerTreeWidget::addRasterLayer(const QString& filePath){
    if (filePath.isEmpty()) {
        return false;
    }
    QgsRasterLayer* rasterLayer = new QgsRasterLayer(filePath, QFileInfo(filePath).baseName());
    if (!rasterLayer->isValid()) {
        return false;
    }
    QgsProject::instance()->addMapLayer(rasterLayer);
    return true;
}

void LayerTreeWidget::drawElements(const QMatrix4x4 &view, const QMatrix4x4 &projection){
    if (nodes.empty())
        return;
    this->context->makeCurrent(this->context->surface());
    for (auto node_it = nodes.rbegin(); node_it != nodes.rend(); ++node_it)
        (*node_it)->draw(view, projection);
}

void LayerTreeWidget::appendLayerNode(QgsLayerTreeNode * node){
    QgsLayerTreeLayer *layerNode = static_cast<QgsLayerTreeLayer*>(node);
    QgsMapLayer *mapLayer = layerNode->layer();
    if (mapLayer->type() == Qgis::LayerType::Vector)
        nodes.push_back(std::make_shared<LayerNode>(std::shared_ptr<QgsLayerTreeLayer>(layerNode)));
}

void LayerTreeWidget::traverseLayerTree(QgsLayerTreeNode *layerTree, const std::function<void(QgsLayerTreeNode *)> &func){
    if (!layerTree) return;
    if (layerTree->nodeType() == QgsLayerTree::NodeLayer)
        func(layerTree);
    else if (layerTree->nodeType() == QgsLayerTree::NodeGroup) {    
        QgsLayerTreeGroup *group = static_cast<QgsLayerTreeGroup*>(layerTree);
        const auto children = group->children();
        for (QgsLayerTreeNode *child : children)
            traverseLayerTree(child, func);
    }
}
void LayerTreeWidget::init3Dresources(){
    if (this->context == nullptr) {
        logMessage("context is null", Qgis::MessageLevel::Critical);
        return;
    }
    this->context->makeCurrent(this->context->surface());
    nodes.clear();
    std::function<void(QgsLayerTreeNode *)> func = [&widget = *this](QgsLayerTreeNode * node) {
        widget.appendLayerNode(node);
    };
    traverseLayerTree(mLayerTree, func);
}
