#include "MyOpenGLWidget.h"
#include "qgis_debug.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMouseEvent>
#include <QOpenGLFunctions_3_3_Core>
#include <QTextStream>
#include <QTransform>
#include <QWheelEvent>
#include <qtimer.h>
#include <qgsapplication.h>
#include <qgis.h>

MyOpenGLWidget::MyOpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent), m_routePlanner(new RoutePlanner(this)),
      m_animationProgress(0.0f), m_isAnimating(false),
      m_cameraFollowAircraft(false), m_viewTranslation(0, 0, 0) {
  logMessage("MyOpenGLWidget constructor", Qgis::MessageLevel::Info);
  
  mfDistance = -1100.0f; // 增大距离确保模型可见
  RoutePlanner *routePlanner = new RoutePlanner(this);
  m_animationTimer = new QTimer(this);
  connect(m_animationTimer, &QTimer::timeout, this,
          &MyOpenGLWidget::updateAnimation);
  logMessage("m_animationTimer connected", Qgis::MessageLevel::Info);
  setFocusPolicy(Qt::StrongFocus); // 设置为强焦点模式
  setFocus();                      // 主动获取焦点
}

MyOpenGLWidget::~MyOpenGLWidget() {
  makeCurrent();
  mVAO.destroy();
  mVBO.destroy();
  delete m_texture;
  m_texture = nullptr;
  doneCurrent();
}

void MyOpenGLWidget::initializeGL() {
  // 检查QGIS环境
  if (!QgsApplication::instance()) {
    qWarning() << "QGIS application not initialized";
    return;
  }
  
  // 设置QGIS OpenGL格式
  QgsApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
  QgsApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
  
  // 设置OpenGL格式
  QSurfaceFormat format;
  format.setVersion(3, 3);
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setDepthBufferSize(24);
  format.setStencilBufferSize(8);
  format.setSamples(4);  // 多重采样
  QSurfaceFormat::setDefaultFormat(format);
  setFormat(format);
  
  // 确保当前上下文
  makeCurrent();
  
  // 检查上下文有效性
  if (!isValid()) {
    logMessage("OpenGL context is not valid", Qgis::MessageLevel::Critical);
    return;
  }
  initializeOpenGLFunctions();
  
  // 检查OpenGL版本
  QString version = QString::fromUtf8((const char*)glGetString(GL_VERSION));
  logMessage("OpenGL Version: " + version, Qgis::MessageLevel::Info);
  
  // 继续其他初始化
  glEnable(GL_DEPTH_TEST);
  logMessage("depth test enabled", Qgis::MessageLevel::Info);
  initShaders();
  logMessage("shader initialized", Qgis::MessageLevel::Info);
  initBuffers();
  logMessage("buffers initialized", Qgis::MessageLevel::Info);

  mProjection.perspective(45.0f, width() / (float)height(), 0.1f, 1000.0f);
  logMessage("projection initialized", Qgis::MessageLevel::Info);
  // 线框着色器
  if (!m_lineShader.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                            ":/shaders/line_vshader.glsl")) {
    logMessage(QString("m_lineShader Line Vertex Shader Error:") + m_lineShader.log(), Qgis::MessageLevel::Critical);
  }else{
    logMessage("m_lineShader Line Vertex Shader initialized", Qgis::MessageLevel::Info);
  }
  if (!m_lineShader.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                            ":/shaders/line_fshader.glsl")) {
    logMessage(QString("m_lineShader Line Fragment Shader Error:") + m_lineShader.log(), Qgis::MessageLevel::Critical);
  }else{
    logMessage("m_lineShader Line Fragment Shader initialized", Qgis::MessageLevel::Info);
  }
  if (!m_lineShader.link()) {
    logMessage(QString("m_lineShader Line Shader Link Error:") + m_lineShader.log(), Qgis::MessageLevel::Critical);
  }else{
    logMessage("m_lineShader Line Shader initialized", Qgis::MessageLevel::Info);
  }
  m_pointVAO.create();
  m_hullVAO.create();
  m_routeVAO.create();

  m_pointVBO.create();
  m_hullVBO.create();
  m_routeVBO.create();

  // 初始化基准面数据
  m_basePlaneVAO.create();
  m_basePlaneVBO.create();

  // 生成顶点数据
  const float size = 1000.0f;
  const float step = 50.0f;
  QVector<float> vertices;
  for (float x = -size; x <= size; x += step) {
    vertices << x << -size << 0.0f << x << size << 0.0f;
  }
  for (float y = -size; y <= size; y += step) {
    vertices << -size << y << 0.0f << size << y << 0.0f;
  }

  // 上传数据到 VBO
  m_basePlaneVAO.bind();
  m_basePlaneVBO.bind();
  m_basePlaneVBO.allocate(vertices.constData(),
                          vertices.size() * sizeof(float));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  m_basePlaneVAO.release();
  m_basePlaneVBO.release();

  emit glInitialized(); // 添加在函数末尾
  doneCurrent();
}

void MyOpenGLWidget::paintGL() {
  qDebug() << "paintGL";
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // 绑定着色器程序
  mShaderProgram.bind();

  QMatrix4x4 view;
  // MyOpenGLWidget.cpp 修改paintGL()中的视角计算
  QVector3D up = QVector3D(0, 1, 0); // 改为固定世界坐标系的上方向
  if (m_cameraFollowAircraft) {
    QVector3D eyeOffset = m_aircraftOrientation * QVector3D(0, 0, 50.0f);
    QVector3D eye = m_aircraftPosition + eyeOffset;
    view.lookAt(eye, m_aircraftPosition, up); // 使用固定up向量
  }

  else {
    view.translate(0, 0, mfDistance);
    view.translate(m_viewTranslation); // 应用平移
    mViewMatrix = view;                // 保存当前视图矩阵
  }
  // 遍历所有模型并渲染
  for (ModelData *model : m_models) {
    // 绑定当前模型的 VAO
    model->vao.bind();

    // 设置模型视图矩阵和 MVP 矩阵
    QMatrix4x4 modelView = view * mModelView; // 使用全局的 m_modelView
    QMatrix4x4 mvp = mProjection * view * mModelView; // MVP 矩阵

    // 传递矩阵到着色器
    mShaderProgram.setUniformValue("mvp", mvp);
    mShaderProgram.setUniformValue("modelView", modelView);
    mShaderProgram.setUniformValue("normalMatrix", modelView.normalMatrix());

    //// 绑定纹理（如果存在）
    // if (model->texture && model->texture->isCreated()) {
    //     model->texture->bind();
    //     mShaderProgram.setUniformValue("textureSampler", 0);
    // }
    // MyOpenGLWidget.cpp 中 paintGL() 的绘制部分修改为：
    QMap<QString, QVector<Vertex>>::const_iterator it;
    for (it = model->materialGroups.constBegin();
         it != model->materialGroups.constEnd(); ++it) {
      const QString &materialName = it.key();
      const QVector<Vertex> &vertices = it.value();

      if (model->textures.contains(materialName)) {
        model->textures[materialName]->bind();
        mShaderProgram.setUniformValue("textureSampler", 0);
      }

      // 绘制该材质组
      model->vao.bind();
      model->vbo.bind();
      model->vbo.allocate(vertices.constData(),
                          vertices.size() * sizeof(Vertex));
      glDrawArrays(GL_TRIANGLES, 0, vertices.size());
      model->vbo.release();
      model->vao.release();
    }

    // 绘制当前模型
    glDrawArrays(GL_TRIANGLES, 0, model->vertices.size());

    // 解绑 VAO
    model->vao.release();
  }

  // 解绑着色器程序
  mShaderProgram.release();
  drawBasePlane();
  if (m_routePlanner) {
    drawControlPoints();
    drawConvexHull();
    drawRoutePath();
  }
  if (m_isAnimating || m_animationProgress > 0) {
    drawAircraft(m_aircraftPosition, m_aircraftOrientation);
  }
}

void MyOpenGLWidget::resizeGL(int w, int h) {
  mProjection.setToIdentity();
  mProjection.perspective(45.0f, w / (float)h, 1.0f, 100000.0f);
}

void MyOpenGLWidget::initShaders() {
  if (!mShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                              ":/shaders/vshader.glsl")) {
    logMessage(mShaderProgram.log(), Qgis::MessageLevel::Info);
    logMessage(QString("Vertex Shader Error:") + mShaderProgram.log(), Qgis::MessageLevel::Critical);
  }
  if (!mShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                              ":/shaders/fshader.glsl")) {
    logMessage(mShaderProgram.log(), Qgis::MessageLevel::Info);
    logMessage(QString("Fragment Shader Error:") + mShaderProgram.log(), Qgis::MessageLevel::Critical);
  }
  if (!mShaderProgram.link()) {
    logMessage(mShaderProgram.log(), Qgis::MessageLevel::Info);
    logMessage(QString("Shader Link Error:") + mShaderProgram.log(), Qgis::MessageLevel::Critical);
  }
}

void MyOpenGLWidget::initBuffers() {
  mVAO.create();
  QOpenGLVertexArrayObject::Binder vaoBinder(&mVAO);
  mVBO.create();
  mVBO.bind();
  mShaderProgram.bind();

  // 在初始化时绑定顶点属性
  mShaderProgram.enableAttributeArray(0);
  mShaderProgram.setAttributeBuffer(0, GL_FLOAT, offsetof(Vertex, position), 3,
                                    sizeof(Vertex));

  mShaderProgram.enableAttributeArray(1);
  mShaderProgram.setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, texCoord), 2,
                                    sizeof(Vertex));

  mVAO.release();
  mVBO.release();
}

void MyOpenGLWidget::loadObjModel(const QString &filePath,
                                  const QString &texturePath) {
  makeCurrent();

  ModelData *modelData = new ModelData;
  QString mtlPath;

  // [1] 解析 OBJ 文件头获取材质库
  QFile objFile(filePath);
  if (objFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&objFile);
    while (!in.atEnd()) {
      QString line = in.readLine().trimmed();
      if (line.startsWith("mtllib")) {
        mtlPath = QFileInfo(filePath).absolutePath() + "/" + line.split(" ")[1];
        break;
      }
    }
    objFile.close();
  }

  // [2] 加载材质文件
  if (!mtlPath.isEmpty()) {
    loadMtl(mtlPath, modelData);
  }

  // [3] 重新打开OBJ文件解析几何数据
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

      // 处理材质切换
      if (parts[0] == "usemtl") {
        currentMaterial = parts[1];
      }
      // 处理顶点数据
      else if (parts[0] == "v") {
        positions.append(QVector3D(parts[1].toFloat(), parts[2].toFloat(),
                                   parts[3].toFloat()));
      } else if (parts[0] == "vt") {
        texCoords.append(QVector2D(parts[1].toFloat(),
                                   1.0f - parts[2].toFloat() // 翻转Y轴
                                   ));
      }
      // 处理面数据
      // 修改后的面解析逻辑
      else if (parts[0] == "f") {
        QVector<Vertex> faceVertices;
        // 三角化面数据（假设是三角形面）
        for (int i = 1; i <= 3; ++i) { // 只处理三角形前三个顶点
          QStringList indices = parts[i].split("/", Qt::KeepEmptyParts);

          Vertex vertex;
          // 位置索引
          int posIndex = indices[0].toInt() - 1;
          if (posIndex >= 0 && posIndex < positions.size()) {
            vertex.position = positions[posIndex];
          }

          // 纹理坐标索引
          if (indices.size() > 1 && !indices[1].isEmpty()) {
            int texIndex = indices[1].toInt() - 1;
            if (texIndex >= 0 && texIndex < texCoords.size()) {
              vertex.texCoord = texCoords[texIndex];
              vertex.texCoord.setY(
                  1.0f - vertex.texCoord.y()); // OpenGL纹理坐标需要翻转Y轴
            }
          }

          modelData->materialGroups[currentMaterial].append(vertex);
        }
      }
    }
    objFile.close();
  }

  //// [4] 计算模型中心并平移
  // QVector3D center = calculateModelCenter(modelData);
  // for (auto& group : modelData->materialGroups) {
  //     for (Vertex& v : group) {
  //         v.position -= center;
  //     }
  // }
  //  [4] 计算模型原始包围盒（不要修改顶点）
  ModelBounds bounds;
  QString bound_range = QString("bound_range: %1-%2-%3+%4-%5-%6").arg(bounds.min.x()).arg(bounds.min.y()).arg(bounds.min.z()).arg(bounds.max.x()).arg(bounds.max.y()).arg(bounds.max.z());
  logMessage(bound_range, Qgis::MessageLevel::Info);
  calculateModelBounds(modelData, bounds);

  // 存储原始数据
  m_modelBoundsMap[modelData] = bounds;

  // [5] 更新全局包围盒
  updateGlobalBounds(bounds);

  // [5] 初始化OpenGL资源
  modelData->vao.create();
  modelData->vbo.create();

  modelData->vao.bind();
  modelData->vbo.bind();
  modelData->vbo.allocate(nullptr, 0); // 先分配空缓冲
  logMessage("initBuffers", Qgis::MessageLevel::Info);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, position));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, texCoord));

  modelData->vbo.release();
  modelData->vao.release();
  logMessage("releaseBuffers", Qgis::MessageLevel::Info);
  m_models.append(modelData);
  doneCurrent();
  update();
  logMessage("loadObjModel", Qgis::MessageLevel::Info);
}

// 辅助函数：计算模型中心
QVector3D MyOpenGLWidget::calculateModelCenter(ModelData *modelData) {
  float minX = FLT_MAX, maxX = -FLT_MAX;
  float minY = FLT_MAX, maxY = -FLT_MAX;
  float minZ = FLT_MAX, maxZ = -FLT_MAX;

  for (const auto &group : modelData->materialGroups) {
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
void MyOpenGLWidget::mousePressEvent(QMouseEvent *event) {

  if (m_routePlanner && m_routePlanner->mCreateRoute) {
    // 优先处理添加控制点模式
    if (m_routePlanner->isSettingHomePointMode() &&
        event->button() == Qt::LeftButton) {
      QVector3D surfacePoint = getSurfacePointFromMouse();
      if (surfacePoint != QVector3D(0, 0, 0)) {
        m_routePlanner->setHomePoint(surfacePoint);
        m_routePlanner->setSettingHomePointMode(false); // 退出设置模式
        update();
        return;
      }
    }
    if (m_routePlanner->isAddingControlPoint() &&
        event->button() == Qt::LeftButton) {
      QVector3D surfacePoint = getSurfacePointFromMouse();
      if (surfacePoint != QVector3D(0, 0, 0)) {
        qDebug() << "Adding control point with coordinates: "
                 << "X:" << surfacePoint.x() << "Y:" << surfacePoint.y()
                 << "Z:" << surfacePoint.z();
        addControlPoint(surfacePoint); // 添加点并自动退出添加模式
      } else {
        qDebug() << "surfacePoint == QVector3D(0, 0, 0)";
      }
      qDebug() << "111stop mousePressEvent";
      return; // 阻断后续逻辑
    }

    //其次处理编辑模式（移动/删除点）

    if (m_routePlanner->isEditing()) {
      QVector3D surfacePoint = getSurfacePointFromMouse();

      if (event->button() == Qt::LeftButton) {
        // 选择最近的控制点
        float minDist = FLT_MAX;
        int selectedIndex = -1;
        QVector<QVector3D> points = m_routePlanner->controlPoints();

        for (int i = 0; i < points.size(); ++i) {
          float dist = (points[i] - surfacePoint).lengthSquared();
          if (dist < minDist) {
            qDebug() << "选择最近的控制点";
            minDist = dist;
            selectedIndex = i;
          }
        }

        if (minDist < 10000000.0f) { // 10单位的平方
          qDebug() << "minDist < 1000.0f";
          m_routePlanner->setSelectedPoint(selectedIndex);
        } else {
          m_routePlanner->setSelectedPoint(-1);
        }
      } else if (event->button() == Qt::RightButton) {
        // 删除选中的点
        m_routePlanner->removeSelectedPoint();
      }

      update();
      return;
    }
  }

  // 默认处理：视图旋转
  m_lastMousePos = event->pos();
}

void MyOpenGLWidget::mouseMoveEvent(QMouseEvent *event) {
  if (m_cameraFollowAircraft) {
    return; // 跟随视角下不处理旋转
  }
  if (m_isAnimating)
    return;
  if (m_routePlanner) {
    // 优先处理添加控制点模式

    if (m_routePlanner->mCreateRoute) {

      if (m_routePlanner->isAddingControlPoint()) {
        return;
      }
      if (m_routePlanner->isSettingHomePointMode()) {
        return;
      }
    }
    if (m_routePlanner->isEditing() && m_routePlanner->mCreateRoute) {
      handleMouseMove(event);
    } else {
      // 默认处理：视图旋转
      float dx = event->x() - m_lastMousePos.x();
      float dy = event->y() - m_lastMousePos.y();
      if (event->buttons() & Qt::LeftButton) {
        QMatrix4x4 rotation;
        rotation.rotate(dx, QVector3D(0, 1, 0));
        rotation.rotate(dy, QVector3D(1, 0, 0));
        mModelView = rotation * mModelView;
        update();
      }
      m_lastMousePos = event->pos();
    }
  }
}

void MyOpenGLWidget::wheelEvent(QWheelEvent *event) {
  mfDistance *= event->angleDelta().y() > 0 ? 0.9f : 1.1f;
  update();
}

void MyOpenGLWidget::updateFlightHeight(double height) {
  mfFlightHight = height - m_initialBaseHeight;
  qDebug() << "height=" << mfFlightHight + 25.0f;
  update();
}

void MyOpenGLWidget::resetView() {

  mfDistance = -1100.0f;
  mModelView.setToIdentity();

  update();
}

void MyOpenGLWidget::setRoutePlanner(RoutePlanner *planner) {
  m_routePlanner = planner;
  connect(planner, &RoutePlanner::dataUpdated, this,
          QOverload<>::of(&QOpenGLWidget::update));
}
// MyOpenGLWidget.cpp 绘制控制点实现
void MyOpenGLWidget::drawControlPoints() {

  if (!m_routePlanner || m_routePlanner->controlPoints().isEmpty())
    return;

  if (!m_lineShader.bind()) {
    qDebug() << "Failed to bind line shader";
    return;
  }

  glDisable(GL_DEPTH_TEST); // 禁用深度测试
  QMatrix4x4 view;
  view.translate(0, 0, mfDistance);
  view.translate(m_viewTranslation); // 添加平移
  QMatrix4x4 mvp = mProjection * view * mModelView;
  m_lineShader.setUniformValue("mvp", mvp);

  // 绘制控制点
  QVector<QVector3D> points = m_routePlanner->controlPoints();
  QVector<float> vertexData;
  for (const QVector3D &p : points) {
    vertexData << p.x() << p.y() << p.z() + mfFlightHight + m_baseHeight;
  }

  m_pointVAO.bind();
  m_pointVBO.bind();
  m_pointVBO.allocate(vertexData.constData(),
                      vertexData.size() * sizeof(float));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  // 获取 OpenGL 函数指针并设置点的大小
  QOpenGLFunctions_3_3_Core *functions =
      QOpenGLContext::currentContext()
          ->versionFunctions<QOpenGLFunctions_3_3_Core>();
  if (functions) {
    functions->glPointSize(10.0f);
  }

  // 绘制所有控制点
  m_lineShader.setUniformValue("color",
                               QVector4D(1.0f, 0.0f, 0.0f, 1.0f)); // 红色
  glDrawArrays(GL_POINTS, 0, points.size());

  // 绘制选中的点（黄色）
  int selectedIndex = m_routePlanner->selectedPointIndex();
  if (selectedIndex >= 0 && selectedIndex < points.size()) {
    m_lineShader.setUniformValue("color",
                                 QVector4D(1.0f, 1.0f, 0.0f, 1.0f)); // 黄色
    glDrawArrays(GL_POINTS, selectedIndex, 1);
  }

  // 绘制Home点（紫色）
  if (m_routePlanner && !m_routePlanner->homePoint().isNull()) {
    QVector3D homePoint = m_routePlanner->homePoint();
    QVector<float> homeVertex;
    homeVertex << homePoint.x() << homePoint.y() << homePoint.z();

    m_lineShader.setUniformValue("color",
                                 QVector4D(0.5f, 0.0f, 0.5f, 1.0f)); // 紫色
    m_pointVBO.allocate(homeVertex.constData(),
                        homeVertex.size() * sizeof(float));
    glDrawArrays(GL_POINTS, 0, 1);
  }

  // 清理状态并恢复深度测试
  m_pointVBO.release();
  m_pointVAO.release();
  m_lineShader.release();
  glEnable(GL_DEPTH_TEST);
}

// MyOpenGLWidget.cpp 绘制凸包实现
void MyOpenGLWidget::drawConvexHull() {
  if (!m_routePlanner || m_routePlanner->convexHull().size() < 2)
    return;

  m_lineShader.bind();
  QMatrix4x4 view;
  view.translate(0, 0, mfDistance);
  QMatrix4x4 mvp = mProjection * view * mModelView;
  m_lineShader.setUniformValue("mvp", mvp);
  m_lineShader.setUniformValue("color",
                               QVector4D(0.0f, 1.0f, 0.0f, 1.0f)); // 绿色

  // 准备顶点数据
  QVector<QVector3D> hull = m_routePlanner->convexHull();
  QVector<float> vertexData;
  for (const QVector3D &p : hull) {
    vertexData << p.x() << p.y() << p.z() + mfFlightHight + m_baseHeight;
  }

  // 设置顶点缓冲
  m_hullVAO.bind();
  m_hullVBO.bind();
  m_hullVBO.allocate(vertexData.constData(), vertexData.size() * sizeof(float));

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  // 绘制设置
  glLineWidth(2.0f);
  glDrawArrays(GL_LINE_LOOP, 0, hull.size());

  // 清理状态
  m_hullVAO.release();
  m_hullVBO.release();
  m_lineShader.release();
}

void MyOpenGLWidget::drawRoutePath() {
  if (!m_routePlanner || m_routePlanner->routePath().isEmpty())
    return;

  QVector<QVector3D> route = m_routePlanner->routePath();
  if (route.size() < 2)
    return;
  QMatrix4x4 model = mModelView; // 获取当前模型矩阵
  m_lineShader.setUniformValue("model", model);
  m_lineShader.bind();
  QMatrix4x4 view;
  view.translate(0, 0, mfDistance);
  QMatrix4x4 mvp = mProjection * view * mModelView;
  m_lineShader.setUniformValue("mvp", mvp);

  // 绘制主路径（实线）
  if (route.size() > 2) {
    QVector<QVector3D> mainRoute = route.mid(1, route.size() - 2);
    drawPathSection(mainRoute, QVector4D(0.0f, 0.0f, 1.0f, 1.0f), 2.0f, false);
  }

  // 绘制启程点连接线（虚线）
  if (route.size() >= 2) {
    QVector<QVector3D> startLine = {route[0], route[1]};
    QVector<QVector3D> endLine = {route[route.size() - 2], route.back()};
    drawPathSection(startLine, QVector4D(1.0f, 0.5f, 0.0f, 1.0f), 2.0f, true);
    drawPathSection(endLine, QVector4D(1.0f, 0.5f, 0.0f, 1.0f), 2.0f, true);
  }

  m_lineShader.release();
}

void MyOpenGLWidget::drawPathSection(const QVector<QVector3D> &points,
                                     const QVector4D &color, float lineWidth,
                                     bool dashed) {
  if (points.size() < 2)
    return;

  // 设置绘制参数
  m_lineShader.setUniformValue("color", color);
  glLineWidth(lineWidth);

  if (dashed) {
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(4, 0x00FF); // 虚线模式
  }

  // 准备顶点数据
  QVector<float> vertexData;
  for (const QVector3D &p : points) {
    vertexData << p.x() << p.y() << p.z() + mfFlightHight + 2.0f;
  }

  // 绘制线段
  m_routeVAO.bind();
  m_routeVBO.bind();
  m_routeVBO.allocate(vertexData.constData(),
                      vertexData.size() * sizeof(float));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glDrawArrays(GL_LINES, 0, points.size());

  // 清理状态
  if (dashed)
    glDisable(GL_LINE_STIPPLE);
  m_routeVBO.release();
  m_routeVAO.release();
}

void MyOpenGLWidget::addControlPoint(const QVector3D &point) {
  if (m_routePlanner) {
    // 直接在基准面高度上添加控制点（不再需要Z轴偏移）

    QVector3D modelPoint =
        mModelView.inverted() * QVector3D(point.x(), point.y(), m_baseHeight);

    m_routePlanner->addControlPoint(modelPoint);
    update();
  } else {
    qDebug() << "错误：RoutePlanner 未初始化！";
  }
}
QVector3D MyOpenGLWidget::getSurfacePointFromMouse() {

  // 获取鼠标点击位置
  QPoint mousePos = this->mapFromGlobal(QCursor::pos());

  // 转换为标准化设备坐标
  float x = (2.0f * mousePos.x()) / width() - 1.0f;
  float y = 1.0f - (2.0f * mousePos.y()) / height();
  float z = 1.0f;

  // 计算射线方向
  QMatrix4x4 inverseProjection = mProjection.inverted();
  QVector4D rayEye = inverseProjection * QVector4D(x, y, -1.0f, 1.0f);
  rayEye.setZ(-1.0f);
  rayEye.setW(0.0f);

  QMatrix4x4 inverseView = mModelView.inverted();
  QVector4D rayWorld = inverseView * rayEye;
  QVector3D rayDir = rayWorld.toVector3D().normalized();

  // 计算射线起点（相机位置）
  QVector3D rayOrigin =
      QVector3D(inverseView(0, 3), inverseView(1, 3), inverseView(2, 3));

  return calculateRayIntersection(rayOrigin, rayDir);
}

QVector3D
MyOpenGLWidget::calculateRayIntersection(const QVector3D &rayOrigin,
                                         const QVector3D &rayDirection) {
  float closestDistance = FLT_MAX;
  QVector3D closestPoint;

  // 基准面高度
  const float planeZ = m_baseHeight;

  // 组合视图和模型矩阵
  QMatrix4x4 modelViewMatrix = mViewMatrix * mModelView;

  for (ModelData *model : m_models) {
    for (int i = 0; i < model->vertices.size(); i += 3) {
      // 应用模型视图变换到顶点
      QVector3D v0 = modelViewMatrix * model->vertices[i].position;
      QVector3D v1 = modelViewMatrix * model->vertices[i + 1].position;
      QVector3D v2 = modelViewMatrix * model->vertices[i + 2].position;

      // 使用变换后的顶点进行相交检测
      QVector3D edge1 = v1 - v0;
      QVector3D edge2 = v2 - v0;
      QVector3D h = QVector3D::crossProduct(rayDirection, edge2);
      float a = QVector3D::dotProduct(edge1, h);

      if (fabs(a) < 0.0001f)
        continue;

      float f = 1.0f / a;
      QVector3D s = rayOrigin - v0;
      float u = f * QVector3D::dotProduct(s, h);

      if (u < 0.0f || u > 1.0f)
        continue;

      QVector3D q = QVector3D::crossProduct(s, edge1);
      float v = f * QVector3D::dotProduct(rayDirection, q);

      if (v < 0.0f || u + v > 1.0f)
        continue;

      float t = f * QVector3D::dotProduct(edge2, q);
      if (t > 0.0001f && t < closestDistance) {
        closestDistance = t;
        // 计算交点坐标
        QVector3D intersection = rayOrigin + rayDirection * t;

        // 投影到基准面 Z = planeZ
        intersection.setZ(planeZ);
        closestPoint = intersection;
      }
    }
  }

  return closestDistance < FLT_MAX ? closestPoint : QVector3D();
}
void MyOpenGLWidget::handleMouseMove(QMouseEvent *event) {

  QPoint screenPos = event->pos(); // 从 QMouseEvent 获取鼠标位置
  if (m_routePlanner->selectedPointIndex() >= 0) {
    QVector3D surfacePoint = getSurfacePointFromMouse();
    m_routePlanner->m_controlPoints[m_routePlanner->selectedPointIndex()] =
        surfacePoint;
    m_routePlanner->handleMouseMove(event);
  }
}
void MyOpenGLWidget::generateFlightRoute(float height) // 生成航线
{
  m_routePlanner->m_editingMode = false;
}; // 生成航线

void MyOpenGLWidget::drawBasePlane() {
  if (!m_lineShader.bind())
    return;

  // 设置矩阵和颜色
  QMatrix4x4 view;
  view.translate(0, 0, mfDistance);
  view.translate(m_viewTranslation); // 添加这行
  QMatrix4x4 mvp = mProjection * view * mModelView;
  m_lineShader.setUniformValue("mvp", mvp);
  m_lineShader.setUniformValue("color", QVector4D(0.6f, 0.6f, 0.6f, 0.5f));

  // 生成网格顶点数据
  const float size = 1000.0f;
  const float step = 50.0f;
  QVector<float> vertices;
  for (float x = -size; x <= size; x += step) {
    vertices << x << -size << m_baseHeight << x << size << m_baseHeight;
  }
  for (float y = -size; y <= size; y += step) {
    vertices << -size << y << m_baseHeight << size << y << m_baseHeight;
  }

  // 上传并绘制数据
  m_basePlaneVAO.bind();
  m_basePlaneVBO.bind();
  m_basePlaneVBO.allocate(vertices.constData(),
                          vertices.size() * sizeof(float));

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glDrawArrays(GL_LINES, 0, vertices.size() / 3);

  m_basePlaneVBO.release();
  m_basePlaneVAO.release();
  m_lineShader.release();

  qDebug() << "drawBasePlane";
}

void MyOpenGLWidget::setBaseHeight(double height) {
  m_baseHeight = height + m_initialBaseHeight;
  qDebug() << "setBaseHeight =" << m_baseHeight - 25.0f;
  update(); // 触发重绘
}
void MyOpenGLWidget::startSimulation(float speed) {
  if (!m_routePlanner)
    return;

  m_flightPath = m_routePlanner->routePath();
  if (m_flightPath.size() < 2)
    return;

  m_animationProgress = 0.0f;
  m_isAnimating = true;
  m_animationTimer->start(16); // ~60 FPS
}

void MyOpenGLWidget::pauseSimulation() { m_animationTimer->stop(); }

void MyOpenGLWidget::resumeSimulation() { m_animationTimer->start(); }

void MyOpenGLWidget::returnToHome() {
  m_animationTimer->stop();
  m_animationProgress = 0.0f;
  update();
}

void MyOpenGLWidget::updateAnimation() {
  if (!m_isAnimating)
    return;

  // 计算新进度（示例使用固定速度，可根据实际速度参数调整）
  m_animationProgress += 0.002f;

  if (m_animationProgress >= 1.0f) {
    m_animationProgress = 1.0f;
    m_animationTimer->stop();
  }
  float currentHeight = m_baseHeight + mfFlightHight + m_initialBaseHeight;
  // 计算当前路径位置
  int index = static_cast<int>((m_flightPath.size() - 1) * m_animationProgress);
  float t = (m_flightPath.size() - 1) * m_animationProgress - index;
  index = qMin(index, m_flightPath.size() - 2);

  //   // 计算当前路径位置时应用模型旋转
  QVector3D start = mModelView * m_flightPath[index];   // 应用旋转
  QVector3D end = mModelView * m_flightPath[index + 1]; // 应用旋转
  m_aircraftPosition = start + t * (end - start);
  m_aircraftPosition.setZ(currentHeight);
  // 计算方向
  QVector3D direction = (end - start).normalized();
  float yaw = qRadiansToDegrees(atan2(direction.y(), direction.x()));
  m_aircraftOrientation = QQuaternion::fromEulerAngles(0, 0, -yaw);

  update();
}

void MyOpenGLWidget::drawAircraft(const QVector3D &position,
                                  const QQuaternion &orientation) {
  mShaderProgram.bind();

  QMatrix4x4 model;
  model.translate(position);
  model.rotate(orientation);
  model.scale(5.0f); // 调整飞机大小

  QMatrix4x4 view;
  if (m_cameraFollowAircraft) {
    // 调整相机位置到无人机后上方，实现倾斜视角
    QVector3D cameraOffset =
        QVector3D(0, 30, 50); // Y分量控制高度，Z分量控制距离
    QVector3D cameraPosition =
        m_aircraftPosition + m_aircraftOrientation * cameraOffset;

    view.lookAt(cameraPosition,     // 相机位置
                m_aircraftPosition, // 目标点（无人机位置）
                m_aircraftOrientation *
                    QVector3D(0, 1, 1)); // 保持相机的上方向与无人机一致
  } else {
    view.translate(0, 0, mfDistance);
    view = view * mModelView;
  }

  QMatrix4x4 mvp = mProjection * view * model; // 确保应用模型矩阵

  mShaderProgram.setUniformValue("mvp", mvp);

  // 使用简单立方体作为临时飞机模型
  static QVector<Vertex> aircraftVertices = createAircraftModel();

  // 创建并绑定临时VAO/VBO
  QOpenGLVertexArrayObject::Binder vaoBinder(&mVAO);
  mVBO.bind();
  mVBO.allocate(aircraftVertices.constData(),
                aircraftVertices.size() * sizeof(Vertex));

  glDrawArrays(GL_TRIANGLES, 0, aircraftVertices.size());

  mVBO.release();
  mShaderProgram.release();
}

QVector<MyOpenGLWidget::Vertex> MyOpenGLWidget::createAircraftModel() {
  QVector<Vertex> vertices;

  // 机身（使用红色纹理坐标区域）
  vertices << Vertex(QVector3D(-0.5, -0.5, 0.0), QVector2D(1.0, 0.0)) // 右下红
           << Vertex(QVector3D(0.5, -0.5, 0.0), QVector2D(1.0, 0.0)) // 左下红
           << Vertex(QVector3D(0.0, 1.0, 0.0), QVector2D(1.0, 0.0)); // 顶部红

  // 机翼（使用蓝色纹理坐标区域）
  vertices << Vertex(QVector3D(-1.0, 0.0, 0.0), QVector2D(0.0, 1.0)) // 左翼蓝
           << Vertex(QVector3D(1.0, 0.0, 0.0), QVector2D(0.0, 1.0)) // 右翼蓝
           << Vertex(QVector3D(0.0, 0.0, 1.0), QVector2D(0.0, 1.0)); // 顶部蓝

  return vertices;
}

void MyOpenGLWidget::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Space) {
    m_cameraFollowAircraft = !m_cameraFollowAircraft;
    qDebug() << "  m_cameraFollowAircraft=" << m_cameraFollowAircraft;
    update();
  } else {
    float step = 5.0f; // 调整平移步长
    switch (event->key()) {
    case Qt::Key_Left:
      m_viewTranslation += QVector3D(-step, 0, 0);
      update();
      break;
    case Qt::Key_Right:
      m_viewTranslation += QVector3D(step, 0, 0);
      update();
      break;
    case Qt::Key_Up:
      m_viewTranslation += QVector3D(0, step, 0);
      update();
      break;
    case Qt::Key_Down:
      m_viewTranslation += QVector3D(0, -step, 0);
      update();
      break;
    default:
      QOpenGLWidget::keyPressEvent(event);
      return;
    }
  }
}

void MyOpenGLWidget::stopSimulation() {
  m_animationTimer->stop();
  m_isAnimating = false;
  m_animationProgress = 0.0f;
  update(); // 更新界面
}
void MyOpenGLWidget::loadMtl(const QString &mtlPath, ModelData *modelData) {
  QFile file(mtlPath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return;

  Material currentMaterial;
  QTextStream in(&file);
  while (!in.atEnd()) {
    QString line = in.readLine().trimmed();
    QStringList parts = line.split(" ", Qt::SkipEmptyParts);
    if (parts.isEmpty())
      continue;

    if (parts[0] == "newmtl") {
      if (!currentMaterial.name.isEmpty()) {
        modelData->materials.append(currentMaterial);
      }
      currentMaterial = Material();
      currentMaterial.name = parts[1];
    } else if (parts[0] == "map_Kd") {
      QString texPath = QFileInfo(mtlPath).absolutePath() + "/" + parts[1];
      if (!modelData->textures.contains(currentMaterial.name)) {
        QImage img(texPath);
        if (!img.isNull()) {
          QOpenGLTexture *texture = new QOpenGLTexture(img.mirrored());
          texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
          texture->setMagnificationFilter(QOpenGLTexture::Linear);
          modelData->textures[currentMaterial.name] = texture;
        }
      }
    }
  }
  if (!currentMaterial.name.isEmpty()) {
    modelData->materials.append(currentMaterial);
  }
}
// 计算单个模型包围盒
void MyOpenGLWidget::calculateModelBounds(ModelData *modelData,
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
void MyOpenGLWidget::updateGlobalBounds(const ModelBounds &modelBounds) {
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

void MyOpenGLWidget::applyGlobalCentering() {
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

