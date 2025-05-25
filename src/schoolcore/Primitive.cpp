#include "Primitive.h"
#include "qgis_debug.h"

namespace gl{
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
Primitive::Primitive(GLenum primitiveType, GLfloat* vertices, GLuint vertexNum){
    this->primitiveType = primitiveType;
    this->vertices = vertices;
    this->vertexNum = vertexNum;
    this->vao.create();
    this->vbo.create();
    this->modelMatrix.setToIdentity();
    shader = nullptr;
    this->vbo.bind();
    this->vbo.allocate(this->vertices, this->vertexNum * sizeof(GLfloat));
    this->vbo.release();
    }
    Primitive::~Primitive(){
    this->vao.destroy();
    this->vbo.destroy();
    shader = nullptr;
    delete[] this->vertices;
    }
    void Primitive::setModelMatrix(const QMatrix4x4 &matrix){
    this->modelMatrix = matrix;
}
ColorPrimitive::ColorPrimitive(GLenum primitiveType, GLfloat* vertices, GLuint vertexNum, const QVector4D& color):
Primitive(primitiveType, vertices, vertexNum),color(color){}
void ColorPrimitive::draw(){
    this->shader->bind();
    this->shader->setUniformValue("modelMatrix", this->modelMatrix);
    this->shader->setUniformValue("vColor", this->color);
    this->vao.bind();
    this->vbo.bind();
    glDrawArrays(this->primitiveType, 0, this->vertexNum * stride);
    this->vbo.release();
    this->vao.release();
    this->shader->release();
}
BasePlane::BasePlane(GLfloat* vertices, GLuint vertexNum, const QVector4D& color):
ColorPrimitive(GL_LINES, vertices, vertexNum, color){}
ControlPoints::ControlPoints(GLfloat* vertices, GLuint vertexNum, const QVector4D& color):
ColorPrimitive(GL_POINTS, vertices, vertexNum, color){}
Model::Model(GLfloat* vertices, GLuint vertexNum, std::shared_ptr<QOpenGLTexture> texture):
Primitive(GL_TRIANGLES, vertices, vertexNum), texture(texture){}
void Model::draw(){
    this->shader->bind();
    this->shader->setUniformValue("modelMatrix", this->modelMatrix);
    this->shader->setUniformValue("textureSampler", 0);
    this->texture->bind();
    this->vao.bind();
    this->vbo.bind();
    glDrawArrays(this->primitiveType, 0, this->vertexNum * stride);
    this->vbo.release();
    this->vao.release();
    this->shader->release();
    this->texture->release();
}

std::shared_ptr<QOpenGLShaderProgram> constructShader(const QString& vertexShaderPath, const QString& fragmentShaderPath, const QString& geometryShaderPath) {
    std::shared_ptr<QOpenGLShaderProgram> shader = std::make_shared<QOpenGLShaderProgram>();
    if (!shader->addShaderFromSourceFile(QOpenGLShader::Vertex, vertexShaderPath)) {
        logMessage(QString("Shader Error:") + shader->log(), Qgis::MessageLevel::Critical);
        return nullptr;
    }
    if (!geometryShaderPath.isEmpty()) {
        if (!shader->addShaderFromSourceFile(QOpenGLShader::Geometry, geometryShaderPath)) {
            logMessage(QString("Shader Error:") + shader->log(), Qgis::MessageLevel::Critical);
            return nullptr;
        }
    }
    if (!shader->addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentShaderPath)) {
        logMessage(QString("Shader Error:") + shader->log(), Qgis::MessageLevel::Critical);
        return nullptr;
    }
    if (!shader->link()) {
        logMessage(QString("Shader Link Error:") + shader->log(), Qgis::MessageLevel::Critical);
    }
    shader->setUniformValue("projection", ws::WorkspaceState::getInstance().getProjection());
    return shader;
}
ModelData::ModelData(pMaterialVector materials, pTextureMap textures, pMaterialGroupMap materialGroups):
    materials(materials), textures(textures), materialGroups(materialGroups){
        calculateModelBounds(this, mBounds);
    }
ModelData::~ModelData(){
    this->materials->clear();
    this->textures->clear();
    this->materialGroups->clear();
    this->materials = nullptr;
    this->textures = nullptr;
    this->materialGroups = nullptr;
}

QString ModelData::retriveMtlPath(const QString &objfilePath){
  QFile objFile(objfilePath);
  QString mtlPath;
  if (objFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&objFile);
    while (!in.atEnd()) {
      QString line = in.readLine().trimmed();
      if (line.startsWith("mtllib")) {
        mtlPath = QFileInfo(objfilePath).absolutePath() + "/" + line.split(" ")[1];
        break;
      }
    }
    objFile.close();
  }
  return mtlPath;
}
pMaterialGroupMap ModelData::loadMaterialGroups(const QString &filePath){
  using Vertex = ModelData::Vertex;
  pMaterialGroupMap materialGroups = std::make_shared<MaterialGroupMap>();
  QFile objFile(filePath);
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
        else if (parts[0] == "v")//point
            positions.append(QVector3D(parts[1].toFloat(), parts[2].toFloat(),parts[3].toFloat()));
        else if (parts[0] == "vt")// texture
            texCoords.append(QVector2D(parts[1].toFloat(),1.0f - parts[2].toFloat())); // flip y for opengl
        else if (parts[0] == "f") { // face
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
                    vertex.texCoord.setY(1.0f - vertex.texCoord.y()); // OpenGL纹理坐标需要翻转Y轴
                }
            }

            materialGroups[currentMaterial]->append(vertex);
        }
      }
    }
    objFile.close();
  }

  return materialGroups;
}
std::shared_ptr<ModelData> ModelData::loadObjModel(const QString &objFilePath) {
    QString mtlPath = retriveMtlPath(objFilePath);
    if (mtlPath.isEmpty())
        return nullptr;
    TexturePair mtlResource = loadMtl(mtlPath);
    pMaterialGroupMap materialGroups = loadMaterialGroups(objFilePath);

    return std::make_shared<ModelData>(mtlResource.first, mtlResource.second, materialGroups);
}

TexturePair ModelData::loadMtl(const QString &mtlPath) {
  using Material = ModelData::Material;
  QFile file(mtlPath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return std::make_pair(nullptr, nullptr);
  MaterialVector materials = std::make_shared<MaterialVector>();
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
          std::shared_ptr<QOpenGLTexture> texture = std::make_shared<QOpenGLTexture>(img.mirrored());
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

// 计算单个模型包围盒
void ModelData::calculateModelBounds(ModelData *modelData,
                                          ModelBounds &bounds) {
  bounds.min = QVector3D(FLT_MAX, FLT_MAX, FLT_MAX);
  bounds.max = QVector3D(-FLT_MAX, -FLT_MAX, -FLT_MAX);

  for (const auto &group : modelData->materialGroups) {
    for (const Vertex &v : group) {
      bounds.min.setX(qMin(bounds.min.x(), v.position.x()));
      bounds.min.setY(qMin(bounds.min.y(), v.position.y()));
      bounds.min.setZ(qMin(bounds.min.z(), v.position.z()));

      bounds.max.setX(qMax(bounds.max.x(), v.position.x()));
      bounds.max.setY(qMax(bounds.max.y(), v.position.y()));
      bounds.max.setZ(qMax(bounds.max.z(), v.position.z()));
    }
  }

  bounds.originalCenter = QVector3D((bounds.min.x() + bounds.max.x()) / 2.0f,
                                    (bounds.min.y() + bounds.max.y()) / 2.0f,
                                    (bounds.min.z() + bounds.max.z()) / 2.0f);
}

// 更新全局包围盒
void ModelData::updateGlobalBounds(const ModelBounds &modelBounds) {
  m_globalBounds.sceneMin.setX(
      qMin(m_globalBounds.sceneMin.x(), modelBounds.min.x()));
  m_globalBounds.sceneMin.setY(
      qMin(m_globalBounds.sceneMin.y(), modelBounds.min.y()));
  m_globalBounds.sceneMin.setZ(
      qMin(m_globalBounds.sceneMin.z(), modelBounds.min.z()));

  m_globalBounds.sceneMax.setX(
      qMax(m_globalBounds.sceneMax.x(), modelBounds.max.x()));
  m_globalBounds.sceneMax.setY(
      qMax(m_globalBounds.sceneMax.y(), modelBounds.max.y()));
  m_globalBounds.sceneMax.setZ(
      qMax(m_globalBounds.sceneMax.z(), modelBounds.max.z()));

  m_globalBounds.sceneCenter = QVector3D(
      (m_globalBounds.sceneMin.x() + m_globalBounds.sceneMax.x()) / 2.0f,
      (m_globalBounds.sceneMin.y() + m_globalBounds.sceneMax.y()) / 2.0f,
      (m_globalBounds.sceneMin.z() + m_globalBounds.sceneMax.z()) / 2.0f);
  qDebug() << "x"
           << (m_globalBounds.sceneMin.x() + m_globalBounds.sceneMax.x()) / 2.0f
           << "y"
           << (m_globalBounds.sceneMin.y() + m_globalBounds.sceneMax.y()) / 2.0f
           << "z"
           << (m_globalBounds.sceneMin.z() + m_globalBounds.sceneMax.z()) /
                  2.0f;
}

void ModelData::applyGlobalCentering() {
  makeCurrent();

  // 重置全局包围盒
  m_globalBounds = GlobalBounds();

  // 重新计算所有模型的全局包围盒
  for (ModelData *model : m_models) {
    ModelBounds modelBounds;
    calculateModelBounds(model, modelBounds);
    updateGlobalBounds(modelBounds);
  }

  // 计算全局中心
  QVector3D globalCenter = m_globalBounds.sceneCenter;
  qDebug() << "x:" << m_globalBounds.sceneCenter.x()
           << ",y:" << m_globalBounds.sceneCenter.y()
           << ",z:" << m_globalBounds.sceneCenter.z();
  // 调整每个模型的顶点位置
  for (ModelData *model : m_models) {
    ModelBounds &bounds = m_modelBoundsMap[model];
    QVector3D modelOffset = globalCenter;

    // 修改顶点数据
    for (auto &group : model->materialGroups) {
      for (Vertex &v : group) {
        v.position -= modelOffset;
      }
    }

    // 更新VBO
    model->vao.bind();
    model->vbo.bind();
    int offset = 0;
    for (auto it = model->materialGroups.begin();
         it != model->materialGroups.end(); ++it) {
      const QVector<Vertex> &vertices = it.value();
      model->vbo.write(offset, vertices.constData(),
                       vertices.size() * sizeof(Vertex));
      offset += vertices.size() * sizeof(Vertex);
    }
    model->vbo.release();
    model->vao.release();
  }

  doneCurrent();
  update();
  qDebug() << "omggggggggggggggg";
}

QVector3D ModelData::calculateModelCenter() {
  float minX = FLT_MAX, maxX = -FLT_MAX;
  float minY = FLT_MAX, maxY = -FLT_MAX;
  float minZ = FLT_MAX, maxZ = -FLT_MAX;

  for (const auto &group : *materialGroups) {
    for (const Vertex &v : group) {
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
/*
    mShaderProgram.setAttributeBuffer(0, GL_FLOAT, offsetof(Vertex, position), 3, sizeof(Vertex));

    mShaderProgram.enableAttributeArray(1);
    mShaderProgram.setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, texCoord), 2, sizeof(Vertex));
*/