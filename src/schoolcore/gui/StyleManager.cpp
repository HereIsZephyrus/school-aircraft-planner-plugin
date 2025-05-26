

// StyleManager.cpp
#include "StyleManager.h"
#include <QFile>
#include <QApplication>

void StyleManager::initializeStyle() {
    qApp->setStyleSheet(loadQSS(":/style/my.qss"));
}

QString StyleManager::loadQSS(const QString& path) {
    QFile file(path);
    file.open(QFile::ReadOnly);
    return QLatin1String(file.readAll());
}