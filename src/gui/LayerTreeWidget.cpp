#include "LayerTreeWidget.h"
#include "../log/QgisDebug.h"

LayerNode::LayerNode(std::shared_ptr<QgsLayerTreeLayer> layerNode) : mVectorLayer(layerNode){
    
    QVector<QVector3D> vertices;
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
    this->context->makeCurrent(this->context->surface());
    for (auto node_it = nodes.rbegin(); node_it != nodes.rend(); ++node_it) {
        
    }
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
        for (QgsLayerTreeNode *child : children) {
            traverseLayerTree(child, func);
        }
    }
}
void LayerTreeWidget::init3Dresources(){
    if (this->context == nullptr) {
        logMessage("context is null", Qgis::MessageLevel::Critical);
        return;
    }
    nodes.clear();
    std::function<void(QgsLayerTreeNode *)> func = [&widget = *this](QgsLayerTreeNode * node) {
        widget.appendLayerNode(node);
    };
    traverseLayerTree(mLayerTree, func);
}
