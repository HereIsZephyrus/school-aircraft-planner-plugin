#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QVector3D>
#include <QVector2D>
#include <memory>
#include <QMap>
#include <QVector>
#include <QOpenGLTexture>
#include "WorkspaceState.h"
namespace model{
struct Vertex {
QVector3D position;
QVector2D texCoord;
Vertex(QVector3D pos = QVector3D(0, 0, 0), QVector2D tex = QVector2D(0, 0))
    : position(pos), texCoord(tex) {}
};
struct Material {
    QString name;
    QVector3D ambient;
    QVector3D diffuse;
    QVector3D specular;
    float shininess;
    QString diffuseTexture;
    Material(const QString &name = "",
                const QVector3D &ambient = QVector3D(1.0f, 1.0f, 1.0f),
                const QVector3D &diffuse = QVector3D(1.0f, 1.0f, 1.0f),
                const QVector3D &specular = QVector3D(1.0f, 1.0f, 1.0f),
                float shininess = 32.0f, const QString &diffuseTexture = "")
        : name(name), ambient(ambient), diffuse(diffuse), specular(specular),
            shininess(shininess), diffuseTexture(diffuseTexture) {}
};

class ModelData {
public:
    ModelData(const QString &objFilePath);
    ~ModelData();
    Material material;
    QString texturePath;
    QString objFilePath;
    QVector<Vertex> vertices;
    Bounds calculateModelBounds();
    void updateGlobalBounds(const Bounds &bounds);
    void applyGlobalCentering();
    QVector3D calculateModelCenter();
    Bounds mBounds;
};

QString retriveMtlPath(const QString &objfilePath);
void loadMtl(const QString &mtlPath, Material &material, QString &texturePath);
void loadVertices(const QString &objFilePath, QVector<Vertex> &vertices);
qint64 calcFaceNum(const QString &objFilePath);
bool displayProgress(qint64 progressUpdateInterval);
}

#endif