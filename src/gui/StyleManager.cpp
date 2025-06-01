

// StyleManager.cpp
#include "StyleManager.h"
#include <QApplication>
#include <QFile>

void StyleManager::initializeStyle() {
  qApp->setStyleSheet(loadQSS(":/schoolcore/style/my.qss"));
}

QString StyleManager::loadQSS(const QString &path) {
  QFile file(path);
  file.open(QFile::ReadOnly);
  return QLatin1String(file.readAll());
}