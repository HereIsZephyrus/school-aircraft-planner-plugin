#ifndef SHARED_CONTEXT_MANAGER_H
#define SHARED_CONTEXT_MANAGER_H

#include <QOpenGLContext>
#include <QSurface>
#include <memory>

class SharedContextManager {
private:
    SharedContextManager();
    ~SharedContextManager();

    QOpenGLContext* mSharedContext;
    QSurface* mSurface;
    bool mIsInitialized;

public:
    static SharedContextManager& getInstance() {
        static SharedContextManager instance;
        return instance;
    }

    SharedContextManager(const SharedContextManager&) = delete;
    SharedContextManager& operator=(const SharedContextManager&) = delete;

    bool initialize(QOpenGLContext* mainContext);
    void cleanup();
    
    bool isValid() const;
    QOpenGLContext* getSharedContext() const { return mSharedContext; }
    
    bool makeCurrent();
    void doneCurrent();
};

#endif // SHARED_CONTEXT_MANAGER_H 