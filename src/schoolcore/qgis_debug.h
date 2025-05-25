#pragma once
#include <QString>
#include <qdebug.h>
#include <qfiledialog.h>
#include <qgsmessagelog.h>
#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

void logMessage(const QString &message,Qgis::MessageLevel level = Qgis::MessageLevel::Info);