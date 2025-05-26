#include "Primitive.h"
#include "../log/qgis_debug.h"
#include "camera.h"
namespace gl {
using Material = ModelData::Material;
using Texture = QOpenGLTexture;
using MaterialGroup = QVector<ModelData::Vertex>;
using pMaterial = std::shared_ptr<Material>;
using pTexture = std::shared_ptr<Texture>;
using pMaterialGroup = std::shared_ptr<MaterialGroup>;

using MaterialVector = QVector<pMaterial>;
using pMaterialVector = std::shared_ptr<MaterialVector>;
using TextureMap = QMap<QString, pTexture>;
using pTextureMap = std::shared_ptr<TextureMap>;
using MaterialGroupMap = QMap<QString, pMaterialGroup>;
using pMaterialGroupMap = std::shared_ptr<MaterialGroupMap>;
using TexturePair = std::pair<pMaterialVector, pTextureMap>;

Primitive::Primitive(GLenum primitiveType, GLfloat *vertices, GLuint vertexNum,
                     GLuint stride) {
  this->primitiveType = primitiveType;
  this->vertices = vertices;
  this->vertexNum = vertexNum;
  this->stride = stride;
  this->vao.create();
  this->vbo.create();
  this->modelMatrix.setToIdentity();
  shader = nullptr;
  this->vao.bind();
  this->vbo.bind();
  this->vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
  this->vbo.allocate(this->vertices,
                     this->vertexNum * this->stride * sizeof(GLfloat));
  this->vbo.release();
  this->vao.release();
}
Primitive::Primitive(GLenum primitiveType, GLuint stride)
    : shader(nullptr), stride(stride) {
  this->primitiveType = primitiveType;
  this->vao.create();
  this->vbo.create();
  this->modelMatrix.setToIdentity();
}

Primitive::~Primitive() {
  this->vao.destroy();
  this->vbo.destroy();
  shader = nullptr;
  delete[] this->vertices;
}

void Primitive::checkGLError(const QString &funcName) {
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    logMessage(QString("OpenGL error in %1: %2").arg(funcName).arg(err),
               Qgis::MessageLevel::Critical);
  }
}

void Primitive::setModelMatrix(const QMatrix4x4 &matrix) {
  this->modelMatrix = matrix;
}

ColorPrimitive::ColorPrimitive(GLenum primitiveType, GLfloat *vertices,
                               GLuint vertexNum, const QVector4D &color)
    : Primitive(primitiveType, vertices, vertexNum, 3), color(color) {}

void ColorPrimitive::draw() {
  if (!this->shader) {
    logMessage("Shader is not set", Qgis::MessageLevel::Critical);
    return;
  }
  Camera& camera = Camera::getInstance();
  this->shader->bind();
  this->shader->setUniformValue("model", this->modelMatrix);
  this->shader->setUniformValue("view", camera.viewMatrix());
  this->shader->setUniformValue("projection", camera.projectionMatrix());
  this->shader->setUniformValue("vColor", this->color);
  this->vao.bind();
  glDrawArrays(this->primitiveType, 0, this->vertexNum);
  this->vao.release();
  this->shader->release();
  checkGLError("ColorPrimitive::draw");
}

BasePlane::BasePlane(GLfloat *vertices, GLuint vertexNum,
                     const QVector4D &color)
    : ColorPrimitive(GL_LINES, vertices, vertexNum, color) {
  this->shader = constructShader(":/shaders/line.vs", ":/shaders/line.fs");
}

RoutePath::RoutePath(GLfloat *vertices, GLuint vertexNum,
                     const QVector4D &color)
    : ColorPrimitive(GL_LINE_STRIP, vertices, vertexNum, color) {
  this->shader = constructShader(":/shaders/line.vs", ":/shaders/line.fs");
}

ControlPoints::ControlPoints(GLfloat *vertices, GLuint vertexNum,
                             const QVector4D &color)
    : ColorPrimitive(GL_POINTS, vertices, vertexNum, color) {
  this->shader = constructShader(":/shaders/point.vs", ":/shaders/point.fs");
}

ConvexHull::ConvexHull(GLfloat *vertices, GLuint vertexNum,
                       const QVector4D &color)
    : ColorPrimitive(GL_LINE_LOOP, vertices, vertexNum, color) {
  this->shader = constructShader(":/shaders/line.vs", ":/shaders/line.fs");
}

Model::Model(std::shared_ptr<ModelData> modelData)
    : Primitive(GL_TRIANGLES, 5), modelData(modelData) {
  this->shader = constructShader(":/shaders/model.vs", ":/shaders/model.fs");
  this->vao.bind();
  this->vbo.bind();
  this->vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);

  this->vertexNum = modelData->totalVertices;
  GLuint count = this->vertexNum * this->stride;
  this->vertices = new GLfloat[count];
  GLuint index = 0;
  for (const auto& group : *modelData->materialGroups) {
    for (const auto& vertex : *group) {
      vertices[index++] = vertex.position.x();
      vertices[index++] = vertex.position.y();
      vertices[index++] = vertex.position.z();
      vertices[index++] = vertex.texCoord.x();
      vertices[index++] = vertex.texCoord.y();
    }
  }

  this->vbo.allocate(vertices, count * sizeof(GLfloat));

  this->shader->enableAttributeArray(0);
  this->shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, this->stride * sizeof(GLfloat));
  this->shader->enableAttributeArray(1);
  this->shader->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 2, this->stride * sizeof(GLfloat));

  this->vbo.release();
  this->vao.release();

  checkGLError("Model::Model");
}

void Model::draw() {
    if (!this->shader) {
        logMessage("Shader is not set", Qgis::MessageLevel::Critical);
        return;
    }
    Camera& camera = Camera::getInstance();
    this->shader->bind();
    this->shader->setUniformValue("model", this->modelMatrix);
    this->shader->setUniformValue("view", camera.viewMatrix());
    this->shader->setUniformValue("projection", camera.projectionMatrix());
    this->vao.bind();
    glDrawArrays(this->primitiveType, 0, this->vertexNum);
    this->vao.release();
    this->shader->release();
    checkGLError("Model::draw");
}

std::shared_ptr<QOpenGLShaderProgram> constructShader(
    const QString &vertexShaderPath, const QString &fragmentShaderPath,
    const QString &geometryShaderPath) {
  std::shared_ptr<QOpenGLShaderProgram> shader =
      std::make_shared<QOpenGLShaderProgram>();
  if (!shader->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                       vertexShaderPath)) {
    logMessage(QString("Shader Error:") + shader->log(),
               Qgis::MessageLevel::Critical);
    return nullptr;
  }
  if (!geometryShaderPath.isEmpty()) {
    if (!shader->addShaderFromSourceFile(QOpenGLShader::Geometry,
                                         geometryShaderPath)) {
      logMessage(QString("Shader Error:") + shader->log(),
                 Qgis::MessageLevel::Critical);
      return nullptr;
    }
  }
  if (!shader->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                       fragmentShaderPath)) {
    logMessage(QString("Shader Error:") + shader->log(),
               Qgis::MessageLevel::Critical);
    return nullptr;
  }
  if (!shader->link()) {
    logMessage(QString("Shader Link Error:") + shader->log(),
               Qgis::MessageLevel::Critical);
  }
  return shader;
}

ModelData::ModelData(pMaterialVector materials, pTextureMap textures,
                     pMaterialGroupMap materialGroups, GLuint totalVertices)
    : materials(materials), textures(textures),
      materialGroups(materialGroups), totalVertices(totalVertices) {
  mBounds = calculateModelBounds();
}

ModelData::~ModelData() {
  this->materials->clear();
  this->textures->clear();
  this->materialGroups->clear();
  this->materials = nullptr;
  this->textures = nullptr;
  this->materialGroups = nullptr;
}

QString ModelData::retriveMtlPath(const QString &objfilePath) {
  QFile objFile(objfilePath);
  QString mtlPath;
  if (objFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&objFile);
    while (!in.atEnd()) {
      QString line = in.readLine().trimmed();
      if (line.startsWith("mtllib")) {
        mtlPath =
            QFileInfo(objfilePath).absolutePath() + "/" + line.split(" ")[1];
        break;
      }
    }
    objFile.close();
  }
  return mtlPath;
}

std::pair<pMaterialGroupMap, GLuint> ModelData::loadMaterialGroups(const QString &objFilePath) {
  using Vertex = ModelData::Vertex;
  pMaterialGroupMap materialGroups = std::make_shared<MaterialGroupMap>();
  QFile objFile(objFilePath);
  GLuint totalVertices = 0;
  if (objFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&objFile);
    QVector<QVector3D> positions;
    QVector<QVector2D> texCoords;
    QString currentMaterial;

    while (!in.atEnd()) {
      QString line = in.readLine().trimmed();
      QStringList parts = line.split(" ", Qt::SkipEmptyParts);
      if (parts.isEmpty())
        continue;

      if (parts[0] == "usemtl")
        currentMaterial = parts[1];
      else if (parts[0] == "v") // point
        positions.append(QVector3D(parts[1].toFloat(), parts[2].toFloat(),
                                   parts[3].toFloat()));
      else if (parts[0] == "vt") // texture
        texCoords.append(
            QVector2D(parts[1].toFloat(),
                      1.0f - parts[2].toFloat())); // flip y for opengl
      else if (parts[0] == "f") {                  // face
        QVector<Vertex> faceVertices;
        // 三角化面数据（假设是三角形面）
        for (int i = 1; i <= 3; ++i) { // 只处理三角形前三个顶点
          QStringList indices = parts[i].split("/", Qt::KeepEmptyParts);

          Vertex vertex;
          // 位置索引
          int posIndex = indices[0].toInt() - 1;
          if (posIndex >= 0 && posIndex < positions.size())
            vertex.position = positions[posIndex];

          // 纹理坐标索引
          if (indices.size() > 1 && !indices[1].isEmpty()) {
            int texIndex = indices[1].toInt() - 1;
            if (texIndex >= 0 && texIndex < texCoords.size()) {
              vertex.texCoord = texCoords[texIndex];
              vertex.texCoord.setY(
                  1.0f - vertex.texCoord.y()); // OpenGL纹理坐标需要翻转Y轴
            }
          }

          (*materialGroups)[currentMaterial]->append(vertex);
          totalVertices++;
        }
      }
    }
    objFile.close();
  }

  return std::make_pair(materialGroups, totalVertices);
}

std::shared_ptr<ModelData> ModelData::loadObjModel(
    const QString &objFilePath) {
  QString mtlPath = retriveMtlPath(objFilePath);
  if (mtlPath.isEmpty())
    return nullptr;
  TexturePair mtlResource = loadMtl(mtlPath);
  auto materialGroupPair = loadMaterialGroups(objFilePath);

  return std::make_shared<ModelData>(mtlResource.first, mtlResource.second,
                                     materialGroupPair.first, materialGroupPair.second);
}

TexturePair ModelData::loadMtl(const QString &mtlPath) {
  using Material = ModelData::Material;
  QFile file(mtlPath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return std::make_pair(nullptr, nullptr);
  pMaterialVector materials = std::make_shared<MaterialVector>();
  pTextureMap textures = std::make_shared<TextureMap>();
  pMaterial currentMaterial = std::make_shared<Material>();
  QTextStream in(&file);
  while (!in.atEnd()) {
    QString line = in.readLine().trimmed();
    QStringList parts = line.split(" ", Qt::SkipEmptyParts);
    if (parts.isEmpty())
      continue;

    if (parts[0] == "newmtl") {
      if (!currentMaterial->name.isEmpty()) {
        materials->append(currentMaterial);
      }
      currentMaterial = std::make_shared<Material>();
      currentMaterial->name = parts[1];
    } else if (parts[0] == "map_Kd") {
      QString texPath = QFileInfo(mtlPath).absolutePath() + "/" + parts[1];
      if (!textures->contains(currentMaterial->name)) {
        QImage img(texPath);
        if (!img.isNull()) {
          std::shared_ptr<QOpenGLTexture> texture =
              std::make_shared<QOpenGLTexture>(img.mirrored());
          texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
          texture->setMagnificationFilter(QOpenGLTexture::Linear);
          textures->insert(currentMaterial->name, texture);
        }
      }
    }
  }
  if (!currentMaterial->name.isEmpty()) {
    materials->append(currentMaterial);
  }
  return std::make_pair(materials, textures);
}

Bounds ModelData::calculateModelBounds() {
  Bounds bounds;
  bounds.min = QVector3D(FLT_MAX, FLT_MAX, FLT_MAX);
  bounds.max = QVector3D(-FLT_MAX, -FLT_MAX, -FLT_MAX);

  for (const auto &group : *materialGroups) {
    for (const Vertex &v : *group) {
      bounds.min.setX(qMin(bounds.min.x(), v.position.x()));
      bounds.min.setY(qMin(bounds.min.y(), v.position.y()));
      bounds.min.setZ(qMin(bounds.min.z(), v.position.z()));

      bounds.max.setX(qMax(bounds.max.x(), v.position.x()));
      bounds.max.setY(qMax(bounds.max.y(), v.position.y()));
      bounds.max.setZ(qMax(bounds.max.z(), v.position.z()));
    }
  }

  bounds.center = QVector3D((bounds.min.x() + bounds.max.x()) / 2.0f,
                                  (bounds.min.y() + bounds.max.y()) / 2.0f,
                                  (bounds.min.z() + bounds.max.z()) / 2.0f);
  return bounds;
}

QVector3D ModelData::calculateModelCenter() {
  float minX = FLT_MAX, maxX = -FLT_MAX;
  float minY = FLT_MAX, maxY = -FLT_MAX;
  float minZ = FLT_MAX, maxZ = -FLT_MAX;

  for (const auto &group : *materialGroups) {
    for (const Vertex &v : *group) {
      minX = qMin(minX, v.position.x());
      maxX = qMax(maxX, v.position.x());
      minY = qMin(minY, v.position.y());
      maxY = qMax(maxY, v.position.y());
      minZ = qMin(minZ, v.position.z());
      maxZ = qMax(maxZ, v.position.z());
    }
  }

  return QVector3D((minX + maxX) / 2.0f, (minY + maxY) / 2.0f,
                   (minZ + maxZ) / 2.0f);
}
}