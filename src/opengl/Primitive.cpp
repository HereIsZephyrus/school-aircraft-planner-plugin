#include "Primitive.h"
#include "Camera.h"
#include "../log/QgisDebug.h"
#include "../core/WorkspaceState.h"
#include <GL/gl.h>
#include <QOpenGLContext>
#include <QProgressDialog>
#include <QCoreApplication>
#include <memory>
#include <qmatrix4x4.h>
#include <qvector3d.h>
#include <qvector4d.h>
namespace gl {
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
  //logMessage("Primitive created", Qgis::MessageLevel::Info);
  this->vao.bind();
  this->vbo.bind();
  this->vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
  this->vbo.allocate(this->vertices,
                     this->vertexNum * this->stride * sizeof(GLfloat));
  this->vbo.release();
  this->vao.release();
  //logMessage("Primitive data initialized", Qgis::MessageLevel::Info);
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
  //logMessage("ready to destroy Primitive", Qgis::MessageLevel::Info);
  if (QOpenGLContext::currentContext()) {
    this->vao.destroy();
    this->vbo.destroy();
    this->shader = nullptr;
    delete[] this->vertices;
  }
  //logMessage("Primitive destroyed", Qgis::MessageLevel::Success);
}

void Primitive::checkGLError(const QString &funcName) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        QString errorMsg;
        switch (err) {
            case GL_INVALID_ENUM:
                errorMsg = "GL_INVALID_ENUM: Invalid enum parameter";
                break;
            case GL_INVALID_VALUE:
                errorMsg = "GL_INVALID_VALUE: Invalid value parameter";
                break;
            case GL_INVALID_OPERATION:
                errorMsg = "GL_INVALID_OPERATION: Invalid operation in current state";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                errorMsg = "GL_INVALID_FRAMEBUFFER_OPERATION: Invalid framebuffer operation";
                break;
            case GL_OUT_OF_MEMORY:
                errorMsg = "GL_OUT_OF_MEMORY: Out of memory";
                break;
            case GL_STACK_UNDERFLOW:
                errorMsg = "GL_STACK_UNDERFLOW: Stack underflow";
                break;
            case GL_STACK_OVERFLOW:
                errorMsg = "GL_STACK_OVERFLOW: Stack overflow";
                break;
            default:
                errorMsg = QString("Unknown error: 0x%1").arg(err, 0, 16);
        }

        QString stateInfo;
        if (QOpenGLContext::currentContext()) {
            GLint program;
            glGetIntegerv(GL_CURRENT_PROGRAM, &program);
            stateInfo += QString("\nCurrent shader program: %1").arg(program);

            GLint vao;
            glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
            stateInfo += QString("\nCurrent VAO: %1").arg(vao);

            GLint vbo;
            glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &vbo);
            stateInfo += QString("\nCurrent VBO: %1").arg(vbo);

            GLint texture;
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture);
            stateInfo += QString("\nCurrent texture: %1").arg(texture);
        }
        logMessage(QString("OpenGL error in %1:\nError type: %2\nError description: %3%4")
                  .arg(funcName)
                  .arg(err)
                  .arg(errorMsg)
                  .arg(stateInfo),
                  Qgis::MessageLevel::Critical);
    }
}

void Primitive::setModelMatrix(const QMatrix4x4 &matrix) {
  this->modelMatrix = matrix;
  this->shader->bind();
  this->shader->setUniformValue("model",this->modelMatrix);
  this->shader->release();
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
  //this->shader->setUniformValue("model", this->modelMatrix);
  this->shader->setUniformValue("view", view);
  this->shader->setUniformValue("projection", projection);
  this->vao.bind();
  glDrawArrays(this->primitiveType, 0, this->vertexNum);
  this->vao.release();
  this->shader->release();
  checkGLError("ColorPrimitive::draw");
}

void ColorPrimitive::initShaderAllocate(){
  if (!this->vao.isCreated())
    this->vao.create();
  if (!this->vbo.isCreated())
    this->vbo.create();
  this->vao.bind();
  this->vbo.bind();
  this->vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
  this->shader->bind();
  this->shader->enableAttributeArray(0);
  this->shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, this->stride * sizeof(GLfloat));
  this->shader->setUniformValue("model", this->modelMatrix);
  this->shader->release();
  this->vbo.allocate(this->vertices, this->vertexNum * this->stride * sizeof(GLfloat));
  this->vbo.release();
  this->vao.release();
}

BasePlane::BasePlane(Bounds bounds, double baseHeight, const QVector4D &color) : ColorPrimitive(GL_LINES, color) {
  logMessage(QString("BasePlane bounds: %1, %2, %3, %4").arg(bounds.min.x()).arg(bounds.min.y()).arg(bounds.max.x()).arg(bounds.max.y()), Qgis::MessageLevel::Info);
  logMessage(QString("BasePlane baseHeight: %1").arg(baseHeight), Qgis::MessageLevel::Info);
  //logMessage("start constructing shader", Qgis::MessageLevel::Info);
  constructShader(QStringLiteral(":/schoolcore/shaders/line.vs"), QStringLiteral(":/schoolcore/shaders/line.fs"));
  const GLfloat step = DEFAULT_STEP;
  int xNum = (bounds.max.x() - bounds.min.x()) / step + 1;
  int yNum = (bounds.max.y() - bounds.min.y()) / step + 1;
  this->vertexNum = 2 * (xNum + yNum); // (x + y) * ((size - (-size)) / step + 1) * 2 points(stand for one line)
  this->vertices = new GLfloat[this->vertexNum * 3];
  //logMessage(QString("BasePlane vertices: %1").arg(this->vertexNum), Qgis::MessageLevel::Info);
  GLuint index = 0;
  for (GLfloat x = bounds.min.x(); x <= bounds.max.x(); x += step) {
    this->vertices[index++] = x;
    this->vertices[index++] = bounds.min.y();
    this->vertices[index++] = baseHeight;
    this->vertices[index++] = x;
    this->vertices[index++] = bounds.max.y();
    this->vertices[index++] = baseHeight;
  }
  for (float y = bounds.min.y(); y <= bounds.max.y(); y += step) {
    this->vertices[index++] = bounds.min.x();
    this->vertices[index++] = y;
    this->vertices[index++] = baseHeight;
    this->vertices[index++] = bounds.max.x();
    this->vertices[index++] = y;
    this->vertices[index++] = baseHeight;
  }
  initShaderAllocate();
  logMessage("BasePlane initialized", Qgis::MessageLevel::Info);
}

RoutePath::RoutePath(const QVector<QVector3D>& vertices, const QVector4D& color)
    : ColorPrimitive(GL_LINE_STRIP, vertices, color), routePath(vertices) {
  //logMessage("start constructing shader", Qgis::MessageLevel::Info);
  constructShader(QStringLiteral(":/schoolcore/shaders/line.vs"), QStringLiteral(":/schoolcore/shaders/line.fs"));
  initShaderAllocate();
  //logMessage("RoutePath initialized", Qgis::MessageLevel::Info);
}

OrientLine::OrientLine(const QVector<QVector3D>& vertices, const QVector4D& color)
    : ColorPrimitive(GL_LINES, vertices, color) {
  //logMessage("start constructing shader", Qgis::MessageLevel::Info);
  constructShader(QStringLiteral(":/schoolcore/shaders/line.vs"), QStringLiteral(":/schoolcore/shaders/line.fs"));
  initShaderAllocate();
  //logMessage("OrientLine initialized", Qgis::MessageLevel::Info);
}

ControlPoints::ControlPoints(const QVector<QVector3D>& vertices, const QVector4D& color)
    : ColorPrimitive(GL_POINTS, vertices, color) {
  //logMessage("start constructing shader", Qgis::MessageLevel::Info);
  constructShader(QStringLiteral(":/schoolcore/shaders/line.vs"), QStringLiteral(":/schoolcore/shaders/line.fs"));
  initShaderAllocate();
  //logMessage(QString("control Pointsize is %1").arg(vertexNum), Qgis::MessageLevel::Info);
  //logMessage("ControlPoints initialized", Qgis::MessageLevel::Info);
}

SinglePoint::SinglePoint(const QVector3D& vertices, const QVector4D& color)
    : ColorPrimitive(GL_POINTS, QVector<QVector3D>{vertices}, color), point(vertices) {
  //logMessage("start constructing shader", Qgis::MessageLevel::Info);
  //constructShader(QStringLiteral(":/schoolcore/shaders/point.vs"), QStringLiteral(":/schoolcore/shaders/point.fs"), QStringLiteral(":/schoolcore/shaders/point.gs"));
  constructShader(QStringLiteral(":/schoolcore/shaders/line.vs"), QStringLiteral(":/schoolcore/shaders/line.fs"));
  initShaderAllocate();
  //logMessage("SinglePoint initialized", Qgis::MessageLevel::Info);
}

  ConvexHull::ConvexHull(const QVector<QVector3D>& vertices, const QVector4D& color)
    : ColorPrimitive(GL_LINE_LOOP, vertices, color) {
  //logMessage("start constructing shader", Qgis::MessageLevel::Info);
  constructShader(QStringLiteral(":/schoolcore/shaders/line.vs"), QStringLiteral(":/schoolcore/shaders/line.fs"));
  initShaderAllocate();
  //logMessage("ConvexHull initialized", Qgis::MessageLevel::Info);
}

void ModelGroup::initModelData(){
  if (!QOpenGLContext::currentContext()) {
    logMessage("initModelData: OpenGL context is not current", Qgis::MessageLevel::Critical);
    return;
  }
  if (this->vao.isCreated()) {
    logMessage("VAO is already created", Qgis::MessageLevel::Warning);
  } else {
    logMessage("VAO is not created, creating now", Qgis::MessageLevel::Info);
    this->vao.create();
    if (!this->vao.isCreated()) {
      logMessage("Failed to create VAO", Qgis::MessageLevel::Critical);
      return;
    }
  }
  this->vbo.create();
  this->vao.bind();
  this->vbo.bind();
  this->vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);

  for (const auto& model : models) {
    generateTexture(model->texturePath);
    GLuint modelVertexNum = model->vertices.size();
    this->vertexNum += modelVertexNum;
    verticesRange.append(modelVertexNum);
  }
  logMessage("vertexNum: " + QString::number(this->vertexNum), Qgis::MessageLevel::Info);
  GLuint count = this->vertexNum * this->stride;
  this->vertices = new GLfloat[count];
  GLuint index = 0;
  for (const auto& model : models) {
    for (const auto& vertex : model->vertices) {
      this->vertices[index++] = vertex.position.x();
      this->vertices[index++] = vertex.position.y();
      this->vertices[index++] = vertex.position.z();
      this->vertices[index++] = vertex.texCoord.x();
      this->vertices[index++] = vertex.texCoord.y();
    }
  }
  this->vbo.allocate(this->vertices, count * sizeof(GLfloat));
  //logMessage("start constructing shader", Qgis::MessageLevel::Info);
  constructShader(QStringLiteral(":/schoolcore/shaders/model.vs"), QStringLiteral(":/schoolcore/shaders/model.fs"));
  this->shader->bind();
  this->shader->enableAttributeArray(0);
  this->shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, this->stride * sizeof(GLfloat));
  this->shader->enableAttributeArray(1);
  this->shader->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 2, this->stride * sizeof(GLfloat));
  this->shader->setUniformValue("model", this->modelMatrix);
  this->shader->release();
  /*
  int groupNum = (models.size()-1) / 6 + 1;
  for (int i = 0; i < groupNum; i++) {
    auto newShader = constructMultiShader(QStringLiteral(":/schoolcore/shaders/model.vs"), QStringLiteral(":/schoolcore/shaders/multimodel.fs"));
    newShader->bind();
    newShader->enableAttributeArray(0);
    newShader->setAttributeBuffer(0, GL_FLOAT, 0, 3, this->stride * sizeof(GLfloat));
    newShader->enableAttributeArray(1);
    newShader->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 2, this->stride * sizeof(GLfloat));
    for (int j = 0; j < 6; j++) {
      if (textures[6 * i + j] && textures[6 * i + j]->isCreated()) {
        glActiveTexture(GL_TEXTURE0 + j);
        textures[6 * i + j]->bind();
        newShader->setUniformValue(QString("textureSampler%1").arg(j).toStdString().c_str(), j);
        //textures[6 * i + j]->release();
      } else {
        logMessage("Texture is not created", Qgis::MessageLevel::Critical);
        return;
      }
    }
    newShader->release();
    shaders.append(newShader);
  }
  */
  //logMessage("Model initialized", Qgis::MessageLevel::Info);
  this->vbo.release();
  this->vao.release();
  checkGLError("Model::Model");
}

void ModelGroup::draw(const QMatrix4x4 &view, const QMatrix4x4 &projection) {
    if (!shader || !shader->isLinked()) {
        logMessage("Shader program is not valid or not linked", Qgis::MessageLevel::Critical);
        return;
    }
    if (this->vertexNum == 0 || this->vertices == nullptr) {
        logMessage("Invalid vertex data", Qgis::MessageLevel::Critical);
        return;
    }
    this->vao.bind();
    this->shader->bind();
    checkGLError("Model::draw - after shader bind");
    //this->shader->setUniformValue("model", this->modelMatrix);
    this->shader->setUniformValue("view", view);
    this->shader->setUniformValue("projection", projection);
    GLuint startIndex = 0;
    /*
    int groupNum = (models.size()-1) / 6 + 1;
    for (int g = 0; g < groupNum; g++) {
      //logMessage(QString("Drawing group %1").arg(g), Qgis::MessageLevel::Info);
      shaders[g]->bind();
      shaders[g]->setUniformValue("model", this->modelMatrix);
      shaders[g]->setUniformValue("view", view);
      shaders[g]->setUniformValue("projection", projection);
      for (int j = 0; j < 6; j++) {
        //logMessage(QString("Drawing texture %1").arg(6 * g + j), Qgis::MessageLevel::Info);
        shaders[g]->setUniformValue("textureID", j);
        shaders[g]->setUniformValue(QString("textureSampler%1").arg(j).toStdString().c_str(), j);
        glDrawArrays(this->primitiveType, startIndex, verticesRange[6 * g + j]);
        startIndex += verticesRange[6 * g + j];
      }
      shaders[g]->release();
    }
    */
    for (int i = 0; i < models.size(); i++) {
      if (textures[i] && textures[i]->isCreated()) {
        textures[i]->bind();
        this->shader->setUniformValue("textureSampler", 0);
      } else {
        logMessage("Texture is not created", Qgis::MessageLevel::Critical);
        return;
      }
      glDrawArrays(this->primitiveType, startIndex, verticesRange[i]);
      startIndex += verticesRange[i];
      checkGLError("Model::draw - after setting uniforms");
      textures[i]->release();
    }
    this->shader->release();
    this->vao.release();
    if (wsp::WindowManager::getInstance().isEditing())
      this->basePlaneWidget->draw(view, projection);
}
/*
std::shared_ptr<QOpenGLShaderProgram> ModelGroup::constructMultiShader(const QString& vertexShaderPath, const QString& fragmentShaderPath) {
  // check if opengl context is current
  if (!QOpenGLContext::currentContext()) {
    logMessage("constructShader: OpenGL context is not current", Qgis::MessageLevel::Critical);
    return nullptr;
  }
  auto shader = std::make_shared<QOpenGLShaderProgram>();
  logMessage(QString("Constructing shader from %1 and %2").arg(vertexShaderPath).arg(fragmentShaderPath), Qgis::MessageLevel::Info);
  if (!shader->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                       vertexShaderPath)) {
    logMessage(QString("Shader Error:") + this->shader->log(),
               Qgis::MessageLevel::Critical);
    return nullptr;
  }
  if (!shader->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                       fragmentShaderPath)) {
    logMessage(QString("Shader Error:") + this->shader->log(),
               Qgis::MessageLevel::Critical);
    return nullptr;
  }
  if (!shader->link()) {
    logMessage(QString("Shader Link Error:") + this->shader->log(),
               Qgis::MessageLevel::Critical);
    return nullptr;
  }
  return shader;
}
*/

bool Primitive::constructShader(const QString& vertexShaderPath, const QString& fragmentShaderPath, const QString& geometryShaderPath) {
  // check if opengl context is current
  if (!QOpenGLContext::currentContext()) {
    logMessage("constructShader: OpenGL context is not current", Qgis::MessageLevel::Critical);
    this->shader = nullptr;
    return false;
  }
  this->shader = std::make_shared<QOpenGLShaderProgram>();
  //logMessage(QString("Constructing shader from %1 and %2").arg(vertexShaderPath).arg(fragmentShaderPath), Qgis::MessageLevel::Info);
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

void ModelGroup::generateTexture(const QString &texturePath){
  if (texturePath.isEmpty()) {
    logMessage("Texture path is empty", Qgis::MessageLevel::Critical);
    return;
  }
  textures.append(std::make_shared<QOpenGLTexture>(QImage(texturePath).mirrored()));
  textures.back()->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
  textures.back()->setMagnificationFilter(QOpenGLTexture::Linear);
  textures.back()->setWrapMode(QOpenGLTexture::Repeat);
  logMessage("Texture generated", Qgis::MessageLevel::Success);
}

ModelGroup::~ModelGroup(){
  for (const auto& texture : textures) {
    if (texture && texture->isCreated()) {
      texture->destroy();
    }
  }
  textures.clear();
  models.clear();
  objFilePaths.clear();
  logMessage("ModelGroup destroyed", Qgis::MessageLevel::Success);
}

ModelGroup::ModelGroup(const QString &objFileFolderPath):Primitive(GL_TRIANGLES, 5),objFileFolderPath(objFileFolderPath){
  QDir dir(objFileFolderPath);
  QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
  for (const QString& subDir : subDirs) {
    QString subDirPath = dir.filePath(subDir);
    QString objFilePath = retriveObjFilePath(subDirPath);
    if (objFilePath.isEmpty()) {
      logMessage(QString("No obj file found in %1").arg(subDirPath), Qgis::MessageLevel::Critical);
      continue;
    }
    objFilePaths.append(objFilePath);
    models.append(std::make_shared<model::ModelData>(objFilePath));
  }
  initModelData();
  this->calcBounds();
  this->basePlaneWidget = std::make_shared<BasePlane>(mBounds, wsp::FlightManager::getInstance().getBaseHeight());
}

QString ModelGroup::retriveObjFilePath(const QString &subDirPath){
  QDir dir(subDirPath);
  QStringList fileList = dir.entryList(QDir::Files);
  for (const QString& fileName : fileList) {
    if (fileName.endsWith(".obj")) {
      return dir.filePath(fileName);
    }
  }
  return QString();
}

void ModelGroup::clear(){
  for (const auto& texture : textures) {
    if (texture && texture->isCreated()) {
      texture->destroy();
    }
  }
  textures.clear();
  models.clear();
  objFilePaths.clear();
  logMessage("ModelGroup cleared", Qgis::MessageLevel::Success);
}

void ModelGroup::calcBounds(){
  Bounds bounds;
  for (const auto& model : models) {
    bounds.merge(model->mBounds);
  }
  mBounds = bounds;
  mBounds.center = (mBounds.min + mBounds.max) / 2.0f;
}

Drone::Drone(const QString &objFilePath) : ColorPrimitive(GL_TRIANGLES) {
  modelData = std::make_shared<model::ModelData>(objFilePath);
  initModelData();
  center = std::make_shared<SinglePoint>(QVector3D(0, 0, 0),QVector4D(1.0,0.0,0.0,1.0));
  logMessage("Drone initialized", Qgis::MessageLevel::Info);
}

Drone::~Drone(){
  logMessage("Drone destroyed", Qgis::MessageLevel::Success);
}

void Drone::initModelData(){
  if (!QOpenGLContext::currentContext()) {
    logMessage("initModelData: OpenGL context is not current", Qgis::MessageLevel::Critical);
    return;
  }
  if (this->vao.isCreated()) {
    logMessage("VAO is already created", Qgis::MessageLevel::Warning);
  } else {
    logMessage("VAO is not created, creating now", Qgis::MessageLevel::Info);
    this->vao.create();
    if (!this->vao.isCreated()) {
      logMessage("Failed to create VAO", Qgis::MessageLevel::Critical);
      return;
    }
  }
  this->vbo.create();
  this->vao.bind();
  this->vbo.bind();
  this->vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
  logMessage("stride is " + QString::number(this->stride), Qgis::MessageLevel::Info);
  this->vertexNum = modelData->vertices.size();
  logMessage("vertexNum: " + QString::number(this->vertexNum), Qgis::MessageLevel::Info);
  GLuint count = this->vertexNum * this->stride;
  this->vertices = new GLfloat[count];
  GLuint index = 0;
  for (const auto& vertex : modelData->vertices) {
    this->vertices[index++] = vertex.position.x();
    this->vertices[index++] = vertex.position.y();
    this->vertices[index++] = vertex.position.z();
  }
  this->vbo.allocate(this->vertices, count * sizeof(GLfloat));
  //logMessage("start constructing shader", Qgis::MessageLevel::Info);
  constructShader(QStringLiteral(":/schoolcore/shaders/base.vs"), QStringLiteral(":/schoolcore/shaders/base.fs"));
  this->vbo.release();
  this->vao.release();
  initShaderAllocate();
  checkGLError("Drone::initModelData");
}

void Drone::draw(const QMatrix4x4 &view, const QMatrix4x4 &projection){
  Camera &camera = Camera::getInstance();
  QMatrix4x4 cameraModelMatrix;
  QVector3D dronePosition;
  wsp::FlightManager& flightManager = wsp::FlightManager::getInstance();
  if (flightManager.isManualMode())
    dronePosition = camera.mPosition + camera.mFront * camera.mDis2Camera;
  else
    dronePosition = camera.mPosition;
  flightManager.setPorision(dronePosition);
  cameraModelMatrix.translate(dronePosition);
  cameraModelMatrix.scale(0.1, 0.1, 0.1);
  cameraModelMatrix.rotate(camera.zeroPitchDirect());
  cameraModelMatrix.rotate(90, 0, 0, 1);
  cameraModelMatrix.rotate(90, 1, 0, 0);
  //cameraModelMatrix.rotate(-30, 1, 0, 0);
  this->shader->bind();
  this->shader->setUniformValue("model", cameraModelMatrix);
  this->shader->release();
  ColorPrimitive::draw(view, projection);
  center->setModelMatrix(cameraModelMatrix);
  center->draw(view, projection);
}

SelectLine::SelectLine() {
  orientLine = nullptr;
  orientPoint = nullptr;
}

void SelectLine::draw(const QMatrix4x4 &view, const QMatrix4x4 &projection) {
  QVector<QVector3D> vertices = calcOrientLine(wsp::FlightManager::getInstance().getBaseHeight());
  orientLine = std::make_shared<gl::OrientLine>(vertices);
  orientPoint = std::make_shared<gl::SinglePoint>(vertices[1], QVector4D(0.0, 1.0, 0.0, 1.0));
  orientLine->draw(view, projection);
  orientPoint->draw(view, projection);
}

QVector<QVector3D> SelectLine::calcOrientLine(float baseHeight){
    Camera &camera = Camera::getInstance();
    QVector4D nearPoint{0.0,-1.0,-1.0,1.0}, farPoint{0.0,0.0,1.0,1.0};
    QMatrix4x4 invProjection = camera.projectionMatrix().inverted();
    QMatrix4x4 invView = camera.viewMatrix().inverted();
    QVector4D clipNear = invProjection * nearPoint;
    QVector4D clipFar = invProjection * farPoint;
    clipNear /= clipNear.w();
    clipFar /= clipFar.w();
    QVector4D worldNear = invView * clipNear;
    QVector4D worldFar = invView * clipFar;
    QVector3D origin(worldNear.x(), worldNear.y(), worldNear.z());
    QVector3D farPos(worldFar.x(), worldFar.y(), worldFar.z());
    //logMessage(QString("distance %1").arg((baseHeight - camera.mPosition.z()) / camera.mFront.z()), Qgis::MessageLevel::Info);
    QVector3D direction = origin + camera.mFront * (baseHeight - camera.mPosition.z()) / camera.mFront.z();//(farPos - origin).normalized();
  return QVector<QVector3D>{origin, direction};
}

QVector3D SelectLine::submitPoint(){
  QVector3D point(orientPoint->vertices[0],
                  orientPoint->vertices[1],
                  orientPoint->vertices[2]);
  return point;
}
}