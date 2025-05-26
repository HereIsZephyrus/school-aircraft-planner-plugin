#include "qgis_debug.h"

void logMessage(const QString &message, Qgis::MessageLevel level) {
    QgsMessageLog::logMessage(message, "SchoolPlugin3D", level);
    
    QString logDir = QDir::homePath() + "/.3Dschool/logs";
    QDir().mkpath(logDir);
    
    QString logFile = logDir + "/3Dschool_" + QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm") + ".log";
    QFile file(logFile);
    
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&file);
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        QString levelStr;
        
        switch (level) {
            case Qgis::MessageLevel::Info:
                levelStr = "INFO";
                break;
            case Qgis::MessageLevel::Warning:
                levelStr = "WARNING";
                break;
            case Qgis::MessageLevel::Success:
                levelStr = "SUCCESS";
                break;
            case Qgis::MessageLevel::Critical:
                levelStr = "CRITICAL";
                break;
            case Qgis::MessageLevel::NoLevel:
                levelStr = "NO LEVEL";
                break;
            default:
                levelStr = "INFO";
        }
        
        stream << timestamp << " [" << levelStr << "] " << message << "\n";
        file.close();
    }
}