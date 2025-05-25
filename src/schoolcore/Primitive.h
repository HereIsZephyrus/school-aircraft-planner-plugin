#include <GL/gl.h>
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
#include "WorkspaceState.h"
namespace gl{
std::shared_ptr<QOpenGLShaderProgram> constructShader(const QString& vertexShaderPath, const QString& fragmentShaderPath, const QString& geometryShaderPath="");

class Primitive{
protected:
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer vbo;
    QMatrix4x4 modelMatrix;
    std::shared_ptr<QOpenGLShaderProgram> shader;
    GLenum primitiveType;
    GLfloat* vertices;
    GLuint vertexNum;
public:
    Primitive(GLenum primitiveType, GLfloat* vertices, GLuint vertexNum);
    void setModelMatrix(const QMatrix4x4 &matrix);
    void setShader(std::shared_ptr<QOpenGLShaderProgram> shader);
    virtual ~Primitive();
    virtual void draw()=0;
};

class ColorPrimitive : public Primitive{
    QVector4D color;
    constexpr static int stride = 3;
public:
    ColorPrimitive(GLenum primitiveType, GLfloat* vertices, GLuint vertexNum, const QVector4D& color=QVector4D(1.0f, 1.0f, 1.0f, 1.0f));
    void setColor(const QVector4D& color){this->color = color;}
    QVector4D getColor() const{return this->color;}
    void draw() override;
};

class BasePlane : public ColorPrimitive{
public:
  BasePlane(GLfloat* vertices, GLuint vertexNum, const QVector4D& color=QVector4D(1.0f, 1.0f, 1.0f, 1.0f));
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
    ModelData(pMaterialVector materials = nullptr, pTextureMap textures = nullptr, pMaterialGroupMap materialGroups = nullptr);
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
    static pMaterialGroupMap loadMaterialGroups(const QString &filePath);
    static TexturePair loadMtl(const QString &mtlPath);
    void calculateModelBounds(ModelData *modelData,Bounds &bounds);
    void updateGlobalBounds(const Bounds &bounds);
    void applyGlobalCentering();
    QVector3D calculateModelCenter();
    Bounds mBounds;
};
class Model : public Primitive{
    constexpr static int stride = 5;
    std::shared_ptr<ModelData> modelData;
public:
    Model(GLfloat* vertices, GLuint vertexNum, std::shared_ptr<ModelData> modelData);
    void draw() override;
    QVector3D getModelCenter() const{return modelData->calculateModelCenter();}
    const Bounds& getBounds() const{return modelData->mBounds;}
    void setBounds(const Bounds& bounds){modelData->mBounds = bounds;}
};
}
