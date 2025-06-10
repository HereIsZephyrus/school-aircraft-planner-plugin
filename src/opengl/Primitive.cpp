#include "Primitive.h"
#include "../log/QgisDebug.h"
#include "../core/WorkspaceState.h"
#include <GL/gl.h>
#include <QOpenGLContext>
#include <QProgressDialog>
#include <QCoreApplication>
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
  logMessage("Primitive destroyed", Qgis::MessageLevel::Success);
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
  logMessage("modelData->texturePath: " + modelData->texturePath, Qgis::MessageLevel::Info);
  generateTexture(modelData->texturePath);
  logMessage("start constructing shader", Qgis::MessageLevel::Info);
  constructShader(QStringLiteral(":/schoolcore/shaders/model.vs"), QStringLiteral(":/schoolcore/shaders/model.fs"));
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
  this->shader->bind();
  this->vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
  logMessage("stride is " + QString::number(this->stride), Qgis::MessageLevel::Info);

  this->vertexNum = modelData->vertices.size();
  logMessage("vertexNum: " + QString::number(this->vertexNum), Qgis::MessageLevel::Info);
  GLuint count = this->vertexNum * this->stride;
  logMessage("count is " + QString::number(count), Qgis::MessageLevel::Info);
  this->vertices = new GLfloat[count];
  GLuint index = 0;
  for (const auto& vertex : modelData->vertices) {
    this->vertices[index++] = vertex.position.x();
    this->vertices[index++] = vertex.position.y();
    this->vertices[index++] = vertex.position.z();
    this->vertices[index++] = vertex.texCoord.x();
    this->vertices[index++] = vertex.texCoord.y();
  }
  this->vbo.allocate(this->vertices, count * sizeof(GLfloat));
  
  this->shader->enableAttributeArray(0);
  this->shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, this->stride * sizeof(GLfloat));
  this->shader->enableAttributeArray(1);
  this->shader->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 2, this->stride * sizeof(GLfloat));
  logMessage("Model initialized", Qgis::MessageLevel::Info);
  this->vbo.release();
  this->vao.release();
  this->shader->release();
  checkGLError("Model::Model");
}

void Model::initDemoModelData(){
  logMessage("initDemoModelData", Qgis::MessageLevel::Info);
  //create a cube
  generateTexture(modelData->texturePath);
  this->vertexNum = 36;
  constructShader(QStringLiteral(":/schoolcore/shaders/model.vs"), QStringLiteral(":/schoolcore/shaders/model.fs"));
  this->vao.create();
  this->vbo.create();
  this->vao.bind();
  this->vbo.bind();
  this->shader->bind();
  this->vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);

  GLfloat cubeVertices[] = {
    //face-front
    -10.0f, -10.0f, 10.0f, 0.0f, 0.0f,
    10.0f, -10.0f, 10.0f, 1.0f, 0.0f,
    10.0f, 10.0f, 10.0f, 1.0f, 1.0f,
    10.0f, 10.0f, 10.0f, 1.0f, 1.0f,
    -10.0f, 10.0f, 10.0f, 0.0f, 1.0f,
    -10.0f, 10.0f, 10.0f, 0.0f, 1.0f,
    //face-back
    -10.0f, -10.0f, -10.0f, 0.0f, 0.0f,
    10.0f, -10.0f, -10.0f, 1.0f, 0.0f,
    10.0f, 10.0f, -10.0f, 1.0f, 1.0f,
    10.0f, 10.0f, -10.0f, 0.0f, 1.0f,
    -10.0f, -10.0f, -10.0f, 0.0f, 0.0f,
    -10.0f, 10.0f, -10.0f, 0.0f, 0.0f,
    //face-left
    -10.0f, -10.0f, 10.0f, 0.0f, 0.0f,
    -10.0f, -10.0f, -10.0f, 1.0f, 0.0f,
    -10.0f, 10.0f, -10.0f, 1.0f, 1.0f,
    -10.0f, 10.0f, 10.0f, 0.0f, 1.0f,
    -10.0f, 10.0f, 10.0f, 0.0f, 1.0f,
    -10.0f, 10.0f, -10.0f, 1.0f, 1.0f,
    //face-right
    -10.0f, -10.0f, 10.0f, 0.0f, 0.0f,
    10.0f, -10.0f, 10.0f, 1.0f, 0.0f,
    10.0f, 10.0f, -10.0f, 1.0f, 1.0f,
    -10.0f, -10.0f, -10.0f, 0.0f, 0.0f,
    -10.0f, -10.0f, 10.0f, 1.0f, 0.0f,
    10.0f, 10.0f, 10.0f, 1.0f, 1.0f,
    //face-top
    -10.0f, 10.0f, 10.0f, 0.0f, 0.0f,
    10.0f, 10.0f, 10.0f, 1.0f, 0.0f,
    -10.0f, 10.0f, -10.0f, 1.0f, 1.0f,
    -10.0f, 10.0f, 10.0f, 0.0f, 0.0f,
    10.0f, 10.0f, 10.0f, 1.0f, 0.0f,
    -10.0f, 10.0f, -10.0f, 1.0f, 1.0f,
    //face-bottom
    -10.0f, -10.0f, 10.0f, 0.0f, 0.0f,
    10.0f, -10.0f, 10.0f, 1.0f, 0.0f,
    10.0f, -10.0f, -10.0f, 1.0f, 1.0f,
    -10.0f, -10.0f, 10.0f, 0.0f, 0.0f,
    10.0f, -10.0f, 10.0f, 1.0f, 0.0f,
    10.0f, -10.0f, -10.0f, 1.0f, 1.0f,
  };
  
  this->vertices = new GLfloat[this->vertexNum * this->stride];
  memcpy(this->vertices, cubeVertices, sizeof(cubeVertices));
  this->vbo.allocate(this->vertices, this->vertexNum * this->stride * sizeof(GLfloat));

  this->shader->enableAttributeArray(0);
  this->shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, this->stride * sizeof(GLfloat));
  this->shader->enableAttributeArray(1);
  this->shader->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 2, this->stride * sizeof(GLfloat));
  logMessage("Model initialized", Qgis::MessageLevel::Info);
  this->vbo.release();
  this->vao.release();
  this->shader->release();
  logMessage("initDemoModelData done", Qgis::MessageLevel::Info);
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
    this->vao.bind();
    this->shader->bind();
    checkGLError("Model::draw - after shader bind");
    this->shader->setUniformValue("model", this->modelMatrix);
    this->shader->setUniformValue("view", view);
    this->shader->setUniformValue("projection", projection);
    if (texture && texture->isCreated()) {
      texture->bind();
      this->shader->setUniformValue("textureSampler",0);
    }else {
      logMessage("Texture is not created", Qgis::MessageLevel::Critical);
      return;
    }
    glDrawArrays(this->primitiveType, 0, this->vertexNum);
    checkGLError("Model::draw - after setting uniforms");
    texture->release();

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

void Model::generateTexture(const QString &texturePath){
  if (texturePath.isEmpty()) {
    logMessage("Texture path is empty", Qgis::MessageLevel::Critical);
    return;
  }
  texture = std::make_shared<QOpenGLTexture>(QImage(texturePath).mirrored());
  texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
  texture->setMagnificationFilter(QOpenGLTexture::Linear);
  texture->setWrapMode(QOpenGLTexture::Repeat);
  logMessage("Texture generated", Qgis::MessageLevel::Success);
}

Model::Model(const QString& objFilePath):Primitive(GL_TRIANGLES, 5){
  if (!QOpenGLContext::currentContext()) {
    logMessage("Model::Model: OpenGL context is not current", Qgis::MessageLevel::Critical);
    return;
  }
  modelData = std::make_shared<model::ModelData>(objFilePath);
  initModelData();
  //initDemoModelData();
  logMessage("Model constructed", Qgis::MessageLevel::Success);
}

Model::~Model(){
  if (texture && texture->isCreated()) {
    texture->destroy();
  }
  modelData.reset();
  modelData = nullptr;
  logMessage("Model destroyed", Qgis::MessageLevel::Success);
}

  if (progressUpdateInterval >= 97) {
    progressDialog->close();
    delete progressDialog;
    progressDialog = nullptr;
  }

void gl::Model::cleanupTextures() {
  if (!texture->isCreated()) {
    return;
  }
  texture->destroy();
}
}