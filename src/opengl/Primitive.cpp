#include "Primitive.h"
#include "../log/QgisDebug.h"
#include "../core/WorkspaceState.h"
#include "Camera.h"
#include <GL/gl.h>
#include <QOpenGLContext>
#include <QProgressDialog>
#include <QCoreApplication>
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

Primitive::Primitive(GLenum primitiveType, const QVector<QVector3D>& vertices, GLuint stride){
  this->primitiveType = primitiveType;
  this->vertexNum = vertices.size();
  this->vertices = new GLfloat[this->vertexNum * 3];
  for (GLuint i = 0; i < this->vertexNum; i++) {
    this->vertices[i * 3] = vertices[i].x();
    this->vertices[i * 3 + 1] = vertices[i].y();
    this->vertices[i * 3 + 2] = vertices[i].z();
  }
  this->stride = stride;
  this->vao.create();
  this->vbo.create();
  this->modelMatrix.setToIdentity();
  this->shader = nullptr;
  logMessage("Primitive created", Qgis::MessageLevel::Info);
  this->vao.bind();
  this->vbo.bind();
  this->vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
  this->vbo.allocate(this->vertices,
                     this->vertexNum * this->stride * sizeof(GLfloat));
  this->vbo.release();
  this->vao.release();
  logMessage("Primitive data initialized", Qgis::MessageLevel::Info);
}

Primitive::Primitive(GLenum primitiveType, GLuint stride)
    : shader(nullptr), stride(stride) {
  this->primitiveType = primitiveType;
  vao.create();
  vbo.create();
  vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
  modelMatrix.setToIdentity();
}

Primitive::~Primitive() {
  logMessage("ready to destroy Primitive", Qgis::MessageLevel::Info);
  if (QOpenGLContext::currentContext()) {
    this->vao.destroy();
    this->vbo.destroy();
    this->shader = nullptr;
    delete[] this->vertices;
  }
  logMessage("Primitive destroyed", Qgis::MessageLevel::Success);
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

ColorPrimitive::ColorPrimitive(GLenum primitiveType, const QVector4D& color)
    : Primitive(primitiveType, 3), color(color){}

ColorPrimitive::ColorPrimitive(GLenum primitiveType, const QVector<QVector3D>& vertices, const QVector4D& color)
    : Primitive(primitiveType, vertices, 3), color(color){}

void ColorPrimitive::draw(const QMatrix4x4 &view, const QMatrix4x4 &projection){
  this->shader->bind();
  this->shader->setUniformValue("vColor", this->color);
  this->shader->setUniformValue("model", this->modelMatrix);
  this->shader->setUniformValue("view", view);
  this->shader->setUniformValue("projection", projection);
  this->vao.bind();
  glDrawArrays(this->primitiveType, 0, this->vertexNum);
  this->vao.release();
  this->shader->release();
  checkGLError("ColorPrimitive::draw");
}

void ColorPrimitive::initShaderAllocate(){
  this->vao.bind();
  this->vbo.bind();
  this->shader->bind();
  this->shader->enableAttributeArray(0);
  this->shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, this->stride * sizeof(GLfloat));
  this->shader->release();
  this->vbo.allocate(this->vertices, this->vertexNum * this->stride * sizeof(GLfloat));
  this->vbo.release();
  this->vao.release();
}

BasePlane::BasePlane(const QVector4D &color)
    : ColorPrimitive(GL_LINES, color) {
  logMessage("start constructing shader", Qgis::MessageLevel::Info);
  constructShader(QStringLiteral(":/schoolcore/shaders/line.vs"), QStringLiteral(":/schoolcore/shaders/line.fs"));
  const GLfloat size = DEFAULT_SIZE;
  const GLfloat step = DEFAULT_STEP;
  this->vertexNum = (2 * size / step + 1) * 2 * 2; // (x + y) * ((size - (-size)) / step + 1) * 2 points(stand for one line)
  this->vertices = new GLfloat[this->vertexNum * 3];
  double baseHeight = ws::FlightManager::getInstance().getBaseHeight();
  logMessage(QString("Base height: %1").arg(baseHeight), Qgis::MessageLevel::Info);
  GLuint index = 0;
  for (GLfloat x = -size; x <= size; x += step) {
    this->vertices[index++] = x;
    this->vertices[index++] = -size;
    this->vertices[index++] = baseHeight;
    this->vertices[index++] = x;
    this->vertices[index++] = size;
    this->vertices[index++] = baseHeight;
  }
  for (float y = -size; y <= size; y += step) {
    this->vertices[index++] = -size;
    this->vertices[index++] = y;
    this->vertices[index++] = baseHeight;
    this->vertices[index++] = size;
    this->vertices[index++] = y;
    this->vertices[index++] = baseHeight;
  }
  initShaderAllocate();
  logMessage("BasePlane initialized", Qgis::MessageLevel::Info);
}

RoutePath::RoutePath(const QVector<QVector3D>& vertices, const QVector4D& color)
    : ColorPrimitive(GL_LINE_STRIP, vertices, color) {
  logMessage("start constructing shader", Qgis::MessageLevel::Info);
  constructShader(QStringLiteral(":/schoolcore/shaders/line.vs"), QStringLiteral(":/schoolcore/shaders/line.fs"));
  initShaderAllocate();
  logMessage("RoutePath initialized", Qgis::MessageLevel::Info);
}

ControlPoints::ControlPoints(const QVector<QVector3D>& vertices, const QVector4D& color)
    : ColorPrimitive(GL_POINTS, vertices, color) {
  logMessage("start constructing shader", Qgis::MessageLevel::Info);
  constructShader(QStringLiteral(":/schoolcore/shaders/point.vs"), QStringLiteral(":/schoolcore/shaders/point.fs"));
  initShaderAllocate();
  logMessage("ControlPoints initialized", Qgis::MessageLevel::Info);
}

HomePoint::HomePoint(const QVector<QVector3D>& vertices, const QVector4D& color)
    : ColorPrimitive(GL_POINTS, vertices, color) {
  logMessage("start constructing shader", Qgis::MessageLevel::Info);
  constructShader(QStringLiteral(":/schoolcore/shaders/point.vs"), QStringLiteral(":/schoolcore/shaders/point.fs"));
  initShaderAllocate();
  logMessage("HomePoint initialized", Qgis::MessageLevel::Info);
}

  ConvexHull::ConvexHull(const QVector<QVector3D>& vertices, const QVector4D& color)
    : ColorPrimitive(GL_LINE_LOOP, vertices, color) {
  logMessage("start constructing shader", Qgis::MessageLevel::Info);
  constructShader(QStringLiteral(":/schoolcore/shaders/line.vs"), QStringLiteral(":/schoolcore/shaders/line.fs"));
  initShaderAllocate();
  logMessage("ConvexHull initialized", Qgis::MessageLevel::Info);
}

void Model::initModelData(){
  logMessage("start constructing shader", Qgis::MessageLevel::Info);
  constructShader(QStringLiteral(":/schoolcore/shaders/model.vs"), QStringLiteral(":/schoolcore/shaders/model.fs"));
  this->vao.bind();
  this->vbo.bind();
  this->vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
  this->shader->bind();
  this->shader->enableAttributeArray(0);
  this->shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, this->stride * sizeof(GLfloat));
  this->shader->enableAttributeArray(1);
  this->shader->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 2, this->stride * sizeof(GLfloat));
  logMessage("Model initialized", Qgis::MessageLevel::Info);

  this->vertexNum = modelData->totalVertices;
  GLuint count = this->vertexNum * this->stride;
  this->vertices = new GLfloat[count];
  GLuint index = 0;
  for (const auto& group : *modelData->materialGroups) {
    for (const auto& vertex : *group) {
      this->vertices[index++] = vertex.position.x();
      this->vertices[index++] = vertex.position.y();
      this->vertices[index++] = vertex.position.z();
      this->vertices[index++] = vertex.texCoord.x();
      this->vertices[index++] = vertex.texCoord.y();
    }
  }

  this->vbo.allocate(this->vertices, count * sizeof(GLfloat));
  this->shader->release();
  this->vbo.release();
  this->vao.release();
  checkGLError("Model::Model");
}

Model::Model(std::shared_ptr<ModelData> modelData)
    : Primitive(GL_TRIANGLES, 5), modelData(modelData) {
  initModelData();
}

void Model::draw(const QMatrix4x4 &view, const QMatrix4x4 &projection) {
    if (!shader) {
        logMessage("Shader is not set", Qgis::MessageLevel::Critical);
        return;
    }
    this->shader->bind();
    this->shader->setUniformValue("model", this->modelMatrix);
    this->shader->setUniformValue("view", view);
    this->shader->setUniformValue("projection", projection);

    // 绑定纹理
    if (modelData && modelData->textures) {
        int textureUnit = 0;
        for (auto it = modelData->textures->begin(); it != modelData->textures->end(); ++it) {
            if (it.value()) {
                it.value()->bind(textureUnit);
                this->shader->setUniformValue(QString("texture_%1").arg(it.key()).toStdString().c_str(), textureUnit);
                textureUnit++;
            }
        }
    }

    this->vao.bind();
    glDrawArrays(this->primitiveType, 0, this->vertexNum);
    this->vao.release();

    // 解绑纹理
    if (modelData && modelData->textures) {
        for (auto it = modelData->textures->begin(); it != modelData->textures->end(); ++it) {
            if (it.value()) {
                it.value()->release();
            }
        }
    }

    this->shader->release();
    checkGLError("Model::draw");
}

bool Primitive::constructShader(const QString& vertexShaderPath, const QString& fragmentShaderPath, const QString& geometryShaderPath) {
  // check if opengl context is current
  if (!QOpenGLContext::currentContext()) {
    logMessage("OpenGL context is not current", Qgis::MessageLevel::Critical);
    this->shader = nullptr;
    return false;
  }
  this->shader = std::make_shared<QOpenGLShaderProgram>();
  logMessage(QString("Constructing shader from %1 and %2").arg(vertexShaderPath).arg(fragmentShaderPath), Qgis::MessageLevel::Info);
  if (!shader->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                       vertexShaderPath)) {
    logMessage(QString("Shader Error:") + this->shader->log(),
               Qgis::MessageLevel::Critical);
    this->shader = nullptr;
    return false;
  }
  if (!geometryShaderPath.isEmpty()) {
    if (!shader->addShaderFromSourceFile(QOpenGLShader::Geometry,
                                         geometryShaderPath)) {
      logMessage(QString("Shader Error:") + this->shader->log(),
                 Qgis::MessageLevel::Critical);
      this->shader = nullptr;
      return false;
    }
  }
  if (!shader->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                       fragmentShaderPath)) {
    logMessage(QString("Shader Error:") + this->shader->log(),
               Qgis::MessageLevel::Critical);
    this->shader = nullptr;
    return false;
  }
  if (!shader->link()) {
    logMessage(QString("Shader Link Error:") + this->shader->log(),
               Qgis::MessageLevel::Critical);
    this->shader = nullptr;
    return false;
  }
  return true;
}

ModelData::ModelData(pMaterialVector materials, pTextureMap textures,
                     pMaterialGroupMap materialGroups, GLuint totalVertices)
    : materials(materials), textures(textures),
      materialGroups(materialGroups), totalVertices(totalVertices) {
  mBounds = calculateModelBounds();
  logMessage(QString("totalVertices: %1").arg(totalVertices), Qgis::MessageLevel::Info);
  logMessage(QString("ModelData initialized, bounds: [%1, %2, %3] - [%4, %5, %6]").arg(mBounds.min.x()).arg(mBounds.min.y()).arg(mBounds.min.z()).arg(mBounds.max.x()).arg(mBounds.max.y()).arg(mBounds.max.z()), Qgis::MessageLevel::Info);
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


qint64 ModelData::calcFaceNum(const QString &objFilePath) {
  QFile objFile(objFilePath);
  if (objFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qint64 fileSize = objFile.size();
    objFile.close();
    return fileSize;
  }
  return 0;
}

bool ModelData::displayProgress(qint64 progressUpdateInterval) {
  static QProgressDialog* progressDialog = nullptr;
  
  if (!progressDialog) {
    progressDialog = new QProgressDialog("正在加载模型...", "取消", 0, 100);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setMinimumDuration(0);
    progressDialog->setAutoClose(true);
    progressDialog->setAutoReset(true);
  }

  progressDialog->setValue(progressUpdateInterval);

  if (progressUpdateInterval >= 97) {
    progressDialog->close();
    delete progressDialog;
    progressDialog = nullptr;
  }

  if (progressDialog && progressDialog->wasCanceled()) {
    logMessage("User canceled model loading", Qgis::MessageLevel::Critical);
    progressDialog->close();
    delete progressDialog;
    progressDialog = nullptr;
    return true;
  }

  QCoreApplication::processEvents();
  return false;
}

std::pair<pMaterialGroupMap, GLuint> ModelData::loadMaterialGroups(const QString &objFilePath) {
  logMessage(QString("loadMaterialGroups: %1").arg(objFilePath), Qgis::MessageLevel::Info);
  using Vertex = ModelData::Vertex;
  pMaterialGroupMap materialGroups = std::make_shared<MaterialGroupMap>();
  QFile objFile(objFilePath);
  GLuint totalVertices = 0;
  bool isCanceled = false;
  if (objFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    logMessage("open obj file to read", Qgis::MessageLevel::Info);
    QTextStream in(&objFile);
    QVector<QVector3D> positions;
    QVector<QVector2D> texCoords;
    QString currentMaterial;
    const qint64 totalSize = objFile.size();
    qint64 processedBytes = 0;
    qint64 lastProgressUpdate = 0;
    const qint64 progressUpdateInterval = totalSize / 100;

    while (!in.atEnd()) {
      QString line = in.readLine().trimmed();
      processedBytes += line.length() + 1; // +1 for newline
      
      if (processedBytes - lastProgressUpdate >= progressUpdateInterval) {
        isCanceled = displayProgress(progressUpdateInterval);
        lastProgressUpdate = processedBytes;
      }
      
      if (isCanceled)
        return std::make_pair(nullptr, 0);

      QStringList parts = line.split(" ", Qt::SkipEmptyParts);
      if (parts.isEmpty())
        continue;

      if (parts[0] == "usemtl") {
        currentMaterial = parts[1];
        if (!materialGroups->contains(currentMaterial))
          (*materialGroups)[currentMaterial] = std::make_shared<MaterialGroup>();
      }
      else if (parts[0] == "v") {
        positions.append(QVector3D(parts[1].toFloat(), parts[2].toFloat(),
                                 parts[3].toFloat()));
      }
      else if (parts[0] == "vt") {
        texCoords.append(
            QVector2D(parts[1].toFloat(),
                     1.0f - parts[2].toFloat()));
      }
      else if (parts[0] == "f") {
        QVector<Vertex> faceVertices;
        for (int i = 1; i <= 3; ++i) {
          QStringList indices = parts[i].split("/", Qt::KeepEmptyParts);

          Vertex vertex;
          int posIndex = indices[0].toInt() - 1;
          if (posIndex >= 0 && posIndex < positions.size())
            vertex.position = positions[posIndex];

          if (indices.size() > 1 && !indices[1].isEmpty()) {
            int texIndex = indices[1].toInt() - 1;
            if (texIndex >= 0 && texIndex < texCoords.size()) {
              vertex.texCoord = texCoords[texIndex];
              vertex.texCoord.setY(1.0f - vertex.texCoord.y());
            }
          }
          (*materialGroups)[currentMaterial]->append(vertex);
          totalVertices++;
        }
      }
    }
    objFile.close();
  }
  logMessage("load material groups", Qgis::MessageLevel::Info);
  return std::make_pair(materialGroups, totalVertices);
}

std::shared_ptr<ModelData> ModelData::loadObjModel(
    const QString &objFilePath) {
  logMessage(QString("loadObjModel: %1").arg(objFilePath), Qgis::MessageLevel::Info);
  QString mtlPath = retriveMtlPath(objFilePath);
  logMessage(QString("mtlPath: %1").arg(mtlPath), Qgis::MessageLevel::Info);
  if (mtlPath.isEmpty()){
    logMessage("Mtl file not found", Qgis::MessageLevel::Critical);
    return nullptr;
  }

  TexturePair mtlResource = loadMtl(mtlPath);
  auto materialGroupPair = loadMaterialGroups(objFilePath);

  return std::make_shared<ModelData>(mtlResource.first, mtlResource.second,
                                     materialGroupPair.first, materialGroupPair.second);
}

TexturePair ModelData::loadMtl(const QString &mtlPath) {
  using Material = ModelData::Material;
  QFile file(mtlPath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
    logMessage(QString("Failed to open mtl file: %1").arg(mtlPath), Qgis::MessageLevel::Critical);
    return std::make_pair(nullptr, nullptr);
  }
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
      logMessage(QString("newmtl: %1").arg(currentMaterial->name), Qgis::MessageLevel::Info);
    } else if (parts[0] == "map_Kd") {
      QString texPath = QFileInfo(mtlPath).absolutePath() + "/" + parts[1];
      logMessage(QString("texPath: %1").arg(texPath), Qgis::MessageLevel::Info);
      if (!textures->contains(currentMaterial->name)) {
        QImage img(texPath);
        if (!img.isNull()) {
          std::shared_ptr<QOpenGLTexture> texture =
              std::make_shared<QOpenGLTexture>(img.mirrored());
          texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
          texture->setMagnificationFilter(QOpenGLTexture::Linear);
          textures->insert(currentMaterial->name, texture);
          logMessage("insert texture", Qgis::MessageLevel::Info);
        }
      }
    }
  }
  if (!currentMaterial->name.isEmpty()) {
    materials->append(currentMaterial);
    logMessage(QString("append material: %1").arg(currentMaterial->name), Qgis::MessageLevel::Info);
  }
  logMessage("load mtl", Qgis::MessageLevel::Info);
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

Model::Model(const QString& objFilePath):Primitive(GL_TRIANGLES, 5){
  loadModel(objFilePath);
  logMessage("Model constructed", Qgis::MessageLevel::Success);
  initModelData();
}

void Model::loadModel(const QString& objFilePath){
  modelData = ModelData::loadObjModel(objFilePath);
  if (!modelData){
    logMessage("Failed to load model", Qgis::MessageLevel::Critical);
    return;
  }
  logMessage("Model data loaded", Qgis::MessageLevel::Success);
}

Demo::Demo():ColorPrimitive(GL_TRIANGLES,  QVector4D(1.0f, 0.0f, 0.0f, 1.0f)){
  
  this->vertexNum = 3;
  this->vertices = new GLfloat[this->vertexNum * this->stride];
  this->vertices[0] = -0.5f;
  this->vertices[1] = -0.5f;
  this->vertices[2] = 0.0f;
  this->vertices[3] = 0.5f;
  this->vertices[4] = -0.5f;
  this->vertices[5] = 0.0f;
  this->vertices[6] = 0.0f;
  this->vertices[7] = 0.5f;
  this->vertices[8] = 0.0f;
  
  this->vao.bind();
  this->vbo.bind();
  this->vbo.allocate(this->vertices, this->vertexNum * this->stride * sizeof(GLfloat));
  constructShader(QStringLiteral(":/schoolcore/shaders/line.vs"), QStringLiteral(":/schoolcore/shaders/line.fs"));
  if (!shader) {
    logMessage("Shader is not set", Qgis::MessageLevel::Critical);
    return;
  }
  this->shader->bind();
  this->shader->enableAttributeArray(0);
  this->shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, this->stride * sizeof(GLfloat));
  this->shader->release();
  this->vbo.release();
  this->vao.release();
}
}