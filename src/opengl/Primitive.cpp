#include "Primitive.h"
#include "../log/QgisDebug.h"
#include "../core/WorkspaceState.h"
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
  if (!QOpenGLContext::currentContext()) {
    logMessage("Primitive::Primitive: OpenGL context is not current", Qgis::MessageLevel::Critical);
    return;
  }
  this->primitiveType = primitiveType;
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
        QString errorMsg;
        switch (err) {
            case GL_INVALID_ENUM:
                errorMsg = "GL_INVALID_ENUM: 枚举参数不合法";
                break;
            case GL_INVALID_VALUE:
                errorMsg = "GL_INVALID_VALUE: 值参数不合法";
                break;
            case GL_INVALID_OPERATION:
                errorMsg = "GL_INVALID_OPERATION: 当前状态下的操作不合法";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                errorMsg = "GL_INVALID_FRAMEBUFFER_OPERATION: 帧缓冲操作不合法";
                break;
            case GL_OUT_OF_MEMORY:
                errorMsg = "GL_OUT_OF_MEMORY: 内存不足";
                break;
            case GL_STACK_UNDERFLOW:
                errorMsg = "GL_STACK_UNDERFLOW: 栈下溢";
                break;
            case GL_STACK_OVERFLOW:
                errorMsg = "GL_STACK_OVERFLOW: 栈上溢";
                break;
            default:
                errorMsg = QString("未知错误: 0x%1").arg(err, 0, 16);
        }

        // 获取当前OpenGL状态信息
        QString stateInfo;
        if (QOpenGLContext::currentContext()) {
            GLint program;
            glGetIntegerv(GL_CURRENT_PROGRAM, &program);
            stateInfo += QString("\n当前着色器程序: %1").arg(program);

            GLint vao;
            glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
            stateInfo += QString("\n当前VAO: %1").arg(vao);

            GLint vbo;
            glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &vbo);
            stateInfo += QString("\n当前VBO: %1").arg(vbo);

            GLint texture;
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture);
            stateInfo += QString("\n当前纹理: %1").arg(texture);
        }

        // 输出详细的错误信息
        logMessage(QString("OpenGL错误在 %1:\n错误类型: %2\n错误描述: %3%4")
                  .arg(funcName)
                  .arg(err)
                  .arg(errorMsg)
                  .arg(stateInfo),
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
  if (!QOpenGLContext::currentContext()) {
    logMessage("draw: OpenGL context is not current", Qgis::MessageLevel::Critical);
    return;
  }
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
  vao.create();
  vbo.create();
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
  logMessage(QString("VAO is created: %1, VAO id: %2")
    .arg(this->vao.isCreated())
    .arg(this->vao.objectId()));
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
  if (!QOpenGLContext::currentContext()) {
    logMessage("initModelData: OpenGL context is not current", Qgis::MessageLevel::Critical);
    return;
  }
  logMessage("start constructing shader", Qgis::MessageLevel::Info);
  constructShader(QStringLiteral(":/schoolcore/shaders/model.vs"), QStringLiteral(":/schoolcore/shaders/model.fs"));
  this->vao.create();
  this->vbo.create();
  this->vao.bind();
  this->vbo.bind();
  this->vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
  logMessage("stride is " + QString::number(this->stride), Qgis::MessageLevel::Info);

  this->vertexNum = modelData->totalVertices;
  logMessage("vertexNum: " + QString::number(this->vertexNum), Qgis::MessageLevel::Info);
  GLuint count = this->vertexNum * this->stride;
  logMessage("count is " + QString::number(count), Qgis::MessageLevel::Info);
  this->vertices = new GLfloat[count];
  GLuint index = 0;
  for (const auto& group : *modelData->materialGroups) {
    logMessage(QString("group size: %1").arg(group->size()), Qgis::MessageLevel::Info);
    for (const auto& vertex : *group) {
      this->vertices[index++] = vertex.position.x();
      this->vertices[index++] = vertex.position.y();
      this->vertices[index++] = vertex.position.z();
      this->vertices[index++] = vertex.texCoord.x();
      this->vertices[index++] = vertex.texCoord.y();
    }
  }
  this->vbo.allocate(this->vertices, count * sizeof(GLfloat));
  
  this->shader->bind();
  this->shader->enableAttributeArray(0);
  this->shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, this->stride * sizeof(GLfloat));
  this->shader->enableAttributeArray(1);
  this->shader->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 2, this->stride * sizeof(GLfloat));
  logMessage("Model initialized", Qgis::MessageLevel::Info);
  this->vbo.release();
  this->vao.release();
  this->shader->release();
  checkGLError("Model::Model");
  logMessage("Texture Size: " + QString::number(modelData->textures->size()), Qgis::MessageLevel::Info);
  logMessage("Material Size: " + QString::number(modelData->materialGroups->size()), Qgis::MessageLevel::Info);
  logMessage(QString("VAO is created: %1, VAO id: %2")
    .arg(this->vao.isCreated())
    .arg(this->vao.objectId()));
}

Model::Model(std::shared_ptr<ModelData> modelData)
    : Primitive(GL_TRIANGLES, 5), modelData(modelData) {
  initModelData();
}

void Model::draw(const QMatrix4x4 &view, const QMatrix4x4 &projection) {
    if (!shader || !shader->isLinked()) {
        logMessage("Shader program is not valid or not linked", Qgis::MessageLevel::Critical);
        return;
    }
    if (this->vertexNum == 0 || this->vertices == nullptr) {
        logMessage("Invalid vertex data", Qgis::MessageLevel::Critical);
        return;
    }
    this->shader->bind();
    checkGLError("Model::draw - after shader bind");
    this->shader->setUniformValue("model", this->modelMatrix);
    this->shader->setUniformValue("view", view);
    this->shader->setUniformValue("projection", projection);
    checkGLError("Model::draw - after setting uniforms");
    if (modelData && modelData->textures && !modelData->textures->isEmpty()) {
        auto firstTexture = modelData->textures->first();
        if (firstTexture && firstTexture->isCreated()) {
            firstTexture->bind();
            checkGLError("Model::draw - after texture bind");
            this->shader->setUniformValue("textureSampler", firstTexture->textureId());
            this->vao.bind();
            checkGLError("Model::draw - after VAO bind");
            glDrawArrays(this->primitiveType, 0, this->vertexNum);
            checkGLError("Model::draw - after glDrawArrays");
            this->vao.release();
            firstTexture->release();
        }
    }

    if (this->vertexNum == 0 || this->vertices == nullptr) {
        logMessage("Invalid vertex data", Qgis::MessageLevel::Critical);
        return;
    }
    this->shader->release();
}

bool Primitive::constructShader(const QString& vertexShaderPath, const QString& fragmentShaderPath, const QString& geometryShaderPath) {
  // check if opengl context is current
  if (!QOpenGLContext::currentContext()) {
    logMessage("constructShader: OpenGL context is not current", Qgis::MessageLevel::Critical);
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
  if (!QOpenGLContext::currentContext()) {
    logMessage("Model::Model: OpenGL context is not current", Qgis::MessageLevel::Critical);
    return;
  }
  modelData = std::make_shared<ModelData>(objFilePath);
  logMessage("Model constructed", Qgis::MessageLevel::Success);
  initModelData();
}

ModelData::ModelData(const QString &objFilePath){
  if (!QOpenGLContext::currentContext()) {
    logMessage("loadObjModel: OpenGL context is not current", Qgis::MessageLevel::Critical);
    return;
  }
  logMessage(QString("loadObjModel: %1").arg(objFilePath), Qgis::MessageLevel::Info);
  QString mtlPath = ModelDataLoader::retriveMtlPath(objFilePath);
  logMessage(QString("mtlPath: %1").arg(mtlPath), Qgis::MessageLevel::Info);
  if (mtlPath.isEmpty()){
    logMessage("Mtl file not found", Qgis::MessageLevel::Critical);
  }

  TexturePair mtlResource = ModelDataLoader::loadMtl(mtlPath);
  auto materialGroupPair = ModelDataLoader::loadMaterialGroups(objFilePath);
  materialGroups = materialGroupPair.first;
  totalVertices = materialGroupPair.second;
  textures = mtlResource.second;
  materials = mtlResource.first;
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

void gl::Model::cleanupTextures() {
  if (!modelData || !modelData->textures) {
    return;
  }

  // 在正确的上下文中清理纹理
  for (auto it = modelData->textures->begin(); it != modelData->textures->end(); ++it) {
    if (it.value()) {
      it.value()->destroy();
    }
  }
  modelData->textures->clear();
}
}

gl::ModelData::TexturePair ModelDataLoader::loadMtl(const QString &mtlPath) {
  if (!QOpenGLContext::currentContext()) {
    logMessage("loadMtl: OpenGL context is not current", Qgis::MessageLevel::Critical);
    return std::make_pair(nullptr, nullptr);
  }
  using Material = gl::ModelData::Material;
  QFile file(mtlPath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
    logMessage(QString("Failed to open mtl file: %1").arg(mtlPath), Qgis::MessageLevel::Critical);
    return std::make_pair(nullptr, nullptr);
  }
  gl::ModelData::pMaterialVector materials = std::make_shared<gl::MaterialVector>();
  gl::ModelData::pTextureMap textures = std::make_shared<gl::TextureMap>();
  gl::ModelData::pMaterial currentMaterial = std::make_shared<gl::Material>();
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

QString ModelDataLoader::retriveMtlPath(const QString &objfilePath) {
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

std::pair<gl::ModelData::pMaterialGroupMap, GLuint> ModelDataLoader::loadMaterialGroups(const QString &objFilePath) {
  logMessage(QString("loadMaterialGroups: %1").arg(objFilePath), Qgis::MessageLevel::Info);
  using Vertex = gl::ModelData::Vertex;
  gl::ModelData::pMaterialGroupMap materialGroups = std::make_shared<gl::MaterialGroupMap>();
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
          (*materialGroups)[currentMaterial] = std::make_shared<gl::MaterialGroup>();
      }
      else if (parts[0] == "v") {
        positions.append(QVector3D(parts[1].toFloat(), parts[2].toFloat(),
                                 parts[3].toFloat()));
      }
      else if (parts[0] == "vt") {
        texCoords.append(
            QVector2D(parts[1].toFloat(),
                     parts[2].toFloat()));
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


qint64 ModelDataLoader::calcFaceNum(const QString &objFilePath) {
  QFile objFile(objFilePath);
  if (objFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qint64 fileSize = objFile.size();
    objFile.close();
    return fileSize;
  }
  return 0;
}

bool ModelDataLoader::displayProgress(qint64 progressUpdateInterval) {
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