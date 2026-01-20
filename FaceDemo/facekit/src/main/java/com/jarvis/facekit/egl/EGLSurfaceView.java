package com.jarvis.facekit.egl;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.SurfaceHolder;

import com.jarvis.facekit.FaceKit;
import com.jarvis.facekit.Log;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;

public class EGLSurfaceView extends GLSurfaceView {
    private final String TAG = "EGLSurfaceView";
    private final EGLEnvironment mEglEnvironment = FaceKit.Instance.touchEGL();

    public EGLSurfaceView(Context context) {
        super(context);
        initialize();
    }

    public EGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initialize();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        super.surfaceDestroyed(holder);
        mEglEnvironment.release();
    }

    private void initialize() {
        // 设置 EGL 配置
        setEGLConfigChooser(new EGLConfigSelector()); // EGLConfig真正的选择选择逻辑在cpp代码中

        // 设置 EGL Context Factory，创建 shared context
        setEGLContextFactory(new EGLContextFactory(){
            @Override
            public EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig eglConfig) {
                if (mEglEnvironment.createEGLContext(0, EGLEnvironment.EGLScene.BACKGROUND_RENDER | EGLEnvironment.EGLScene.DISPLAY) &&
                    mEglEnvironment.createEGLPBufferSurface(4, 4, true)) { //先创建小的PBuffer避免报错
                    return egl.eglGetCurrentContext();
                }
                return EGL10.EGL_NO_CONTEXT;
            }

            @Override
            public void destroyContext(EGL10 egl, EGLDisplay display, EGLContext context) {
                mEglEnvironment.destroyEGLContext();
            }
        });

        setEGLWindowSurfaceFactory(new EGLWindowSurfaceFactory() {
            @Override
            public EGLSurface createWindowSurface(EGL10 egl, EGLDisplay display, EGLConfig config, Object nativeWindow) {
                Surface nativeSurface = null;
                if (nativeWindow instanceof SurfaceHolder) {
                    SurfaceHolder holder = (SurfaceHolder) nativeWindow;
                    nativeSurface = holder.getSurface();
                    Log.w(TAG, "[createWindowSurface] nativeWindow is SurfaceHolder");
                } else if (nativeWindow instanceof Surface) {
                    nativeSurface = (Surface) nativeWindow;
                    Log.w(TAG, "[createWindowSurface] nativeWindow is Surface");
                }
                if (mEglEnvironment.createEGLWindowSurface(nativeSurface, true)) {
                    return egl.eglGetCurrentSurface(EGL10.EGL_DRAW);
                }
                return EGL10.EGL_NO_SURFACE;
            }

            @Override
            public void destroySurface(EGL10 egl, EGLDisplay display, EGLSurface surface) {
                mEglEnvironment.destroyEGLSurface();
            }
        });
    }


}
