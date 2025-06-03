#include "SharedContextManager.h"
#include "../log/QgisDebug.h"

SharedContextManager::SharedContextManager()
    : mSharedContext(nullptr)
    , mOffscreenSurface(nullptr)
    , mIsInitialized(false) {
}

SharedContextManager::~SharedContextManager() {
    cleanup();
}

bool SharedContextManager::initialize(QOpenGLContext* mainContext) {
    if (mIsInitialized) {
        return true;
    }

    if (!mainContext) {
        logMessage("Cannot create shared context: main context is null", Qgis::MessageLevel::Critical);
        return false;
    }

    // Create shared context
    mSharedContext = new QOpenGLContext();
    mSharedContext->setFormat(mainContext->format());
    mSharedContext->setShareContext(mainContext);

    if (!mSharedContext->create()) {
        logMessage("Failed to create shared context", Qgis::MessageLevel::Critical);
        cleanup();
        return false;
    }

    if (mSharedContext->shareContext() != mainContext) {
        logMessage("Context sharing failed", Qgis::MessageLevel::Critical);
        cleanup();
        return false;
    }

    // Create offscreen surface
    mOffscreenSurface = new QOffscreenSurface();
    mOffscreenSurface->setFormat(mSharedContext->format());
    mOffscreenSurface->create();

    if (!mOffscreenSurface->isValid()) {
        logMessage("Failed to create offscreen surface", Qgis::MessageLevel::Critical);
        cleanup();
        return false;
    }

    mIsInitialized = true;
    logMessage("Shared context manager initialized successfully", Qgis::MessageLevel::Success);
    return true;
}

void SharedContextManager::cleanup() {
    logMessage("ready to cleanup shared context manager", Qgis::MessageLevel::Info);
    
    if (mSharedContext) {
        if (mSharedContext->isValid())
            mSharedContext->doneCurrent();
        mSharedContext = nullptr;
    }

    if (mOffscreenSurface) {
        delete mOffscreenSurface;
        mOffscreenSurface = nullptr;
    }

    mIsInitialized = false;
    logMessage("Shared context manager cleaned up", Qgis::MessageLevel::Success);
}

bool SharedContextManager::isValid() const {
    return mIsInitialized && mSharedContext && mSharedContext->isValid() && 
           mOffscreenSurface && mOffscreenSurface->isValid();
}

bool SharedContextManager::makeCurrent() {
    if (!isValid()) {
        logMessage("Cannot make current: shared context manager is not valid", Qgis::MessageLevel::Critical);
        return false;
    }
    return mSharedContext->makeCurrent(mOffscreenSurface);
}

void SharedContextManager::doneCurrent() {
    if (mSharedContext && mSharedContext->isValid()) {
        mSharedContext->doneCurrent();
    }
} 