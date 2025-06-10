#ifndef PRIMITIVE_H
#define PRIMITIVE_H
#include "../core/WorkspaceState.h"
#include "../core/Model.h"
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
class Primitive : public QObject{
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

class Model : public Primitive {
  std::shared_ptr<model::ModelData> modelData;

public:
  Model(const QString &objFilePath);
  ~Model();
  void draw(const QMatrix4x4 &view, const QMatrix4x4 &projection) override;
  QVector3D getModelCenter() const { return modelData->mBounds.center; }
  const Bounds &getBounds() const { return modelData->mBounds; }
  void setBounds(const Bounds &bounds) { modelData->mBounds = bounds; }
  void cleanupTextures();
  void loadModel(const QString &objFilePath);

protected:
  std::shared_ptr<QOpenGLTexture> texture;
  void generateTexture(const QString &texturePath);
  void initModelData();
  void initDemoModelData();
};

class Demo : public ColorPrimitive {
public:
  Demo();
  // void draw(const QMatrix4x4 &view, const QMatrix4x4 &projection) override;
};
} // namespace gl
#endif // PRIMITIVE_H