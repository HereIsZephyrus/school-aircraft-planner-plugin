#include "Model.h"
#include "../log/QgisDebug.h"
#include <QProgressDialog>
#include <QCoreApplication>
#include <qstringliteral.h>

using namespace model;
void model::loadMtl(const QString &mtlPath, Material &material, QString &texturePath) {
  logMessage("loadMtl: " + mtlPath, Qgis::MessageLevel::Info);
  QFile file(mtlPath);
  
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    logMessage("Cannot open file: " + mtlPath, Qgis::MessageLevel::Critical);
    return;
  }

  QTextStream in(&file);
  while (!in.atEnd()) {
    QString line = in.readLine().trimmed();
    QStringList parts = line.split(" ", Qt::SkipEmptyParts);
    if (parts.isEmpty())
      continue;
    if (parts[0] == "map_Kd") {
      texturePath = QFileInfo(mtlPath).absolutePath() + "/" + parts[1];
      logMessage(QString("texPath: %1").arg(texturePath), Qgis::MessageLevel::Info);
    }
    if (parts[0] == "Kd") {
      material.diffuse = QVector3D(parts[1].toFloat(), parts[2].toFloat(), parts[3].toFloat());
    }
    if (parts[0] == "Ks") {
      material.specular = QVector3D(parts[1].toFloat(), parts[2].toFloat(), parts[3].toFloat());
    }
    if (parts[0] == "Ns") {
      material.shininess = parts[1].toFloat();
    }
    if (parts[0] == "Ka") {
      material.ambient = QVector3D(parts[1].toFloat(), parts[2].toFloat(), parts[3].toFloat());
    }
  }
  file.close();
}

QString model::retriveMtlPath(const QString &objfilePath) {
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

void model::loadVertices(const QString &objFilePath, QVector<Vertex> &vertices) {
  logMessage(QString("loadMaterialGroups: %1").arg(objFilePath), Qgis::MessageLevel::Info);
  QVector<QVector3D> positions;
  QVector<QVector2D> texCoords;
  QFile objFile(objFilePath);
  bool isCanceled = false;
  if (objFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    logMessage("open obj file to read", Qgis::MessageLevel::Info);
    QTextStream in(&objFile);
    const qint64 totalSize = objFile.size();
    qint64 processedBytes = 0;
    qint64 lastProgressUpdate = 0;
    const qint64 progressUpdateInterval = totalSize / 100;

    while (!in.atEnd()) {
      QString line = in.readLine().trimmed();
      processedBytes += line.length() + 1; // +1 for newline
      /*
      if (processedBytes - lastProgressUpdate >= progressUpdateInterval) {
        isCanceled = displayProgress(progressUpdateInterval);
        lastProgressUpdate = processedBytes;
      }
      */
      if (isCanceled){
        vertices.clear();
        return;
      }
        
      QStringList parts = line.split(" ", Qt::SkipEmptyParts);
      if (parts.isEmpty())
        continue;

      else if (parts[0] == "v") {
        positions.append(QVector3D(parts[1].toFloat(), parts[2].toFloat(),parts[3].toFloat()));
      }
      else if (parts[0] == "vt") {
        texCoords.append(
            QVector2D(parts[1].toFloat(), parts[2].toFloat()));
      }
      else if (parts[0] == "f") {
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
              //vertex.texCoord.setY(1.0f - vertex.texCoord.y());
            }
          }
          vertices.append(vertex);
        }
      }
    }
    objFile.close();
  }
  logMessage("load obj file", Qgis::MessageLevel::Info);
}

qint64 model::calcFaceNum(const QString &objFilePath) {
  QFile objFile(objFilePath);
  if (objFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qint64 fileSize = objFile.size();
    objFile.close();
    return fileSize;
  }
  return 0;
}

bool model::displayProgress(qint64 progressUpdateInterval) {
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

model::ModelData::ModelData(const QString &objFilePath): objFilePath(objFilePath) {
  loadVertices(objFilePath, vertices);
  QString mtlPath = retriveMtlPath(objFilePath);
  logMessage("mtlPath: " + mtlPath, Qgis::MessageLevel::Info);
  loadMtl(mtlPath, material, texturePath);
  mBounds = calculateModelBounds();
}

model::ModelData::~ModelData() {
  vertices.clear();
}

Bounds model::ModelData::calculateModelBounds() {
  Bounds bounds;
  bounds.min = QVector3D(FLT_MAX, FLT_MAX, FLT_MAX);
  bounds.max = QVector3D(-FLT_MAX, -FLT_MAX, -FLT_MAX);

  for (const auto &v : vertices) {
      bounds.min.setX(qMin(bounds.min.x(), v.position.x()));
      bounds.min.setY(qMin(bounds.min.y(), v.position.y()));
      bounds.min.setZ(qMin(bounds.min.z(), v.position.z()));

      bounds.max.setX(qMax(bounds.max.x(), v.position.x()));
      bounds.max.setY(qMax(bounds.max.y(), v.position.y()));
      bounds.max.setZ(qMax(bounds.max.z(), v.position.z()));
  }

  bounds.center = QVector3D((bounds.min.x() + bounds.max.x()) / 2.0f, (bounds.min.y() + bounds.max.y()) / 2.0f, (bounds.min.z() + bounds.max.z()) / 2.0f);
  return bounds;
}

QVector3D model::ModelData::calculateModelCenter() {
  float minX = FLT_MAX, maxX = -FLT_MAX;
  float minY = FLT_MAX, maxY = -FLT_MAX;
  float minZ = FLT_MAX, maxZ = -FLT_MAX;

  for (const auto &v : vertices) {
      minX = qMin(minX, v.position.x());
      maxX = qMax(maxX, v.position.x());
      minY = qMin(minY, v.position.y());
      maxY = qMax(maxY, v.position.y());
      minZ = qMin(minZ, v.position.z());
      maxZ = qMax(maxZ, v.position.z());
  }
  return QVector3D((minX + maxX) / 2.0f, (minY + maxY) / 2.0f, (minZ + maxZ) / 2.0f);
}