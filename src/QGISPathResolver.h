#ifndef QGISPATHRESOLVER_H
#define QGISPATHRESOLVER_H

#include <QString>
#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QDebug>
#include "log/QgisDebug.h"

class QGISPathResolver {
public:
    static QString getQGISPrefixPath() {
        #ifdef Q_OS_WIN
            // Windows: 通常在 OSGeo4W 安装目录下
            return "C:/OSGeo4W64/apps/qgis";
        #elif defined(Q_OS_MACOS)
            // macOS: 通常在 /Applications/QGIS.app 内
            return "/Applications/QGIS.app/Contents/MacOS";
        #else
            // Linux/Unix: 通常在 /usr
            return "/usr";
        #endif
    }

    static QString getPluginsPath() {
        QString prefixPath = getQGISPrefixPath();
        #ifdef Q_OS_WIN
            return QDir(prefixPath).filePath("plugins");
        #elif defined(Q_OS_MACOS)
            return QDir(prefixPath).filePath("../PlugIns/qgis");
        #else
            return QDir(prefixPath).filePath("lib/qgis/plugins");
        #endif
    }

    static bool validatePath(const QString& path) {
        if (path.isEmpty()) {
            logMessage("Warning: Empty path provided for validation", Qgis::MessageLevel::Warning);
            return false;
        }

        QDir dir(path);
        if (!dir.exists()) {
            logMessage("Warning: Directory does not exist:" + path, Qgis::MessageLevel::Warning);
            return false;
        }

        bool hasQGISFiles = false;
        #ifdef Q_OS_WIN
            hasQGISFiles = dir.exists("qgis-bin.exe") || dir.exists("bin/qgis-bin.exe");
        #elif defined(Q_OS_MACOS)
            hasQGISFiles = dir.exists("QGIS") || dir.exists("../MacOS/QGIS");
        #else
            hasQGISFiles = dir.exists("bin/qgis") || dir.exists("lib/qgis");
        #endif

        if (!hasQGISFiles) {
            logMessage("Warning: Directory exists but does not appear to be a valid QGIS installation:" + path, Qgis::MessageLevel::Warning);
        }

        return hasQGISFiles;
    }

    static bool isValidQGISPath(const QString& path) {
        // 安全的路径格式验证
        static const QRegularExpression validPathRegex(
            #ifdef Q_OS_WIN
                "^[a-zA-Z]:\\\\(?:[^\\\\/:*?\"<>|\\r\\n]+\\\\)*[^\\\\/:*?\"<>|\\r\\n]*$"
            #else
                "^(/[^/\\0]+)+$"
            #endif
        );

        if (!validPathRegex.isValid()) {
            logMessage("Warning: Invalid regular expression pattern", Qgis::MessageLevel::Warning);
            return false;
        }

        QRegularExpressionMatch match = validPathRegex.match(QDir::toNativeSeparators(path));
        return match.hasMatch();
    }

    static QString findQGISInstallation() {
        QStringList searchPaths;
        
        QString envPath = QString::fromLocal8Bit(qgetenv("QGIS_PREFIX_PATH"));
        if (!envPath.isEmpty() && validatePath(envPath)) {
            return envPath;
        }

        #ifdef Q_OS_WIN
            // Windows 常见安装路径
            searchPaths << "C:/OSGeo4W64/apps/qgis"
                       << "C:/Program Files/QGIS"
                       << "D:/OSGeo4W64/apps/qgis";
        #elif defined(Q_OS_MACOS)
            // macOS 常见安装路径
            searchPaths << "/Applications/QGIS.app/Contents/MacOS"
                       << QDir::homePath() + "/Applications/QGIS.app/Contents/MacOS";
        #else
            // Linux 常见安装路径
            searchPaths << "/usr"
                       << "/usr/local"
                       << "/opt/qgis";
        #endif

        // 遍历所有可能的路径
        for (const QString& path : searchPaths) {
            if (isValidQGISPath(path) && validatePath(path)) {
                return path;
            }
        }
        QString defaultPath = getQGISPrefixPath();
        if (validatePath(defaultPath)) {
            return defaultPath;
        }

        logMessage("Warning: No valid QGIS installation found in common locations", Qgis::MessageLevel::Warning);
        return QString();
    }

    static QString sanitizePath(const QString& path) {
        return QDir::cleanPath(path);
    }
};

#endif // QGISPATHRESOLVER_H 