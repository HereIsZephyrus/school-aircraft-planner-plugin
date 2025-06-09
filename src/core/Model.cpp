#include "Model.h"


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