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

class LayerNode : public QgsVectorLayer{
public:
    LayerNode(const QString &name, const QString &filePath);
    ~LayerNode();

    QString name() const { return mName; }
    void setName(const QString &name) { mName = name; }
    bool isVisible() const { return mVisible; }
    void setVisible(bool visible) { mVisible = visible; }
    std::shared_ptr<gl::Primitive> getPrimitive() { return mPrimitive; }
    void setPrimitive(std::shared_ptr<gl::Primitive> primitive) { mPrimitive = primitive; }

private:
    QString mName;
    bool mVisible;
    std::shared_ptr<gl::Primitive> mPrimitive;
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
};
#endif // LAYERTREEWIDGET_H
