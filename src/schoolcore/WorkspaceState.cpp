#include "WorkspaceState.h"

namespace ws {
    void initializeWorkspaceState() {
        PathManager::getInstance();
    }
}

ws::PathManager::PathManager() {
    mRootDir = QString::fromStdString(getenv("HOME"));
}

ws::PathManager::~PathManager() {
    mObjTexturePairs.clear();
}

void ws::PathManager::findAllObjAndTexturePaths() {
    QDir dir(mRootDir);
    QStringList objFiles = dir.entryList(QStringList() << "*.obj", QDir::Files);
    QStringList textureFiles = dir.entryList(QStringList() << "*.jpg", QDir::Files);
}

ObjTexturePair ws::PathManager::getObjTexturePair(int index) const {
    if (index < 0 || index >= mObjTexturePairs.size()) {
        throw std::out_of_range("Index out of range");
    }
    return mObjTexturePairs[index];
}
