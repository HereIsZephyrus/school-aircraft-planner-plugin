#ifndef PRIMITIVE_H
#define PRIMITIVE_H
#include "../core/WorkspaceState.h"
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

namespace gl {
class Primitive : public QObject {
  Q_OBJECT

protected:
  QOpenGLVertexArrayObject vao;
  QOpenGLBuffer vbo;
  QMatrix4x4 modelMatrix;
  std::shared_ptr<QOpenGLShaderProgram> shader;
  GLenum primitiveType;
  GLfloat *vertices;
  GLuint vertexNum;
  GLuint stride;
  void checkGLError(const QString &funcName);
  bool constructShader(const QString &vertexShaderPath,
                       const QString &fragmentShaderPath,
                       const QString &geometryShaderPath = "");

public:
  Primitive(GLenum primitiveType, const QVector<QVector3D> &vertices,
            GLuint stride);                       // RAII constructor
  Primitive(GLenum primitiveType, GLuint stride); // no init data constructor
  void setModelMatrix(const QMatrix4x4 &matrix);
  virtual ~Primitive();
  virtual void draw(const QMatrix4x4 &view, const QMatrix4x4 &projection) = 0;
};

class ColorPrimitive : public Primitive {
  QVector4D color;
  static constexpr QVector4D DEFAULT_COLOR = QVector4D(1.0f, 1.0f, 1.0f, 1.0f);

public:
  ColorPrimitive(GLenum primitiveType, const QVector<QVector3D> &vertices,
                 const QVector4D &color = DEFAULT_COLOR);
  ColorPrimitive(GLenum primitiveType, const QVector4D &color = DEFAULT_COLOR);
  void setColor(const QVector4D &color) { this->color = color; }
  QVector4D getColor() const { return this->color; }
  void draw(const QMatrix4x4 &view, const QMatrix4x4 &projection) override;

protected:
  void initShaderAllocate();
};

class BasePlane : public ColorPrimitive {
  static constexpr GLfloat DEFAULT_SIZE = 100.0f;
  static constexpr GLfloat DEFAULT_STEP = 2.0f;
  static constexpr QVector4D DEFAULT_COLOR = QVector4D(0.6f, 0.6f, 0.6f, 0.5f);

public:
  BasePlane(const QVector4D &color = DEFAULT_COLOR);
};

class RoutePath : public ColorPrimitive {
public:
  RoutePath(const QVector<QVector3D> &vertices,
            const QVector4D &color = QVector4D(1.0f, 1.0f, 1.0f, 1.0f));
};

class HomePoint : public ColorPrimitive {
public:
  HomePoint(const QVector<QVector3D> &vertices,
            const QVector4D &color = QVector4D(1.0f, 1.0f, 1.0f, 1.0f));
};

class ControlPoints : public ColorPrimitive {
public:
  ControlPoints(const QVector<QVector3D> &vertices,
                const QVector4D &color = QVector4D(1.0f, 1.0f, 1.0f, 1.0f));
};

class ConvexHull : public ColorPrimitive {
public:
  ConvexHull(const QVector<QVector3D> &vertices,
             const QVector4D &color = QVector4D(1.0f, 1.0f, 1.0f, 1.0f));
};

class ModelData {
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
  ModelData(pMaterialVector materials = nullptr, pTextureMap textures = nullptr,
            pMaterialGroupMap materialGroups = nullptr,
            GLuint totalVertices = 0);
  ModelData(const QString &objFilePath);
  ~ModelData();
  struct Material {
    QString name;
    QVector3D ambient;
    QVector3D diffuse;
    QVector3D specular;
    float shininess;
    QString diffuseTexture;
    Material(const QString &name = "",
             const QVector3D &ambient = QVector3D(0.2f, 0.2f, 0.2f),
             const QVector3D &diffuse = QVector3D(0.8f, 0.8f, 0.8f),
             const QVector3D &specular = QVector3D(1.0f, 1.0f, 1.0f),
             float shininess = 32.0f, const QString &diffuseTexture = "")
        : name(name), ambient(ambient), diffuse(diffuse), specular(specular),
          shininess(shininess), diffuseTexture(diffuseTexture) {}
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
  Bounds calculateModelBounds();
  void updateGlobalBounds(const Bounds &bounds);
  void applyGlobalCentering();
  QVector3D calculateModelCenter();
  Bounds mBounds;
  GLuint totalVertices;
};
class Model : public Primitive {
  std::shared_ptr<ModelData> modelData;

public:
  Model(std::shared_ptr<ModelData> modelData);
  Model(const QString &objFilePath);
  void draw(const QMatrix4x4 &view, const QMatrix4x4 &projection) override;
  QVector3D getModelCenter() const { return modelData->mBounds.center; }
  const Bounds &getBounds() const { return modelData->mBounds; }
  void setBounds(const Bounds &bounds) { modelData->mBounds = bounds; }
  void cleanupTextures();

protected:
  void loadModel(const QString &objFilePath);
  void initModelData();
};

class Demo : public ColorPrimitive {
public:
  Demo();
  // void draw(const QMatrix4x4 &view, const QMatrix4x4 &projection) override;
};
} // namespace gl


namespace ModelDataLoader {
void loadObjModel(const QString &objFilePath,gl::ModelData* modelData);
QString retriveMtlPath(const QString &objfilePath);
std::pair<gl::ModelData::pMaterialGroupMap, GLuint>
loadMaterialGroups(const QString &filePath);
gl::ModelData::TexturePair loadMtl(const QString &mtlPath);
qint64 calcFaceNum(const QString &objFilePath);
bool displayProgress(qint64 progressUpdateInterval);
}//namespace ModelDataLoader
#endif // PRIMITIVE_H