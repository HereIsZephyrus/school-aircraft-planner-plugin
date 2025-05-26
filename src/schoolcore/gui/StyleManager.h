#pragma once
#include <QString>

class StyleManager {
public:
    static void initializeStyle();
    static QString loadQSS(const QString& path);
};
