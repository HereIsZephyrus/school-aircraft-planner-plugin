#ifndef STYLEMANAGER_H
#define STYLEMANAGER_H
#include <QString>

class StyleManager {
public:
  static void initializeStyle();
  static QString loadQSS(const QString &path);
};

#endif // STYLEMANAGER_H