package com.jarvis.facekit.egl;

import android.view.Surface;

import com.jarvis.facekit.Log;

public class EGLEnvironment {
    public interface EGLScene{
        int BACKGROUND_RENDER = 1;
        int DISPLAY = 2;
        int HARDWARE_ENCODE = 4;
    }

    private final String TAG = "EGLEnvironment";
    private final long mEGLEnvironment;
    private long mContextPtr = 0;
    private long mContextSharedFrom = 0;
    private long mSurfacePtr = 0;
    private int mEGLScene = EGLScene.BACKGROUND_RENDER | EGLScene.DISPLAY;

    public EGLEnvironment() {
        mEGLEnvironment = createEGLEnvironment();
    }

    public boolean createEGLContext(long sharedContextPtr, int eglScene) {
        if (mContextPtr != 0) {
            Log.e(TAG, "createEGLContext twice");
            return false;
        }
        if (mEGLEnvironment == 0) {
            Log.e(TAG, "createEGLContext when init failed");
            return false;
        }
        mEGLScene = eglScene;
        mContextSharedFrom = sharedContextPtr;
        mContextPtr = nativeEGLCreateContext(mEGLEnvironment, sharedContextPtr, eglScene);
        return mContextPtr != 0;

    }

    public boolean createEGLPBufferSurface(int width, int height, boolean makeCurrent) {
        if (mEGLEnvironment == 0) {
            Log.e(TAG, "createEGLPBufferSurface when init failed");
            return false;
        }
        if (mContextPtr == 0) {
            mContextPtr = nativeEGLCreateContext(mEGLEnvironment, mContextSharedFrom, mEGLScene);
        }
        mSurfacePtr = nativeEGLCreatePBufferSurface(mEGLEnvironment, width, height);
        if (makeCurrent) {
            return eglMakeCurrent();
        } else {
            return mContextPtr != 0 && mSurfacePtr != 0;
        }
    }

    public void destroyEGLContext() {
        if (mEGLEnvironment == 0) {
            Log.e(TAG, "destroyEGLContext when init failed");
            return;
        }
        if (nativeEGLDestroyContext(mEGLEnvironment)) {
            mContextPtr = 0;
        }
    }

    public boolean eglMakeCurrent() {
        if (mEGLEnvironment == 0) {
            Log.e(TAG, "eglMakeCurrent when init failed");
            return false;
        }
        if (mContextPtr == 0 || mSurfacePtr == 0) {
            Log.e(TAG, "eglMakeCurrent when mContextPtr:" + mContextPtr + ", mSurfacePtr:" + mSurfacePtr);
            return false;
        }
        return nativeEGLMakeCurrent(mEGLEnvironment);
    }

    public boolean createEGLWindowSurface(Surface surface, boolean makeCurrent) {
        if (mEGLEnvironment == 0) {
            Log.e(TAG, "recreateEGLSurface when init failed");
            return false;
        }
        if (mContextPtr == 0) {
            mContextPtr = nativeEGLCreateContext(mEGLEnvironment, mContextSharedFrom, mEGLScene);
        }
        mSurfacePtr = nativeEGLCreateWindowSurface(mEGLEnvironment, surface);
        if (makeCurrent) {
            return eglMakeCurrent();
        } else {
            return mContextPtr != 0 && mSurfacePtr != 0;
        }
    }

    public void destroyEGLSurface() {
        if (mEGLEnvironment == 0) {
            Log.e(TAG, "destroyEGLSurface when init failed");
            return;
        }
        if (nativeEGLDestroySurface(mEGLEnvironment)) {
            mSurfacePtr = 0;
        }
    }

    public int getEGLSurfaceWidth() {
        if (mEGLEnvironment == 0) {
            Log.e(TAG, "getEGLSurfaceWidth when init failed");
            return 0;
        }
        if (mSurfacePtr == 0) {
            Log.e(TAG, "getEGLSurfaceWidth when mSurfacePtr == 0");
            return 0;
        }
        return getEGLSurfaceWidth(mEGLEnvironment);
    }

    public int getEGLSurfaceHeight() {
        if (mEGLEnvironment == 0) {
            Log.e(TAG, "getEGLSurfaceHeight when init failed");
            return 0;
        }
        if (mSurfacePtr == 0) {
            Log.e(TAG, "getEGLSurfaceHeight when mSurfacePtr == 0");
            return 0;
        }
        return getEGLSurfaceHeight(mEGLEnvironment);
    }

    public void release() {
        if (mEGLEnvironment == 0) {
            Log.e(TAG, "release when init failed");
            return;
        }
        nativeRelease(mEGLEnvironment);
    }

    private native long createEGLEnvironment();
    private native void nativeRelease(long environmentPtr);

    private native long nativeEGLCreateContext(long environmentPtr, long sharedContextPtr, int eglScene);
    private native boolean nativeEGLDestroyContext(long environmentPtr);

    private native long nativeEGLCreateWindowSurface(long environmentPtr, Surface surface);
    private native long nativeEGLCreatePBufferSurface(long environmentPtr, int width, int height);
    private native boolean nativeEGLDestroySurface(long environmentPtr);

    private native boolean nativeEGLMakeCurrent(long environmentPtr);

    private native int getEGLSurfaceWidth(long environmentPtr);
    private native int getEGLSurfaceHeight(long environmentPtr);
}
