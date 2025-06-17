#ifndef LAYERTREEWIDGET_H
#define LAYERTREEWIDGET_H

#include <qopenglcontext.h>
#include <qgslayertree.h>
#include <qgslayertreeview.h>
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsrasterlayer.h>
#include <qgslayertreemodel.h>
#include <qgslayertreemapcanvasbridge.h>
#include "../opengl/Primitive.h"

class LayerNode{
public:
    LayerNode(std::shared_ptr<QgsLayerTreeLayer> vectorLayer);
    ~LayerNode();
    void draw(const QMatrix4x4 &view, const QMatrix4x4 &projection);

private:
    std::shared_ptr<gl::VectorPrimitive> mPrimitive;
    std::shared_ptr<QgsLayerTreeLayer> mVectorLayer;
    GLenum createVertices(QVector<QVector3D>& vertices);
    QColor createSymbol();
};

class LayerTreeWidget : public QgsLayerTreeView {
    Q_OBJECT

private:
    explicit LayerTreeWidget(QWidget *parent = nullptr);
public:
    static LayerTreeWidget* getInstance() {
        static LayerTreeWidget instance;
        return &instance;
    }
    ~LayerTreeWidget();

    LayerTreeWidget(const LayerTreeWidget&) = delete;
    LayerTreeWidget& operator = (const LayerTreeWidget&) = delete;

    bool addVectorLayer(const QString& filePath);
    bool addRasterLayer(const QString& filePath);

    void setContext(QOpenGLContext* context);
    void setMapCanvasBridge(QgsLayerTreeMapCanvasBridge* bridge) { this->mpLayerTreeCanvasBridge = bridge; }
    void drawElements(const QMatrix4x4 &view, const QMatrix4x4 &projection);
    void init3Dresources();
signals:
    void refreshQgsMapCanvas();

private:
    QOpenGLContext* context;
    QVector<std::shared_ptr<LayerNode>> nodes;
    QgsLayerTree *mLayerTree;
    QgsLayerTreeModel *mLayerTreeModel;
    QgsLayerTreeMapCanvasBridge* mpLayerTreeCanvasBridge;
    void appendLayerNode(QgsLayerTreeNode * node);
    void traverseLayerTree(QgsLayerTreeNode *layerTree, const std::function<void(QgsLayerTreeNode *)> &func);
};
#endif // LAYERTREEWIDGET_H
