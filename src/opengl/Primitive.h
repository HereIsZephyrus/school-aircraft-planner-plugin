#pragma once
//#include <GL/gl.h>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QVector2D>
#include <QVector3D>
#include <QVector>
#include <QtMath>
#include <cfloat>
#include <memory>
#include "../core/WorkspaceState.h"
namespace gl{

class Primitive : protected QOpenGLFunctions{

protected:
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer vbo;
    QMatrix4x4 modelMatrix;
    std::shared_ptr<QOpenGLShaderProgram> shader;
    GLenum primitiveType;
    GLfloat* vertices;
    GLuint vertexNum;
    GLuint stride;
    void checkGLError(const QString& funcName);
    bool constructShader(const QString& vertexShaderPath, const QString& fragmentShaderPath, const QString& geometryShaderPath="");
public:
    Primitive(GLenum primitiveType, GLfloat* vertices, GLuint vertexNum, GLuint stride); // RAII constructor
    Primitive(GLenum primitiveType, GLuint stride); // no init data constructor
    void setModelMatrix(const QMatrix4x4 &matrix);
    virtual ~Primitive();
    virtual void draw()=0;
};

class ColorPrimitive : public Primitive{
    QVector4D color;
    static constexpr QVector4D DEFAULT_COLOR = QVector4D(1.0f, 1.0f, 1.0f, 1.0f);
public:
    ColorPrimitive(GLenum primitiveType, GLfloat* vertices, GLuint vertexNum, const QVector4D& color=DEFAULT_COLOR);
    ColorPrimitive(GLenum primitiveType, const QVector4D& color=DEFAULT_COLOR);
    void setColor(const QVector4D& color){this->color = color;}
    QVector4D getColor() const{return this->color;}
    void draw() override;
};

class BasePlane : public ColorPrimitive{
    static constexpr GLfloat DEFAULT_SIZE = 100.0f;
    static constexpr GLfloat DEFAULT_STEP = 2.0f;
    static constexpr QVector4D DEFAULT_COLOR = QVector4D(0.6f, 0.6f, 0.6f, 0.5f);
public:
  BasePlane(const QVector4D& color=DEFAULT_COLOR);
};

class RoutePath : public ColorPrimitive{
public:
  RoutePath(GLfloat* vertices, GLuint vertexNum, const QVector4D& color=QVector4D(1.0f, 1.0f, 1.0f, 1.0f));
};

class ControlPoints : public ColorPrimitive{
public:
  ControlPoints(GLfloat* vertices, GLuint vertexNum=1, const QVector4D& color=QVector4D(1.0f, 1.0f, 1.0f, 1.0f));
};

class ConvexHull : public ColorPrimitive{
public:
  ConvexHull(GLfloat* vertices, GLuint vertexNum, const QVector4D& color=QVector4D(1.0f, 1.0f, 1.0f, 1.0f));
};

class ModelData{
public:
    struct Material;
    struct Vertex;
    using pMaterial = std::shared_ptr<Material>;
    using pTexture = std::shared_ptr<QOpenGLTexture>;
    using pMaterialGroup = std::shared_ptr<QVector<Vertex>>;
    using pMaterialVector = std::shared_ptr<QVector<pMaterial>>;
    using pTextureMap = std::shared_ptr<QMap<QString, pTexture>>;
    using pMaterialGroupMap = std::shared_ptr<QMap<QString, pMaterialGroup>>;
    using TexturePair = std::pair<pMaterialVector, pTextureMap>;
public:
    ModelData(pMaterialVector materials = nullptr, pTextureMap textures = nullptr, pMaterialGroupMap materialGroups = nullptr, GLuint totalVertices = 0);
    ~ModelData();
    struct Material {
        QString name;
        QVector3D ambient;
        QVector3D diffuse;
        QVector3D specular;
        float shininess;
        QString diffuseTexture;
        Material(const QString& name = "", 
                    const QVector3D& ambient = QVector3D(0.2f, 0.2f, 0.2f), 
                    const QVector3D& diffuse = QVector3D(0.8f, 0.8f, 0.8f), 
                    const QVector3D& specular = QVector3D(1.0f, 1.0f, 1.0f), 
                    float shininess=32.0f, 
                    const QString& diffuseTexture="")
        : name(name), ambient(ambient), diffuse(diffuse), specular(specular), shininess(shininess), diffuseTexture(diffuseTexture) {}
    };

    struct Vertex {
        QVector3D position;
        QVector2D texCoord;
        Vertex(QVector3D pos = QVector3D(0, 0, 0), QVector2D tex = QVector2D(0, 0))
                : position(pos), texCoord(tex) {}
    };
    pMaterialVector materials;
    pTextureMap textures;
    pMaterialGroupMap materialGroups;
    static std::shared_ptr<ModelData> loadObjModel(const QString &objFilePath);
    static QString retriveMtlPath(const QString &objfilePath);
    static std::pair<pMaterialGroupMap, GLuint> loadMaterialGroups(const QString &filePath);
    static TexturePair loadMtl(const QString &mtlPath);
    Bounds calculateModelBounds();
    void updateGlobalBounds(const Bounds &bounds);
    void applyGlobalCentering();
    QVector3D calculateModelCenter();
    Bounds mBounds;
    GLuint totalVertices;
};
class Model : public Primitive{
    std::shared_ptr<ModelData> modelData;
public:
    Model(std::shared_ptr<ModelData> modelData);
    Model(const QString& objFilePath);
    void draw() override;
    QVector3D getModelCenter() const{return modelData->mBounds.center;}
    const Bounds& getBounds() const{return modelData->mBounds;}
    void setBounds(const Bounds& bounds){modelData->mBounds = bounds;}
protected:
    void loadModel(const QString& objFilePath);
};
}
