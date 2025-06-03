#ifndef SHARED_CONTEXT_MANAGER_H
#define SHARED_CONTEXT_MANAGER_H

#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <memory>

class SharedContextManager {
private:
    SharedContextManager();
    ~SharedContextManager();

    QOpenGLContext* mSharedContext;
    QOffscreenSurface* mOffscreenSurface;
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
    QOffscreenSurface* getOffscreenSurface() const { return mOffscreenSurface; }
    
    bool makeCurrent();
    void doneCurrent();
};

#endif // SHARED_CONTEXT_MANAGER_H 