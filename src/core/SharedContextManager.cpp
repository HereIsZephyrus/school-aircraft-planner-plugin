#include "SharedContextManager.h"
#include "../log/QgisDebug.h"

SharedContextManager::SharedContextManager()
    : mSharedContext(nullptr)
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

    mSurface = mainContext->surface();

    if (!mSharedContext->makeCurrent(mSurface)) {
        logMessage("Failed to make shared context current", Qgis::MessageLevel::Critical);
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

    mIsInitialized = false;
    logMessage("Shared context manager cleaned up", Qgis::MessageLevel::Success);
}

bool SharedContextManager::isValid() const {
    return mIsInitialized && mSharedContext && mSharedContext->isValid();
}

bool SharedContextManager::makeCurrent() {
    if (!isValid()) {
        logMessage("Cannot make current: shared context manager is not valid", Qgis::MessageLevel::Critical);
        return false;
    }

    if (!mSharedContext || !mSharedContext->isValid()) {
        logMessage("Invalid context in SharedContextManager", Qgis::MessageLevel::Critical);
        return false;
    }

    if (!mSurface) {
        logMessage("No offscreen surface in SharedContextManager", Qgis::MessageLevel::Critical);
        return false;
    }

    bool success = mSharedContext->makeCurrent(mSurface);
    if (!success) {
        logMessage("Failed to make context current", Qgis::MessageLevel::Critical);
        return false;
    }

    if (!QOpenGLContext::currentContext()) {
        logMessage("Context not current after makeCurrent", Qgis::MessageLevel::Critical);
        return false;
    }

    return true;
}

void SharedContextManager::doneCurrent() {
    if (mSharedContext && mSharedContext->isValid()) {
        mSharedContext->doneCurrent();
    }
} 