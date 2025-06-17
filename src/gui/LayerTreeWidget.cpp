#include "LayerTreeWidget.h"
#include "../log/QgisDebug.h"

LayerNode::LayerNode(const QString &name, const QString &filePath):
    QgsVectorLayer(filePath, name, "ogr") ,mName(name){
    QgsProject::instance()->addMapLayer(this);
    QVector<QVector3D> vertices;
    //readVertices(vertices);
};
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
    mLayerTree = nullptr;
    mLayerTreeModel = nullptr;
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
    this->context->makeCurrent(this->context->surface());
    for (auto node_it = nodes.rbegin(); node_it != nodes.rend(); ++node_it) {
        if ((*node_it)->isVisible())
            (*node_it)->getPrimitive()->draw(view, projection);
    }
}

void LayerTreeWidget::init3Dresources(){
    if (this->context == nullptr) {
        logMessage("context is null", Qgis::MessageLevel::Critical);
        return;
    }
}
