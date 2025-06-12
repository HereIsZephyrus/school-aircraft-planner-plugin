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
#include <qvector3d.h>

namespace gl {
class SelectLine;
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
  static constexpr QVector4D DEFAULT_COLOR = QVector4D(0.0f, 0.2f, 0.8f, 0.9f);

public:
  BasePlane(Bounds bounds = Bounds(), double baseHeight = 0.0, const QVector4D &color = DEFAULT_COLOR);
};

class RoutePath : public ColorPrimitive {
public:
  RoutePath(const QVector<QVector3D> &vertices,
            const QVector4D &color = QVector4D(1.0f, 1.0f, 1.0f, 1.0f));
  const QVector<QVector3D> &getRoutePath() const { return routePath; }
private:
  QVector<QVector3D> routePath;
};

class OrientLine : public ColorPrimitive {
public:
  OrientLine(const QVector<QVector3D> &vertices,
            const QVector4D &color = QVector4D(0.8f, 0.0f, 0.0f, 1.0f));
};

class SinglePoint : public ColorPrimitive {
  friend class SelectLine;
public:
  SinglePoint(const QVector3D &vertices,
            const QVector4D &color = QVector4D(1.0f, 1.0f, 1.0f, 1.0f));
  QVector3D getPoint() const { return point; }
private:
  float pointSize;
  QVector3D point;
};

class SelectLine{
public:
  SelectLine();
  void draw(const QMatrix4x4 &view, const QMatrix4x4 &projection);
  QVector3D submitPoint();
private:
  std::shared_ptr<OrientLine> orientLine;
  std::shared_ptr<SinglePoint> orientPoint;
  QVector<QVector3D> calcOrientLine(float baseHeight);
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

class Drone : public ColorPrimitive {
public:
  Drone(const QString &objFilePath);
  ~Drone();
  void draw(const QMatrix4x4 &view, const QMatrix4x4 &projection) override;
  QVector3D getCenter() const { return modelData->mBounds.center; }
  const Bounds &getBounds() const { return modelData->mBounds; }

protected:
  double mDis2Camera;
  std::shared_ptr<model::ModelData> modelData;
  std::shared_ptr<SinglePoint> center;
  void initModelData();
};

class ModelGroup : public Primitive {
public:
  ModelGroup(const QString &objFileFolderPath);
  ~ModelGroup();
  void draw(const QMatrix4x4 &view, const QMatrix4x4 &projection) override;
  QVector3D getCenter() const { return mBounds.center; }
  const Bounds &getBounds() const { return mBounds; }
  void clear();

protected:
  Bounds mBounds;
  void calcBounds();
  QString objFileFolderPath;
  QVector<QString> objFilePaths;
  QString retriveObjFilePath(const QString &subDirPath);
  QVector<std::shared_ptr<QOpenGLTexture>> textures;
  void generateTexture(const QString &texturePath);
  void initModelData();
  QVector<GLuint> verticesRange;
  QVector<std::shared_ptr<model::ModelData>> models;
  std::shared_ptr<BasePlane> basePlaneWidget;
  //QVector<std::shared_ptr<QOpenGLShaderProgram>> shaders;
  //std::shared_ptr<QOpenGLShaderProgram> constructMultiShader(const QString& vertexShaderPath, const QString& fragmentShaderPath);
};

} // namespace gl
#endif // PRIMITIVE_H