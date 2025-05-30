#ifndef QGISDEBUG_H
#define QGISDEBUG_H
#include <QString>
#include <qdebug.h>
//#include <qfiledialog.h>
#include <qgis.h>
#include <qgsmessagelog.h>
#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

void logMessage(const QString &message,Qgis::MessageLevel level = Qgis::MessageLevel::Info);

#endif // QGISDEBUG_H