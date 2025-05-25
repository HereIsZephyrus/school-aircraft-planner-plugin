#include <QString>
#include <QList>
#include <QDir>
//use a singleton to manage the workspace state
typedef std::pair<QString, QString> ObjTexturePair; // the first is the obj path, the second is the texture path
namespace ws{
void initializeWorkspaceState();
class PathManager {
private:
    PathManager();
    ~PathManager();
public:
    static PathManager& getInstance() {
        static PathManager instance;
        return instance;
    }
    PathManager(const PathManager&) = delete;
    PathManager& operator=(const PathManager&) = delete;
    void setRootDir(const QString& rootDir) {mRootDir = rootDir;}
    void findAllObjAndTexturePaths();
    QString getRootDir() const {return mRootDir;}
    void addObjTexturePair(const ObjTexturePair& objTexturePair) {mObjTexturePairs.append(objTexturePair);}
    QList<ObjTexturePair> getObjTexturePairs() const {return mObjTexturePairs;}
    ObjTexturePair getObjTexturePair(int index) const;
private:
    QString mRootDir;
    //QString mPath3D;
    //QString mPathTexture;
    QList<ObjTexturePair> mObjTexturePairs;
};
}