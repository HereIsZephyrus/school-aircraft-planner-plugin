#include "CsvFlightImporter.h"
#include "../log/QgisDebug.h"
#include "../core/WorkspaceState.h"
#include "../opengl/Camera.h"
#include <QTextCodec>
#include <QDebug>
#include <QRegularExpression>
#include <cmath>


CsvFlightImporter::CsvFlightImporter() 
    : QObject(nullptr), mContext(nullptr), mHasValidData(false) {
    logMessage("CSV飞行导入器初始化", Qgis::MessageLevel::Info);
}

bool CsvFlightImporter::importPathFromCsv(const QString& csvFilePath) {
    logMessage(QString("开始导入CSV文件: %1").arg(csvFilePath), Qgis::MessageLevel::Info);
    
    // 清理之前的数据
    clearData();
    
    // 验证文件存在
    QFileInfo fileInfo(csvFilePath);
    if (!fileInfo.exists()) {
        QString errorMsg = QString("CSV文件不存在: %1").arg(csvFilePath);
        logMessage(errorMsg, Qgis::MessageLevel::Critical);
        emit pathImported(false, errorMsg);
        return false;
    }
    
    // 验证文件格式
    if (!validateCsvFormat(csvFilePath)) {
        QString errorMsg = "CSV文件格式无效";
        logMessage(errorMsg, Qgis::MessageLevel::Critical);
        emit pathImported(false, errorMsg);
        return false;
    }
    
    // 打开文件
    QFile csvFile(csvFilePath);
    if (!csvFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString errorMsg = QString("无法打开CSV文件: %1").arg(csvFilePath);
        logMessage(errorMsg, Qgis::MessageLevel::Critical);
        emit pathImported(false, errorMsg);
        return false;
    }
    
    QTextStream in(&csvFile);
    // 设置UTF-8编码
    in.setCodec("UTF-8");
    
    // 跳过头行
    QString header = in.readLine();
    logMessage(QString("CSV头行: %1").arg(header), Qgis::MessageLevel::Info);
    
    int lineNumber = 1;
    int successCount = 0;
    
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineNumber++;
        
        if (line.isEmpty()) {
            continue;
        }
        
        CsvPathPoint point;
        if (parseCsvLine(line, point)) {
            mPathPoints.append(point);
            successCount++;
        } else {
            logMessage(QString("解析第%1行失败: %2").arg(lineNumber).arg(line), 
                      Qgis::MessageLevel::Warning);
        }
    }
    
    csvFile.close();
    
    if (mPathPoints.isEmpty()) {
        QString errorMsg = "未找到有效的路径点数据";
        logMessage(errorMsg, Qgis::MessageLevel::Critical);
        emit pathImported(false, errorMsg);
        return false;
    }
    
    mHasValidData = true;
    
    // 创建显示对象
    createDisplayObjects();
    
    QString successMsg = QString("成功导入%1个路径点").arg(successCount);
    logMessage(successMsg, Qgis::MessageLevel::Success);
    emit pathImported(true, successMsg);
    
    return true;
}

bool CsvFlightImporter::parseCsvLine(const QString& line, CsvPathPoint& point) {
    // 预期格式: node_id,x,y,z,type,distance_from_start
    QStringList parts = line.split(',');
    
    if (parts.size() < 6) {
        return false;
    }
    
    bool ok;
    
    // 解析节点ID
    point.nodeId = parts[0].toInt(&ok);
    if (!ok) return false;
    
    // 解析坐标
    float x = parts[1].toFloat(&ok);
    if (!ok) return false;
    
    float y = parts[2].toFloat(&ok);
    if (!ok) return false;
    
    float z = parts[3].toFloat(&ok);
    if (!ok) return false;
    
    // CSV坐标与模型坐标系一致，直接使用，统一设置飞行高度为9米
    point.position = QVector3D(x, y, 9.0f);
    
    // 解析类型
    point.type = parts[4].trimmed();
    
    // 解析距离
    point.distanceFromStart = parts[5].toDouble(&ok);
    if (!ok) return false;
    
    return true;
}

bool CsvFlightImporter::validateCsvFormat(const QString& csvFilePath) {
    QFile csvFile(csvFilePath);
    if (!csvFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream in(&csvFile);
    in.setCodec("UTF-8");
    
    // 检查头行
    QString header = in.readLine();
    QStringList expectedHeaders = {"node_id", "x", "y", "z", "type", "distance_from_start"};
    QStringList actualHeaders = header.split(',');
    
    if (actualHeaders.size() < expectedHeaders.size()) {
        csvFile.close();
        return false;
    }
    
    // 简单检查头行是否包含预期字段
    for (const QString& expected : expectedHeaders) {
        bool found = false;
        for (const QString& actual : actualHeaders) {
            if (actual.trimmed().toLower().contains(expected.toLower())) {
                found = true;
                break;
            }
        }
        if (!found) {
            csvFile.close();
            return false;
        }
    }
    
    csvFile.close();
    return true;
}

QVector<QVector3D> CsvFlightImporter::getDisplayPath() const {
    QVector<QVector3D> path;
    for (const CsvPathPoint& point : mPathPoints) {
        path.append(point.position);
    }
    return path;
}



QVector3D CsvFlightImporter::getHomePoint() const {
    if (mPathPoints.isEmpty()) {
        return QVector3D(0, 0, 0);
    }
    return mPathPoints.first().position;
}

bool CsvFlightImporter::createFlightRoute() {
    if (!mHasValidData) {
        logMessage("无有效路径数据，无法创建飞行路线", Qgis::MessageLevel::Warning);
        return false;
    }
    
    if (!mContext) {
        logMessage("OpenGL上下文未设置", Qgis::MessageLevel::Critical);
        return false;
    }
    
    mContext->makeCurrent(mContext->surface());
    
    // 创建显示对象
    createDisplayObjects();
    
    logMessage("CSV飞行路线创建成功", Qgis::MessageLevel::Success);
    return true;
}

void CsvFlightImporter::createDisplayObjects() {
    if (!mContext || mPathPoints.isEmpty()) {
        return;
    }
    
    // 创建路径显示对象 - 用于可视化的连续路径
    QVector<QVector3D> visualPathPoints;
    for (const CsvPathPoint& point : mPathPoints) {
        visualPathPoints.append(point.position);
    }
    mDisplayPath = std::make_shared<gl::RoutePath>(visualPathPoints);
    
    // 创建起点显示对象
    QVector3D homePosition = getHomePoint();
    mHomePoint = std::make_shared<gl::SinglePoint>(homePosition, QVector4D(0.0f, 1.0f, 0.0f, 1.0f));
    
    logMessage("CSV路径显示对象创建完成", Qgis::MessageLevel::Info);
}

void CsvFlightImporter::startCsvFlightSimulation() {
    if (!mHasValidData) {
        logMessage("无有效路径数据，无法开始飞行模拟", Qgis::MessageLevel::Warning);
        return;
    }
    
    // 设置相机到起点
    setCameraToHome();
    
    // 创建一个临时的RoutePlanner路线来兼容现有的飞行模拟系统
    createTemporaryRoute();
    
    // 获取FlightManager实例并设置飞行参数
    wsp::FlightManager& flightManager = wsp::FlightManager::getInstance();
    
    // 确保基础飞行高度为9米
    flightManager.setBaseHeight(9.0);
    
    // 关闭手动模式
    if (flightManager.isManualMode()) {
        flightManager.setManualMode(false);
    }
    
    // 使用现有的AnimationManager系统开始模拟
    wsp::AnimationManager& animationManager = wsp::AnimationManager::getInstance();
    animationManager.startSimulation();
    
    emit flightSimulationStarted();
    logMessage("CSV路径飞行模拟已开始（飞行高度：9米）", Qgis::MessageLevel::Success);
}

void CsvFlightImporter::setCameraToHome() {
    if (mPathPoints.isEmpty()) {
        return;
    }
    
    Camera& camera = Camera::getInstance();
    QVector3D homePosition = getHomePoint();
    

        // 设置相机位置，稍微偏移以获得更好的视角
    QVector3D cameraPosition = homePosition + QVector3D(0, 0, 10);
    camera.setPosition(cameraPosition);

    logMessage(QString("相机位置设置到起点: (%1, %2, %3)")
               .arg(homePosition.x()).arg(homePosition.y()).arg(homePosition.z()), 
               Qgis::MessageLevel::Info);
}

void CsvFlightImporter::clearData() {
    mPathPoints.clear();
    mDisplayPath.reset();
    mHomePoint.reset();
    mHasValidData = false;
    
    logMessage("CSV数据已清理", Qgis::MessageLevel::Info);
}

bool CsvFlightImporter::hasValidPath() const {
    return mHasValidData && !mPathPoints.isEmpty();
}

QString CsvFlightImporter::getPathStatistics() const {
    if (mPathPoints.isEmpty()) {
        return "无路径数据";
    }
    
    int totalPoints = mPathPoints.size();
    int riskPoints = 0;
    double totalDistance = 0.0;
    
    for (const CsvPathPoint& point : mPathPoints) {
        if (point.type.contains("risk", Qt::CaseInsensitive)) {
            riskPoints++;
        }
        if (point.distanceFromStart > totalDistance) {
            totalDistance = point.distanceFromStart;
        }
    }
    
    QString stats = QString("路径统计信息:\n"
                           "总路径点数: %1\n"
                           "风险事件点数: %2\n"
                           "总路径长度: %3 米\n"
                           "起点坐标: (%4, %5, %6)")
                    .arg(totalPoints)
                    .arg(riskPoints)
                    .arg(totalDistance, 0, 'f', 2)
                    .arg(getHomePoint().x(), 0, 'f', 2)
                    .arg(getHomePoint().y(), 0, 'f', 2)
                    .arg(getHomePoint().z(), 0, 'f', 2);
    
    return stats;
}

void CsvFlightImporter::drawCsvPath(const QMatrix4x4& view, const QMatrix4x4& projection) {
    if (!mHasValidData) {
        return;
    }
    
    // 绘制路径
    if (mDisplayPath) {
        mDisplayPath->draw(view, projection);
    }
    
    // 绘制起点
    if (mHomePoint) {
        mHomePoint->draw(view, projection);
    }
}

void CsvFlightImporter::setContext(QOpenGLContext* context) {
    mContext = context;
    logMessage("OpenGL上下文已设置", Qgis::MessageLevel::Info);
}

void CsvFlightImporter::createTemporaryRoute() {
    if (!mHasValidData || !mContext) {
        return;
    }
    
    // 获取RoutePlanner实例
    RoutePlanner& routePlanner = RoutePlanner::getInstance();
    routePlanner.setContext(mContext);
    
    // 获取CSV路径数据
    QVector<QVector3D> csvPath = getDisplayPath();
    
    // 使用专门的方法创建CSV路线
    routePlanner.createCsvRoute(csvPath);
    
    logMessage("CSV路线已设置到RoutePlanner系统", Qgis::MessageLevel::Info);
}

void CsvFlightImporter::importAndStartFlight(const QString& csvFilePath) {
    if (importPathFromCsv(csvFilePath)) {
        if (createFlightRoute()) {
            startCsvFlightSimulation();
        }
    }
} 
