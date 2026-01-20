package com.jarvis.facekit.egl;

import android.opengl.EGL14;
import android.opengl.GLSurfaceView;

import com.jarvis.facekit.FaceKit;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;


public class EGLConfigSelector implements GLSurfaceView.EGLConfigChooser {

    public EGLConfigSelector() {}

    @Override
    public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {
        int mRedSize = 8;
        int mGreenSize = 8;
        int mBlueSize = 8;
        int mAlphaSize = 8;
        int mDepthSize = 16;
        int mStencilSize = 0;
        int[] configSpec = new int[]{
                EGL10.EGL_RED_SIZE, mRedSize,
                EGL10.EGL_GREEN_SIZE, mGreenSize,
                EGL10.EGL_BLUE_SIZE, mBlueSize,
                EGL10.EGL_ALPHA_SIZE, mAlphaSize,
                EGL10.EGL_DEPTH_SIZE, mDepthSize,
                EGL10.EGL_STENCIL_SIZE, mStencilSize,
                EGL10.EGL_RENDERABLE_TYPE, EGL14.EGL_OPENGL_ES2_BIT,
                EGL14.EGL_NONE
        };
        int[] num_config = new int[1];
        if (!egl.eglChooseConfig(display, configSpec, null, 0, num_config)) { // 第一步：只问数量
            throw new IllegalArgumentException("eglChooseConfig failed");
        }
        int numConfigs = num_config[0];
        if (numConfigs <= 0) {
            throw new IllegalArgumentException("No configs match configSpec");
        }
        EGLConfig[] configs = new EGLConfig[numConfigs];
        if (!egl.eglChooseConfig(display, configSpec, configs, numConfigs, num_config)) { // 第二步：真正取出EGLConfig列表
            throw new IllegalArgumentException("eglChooseConfig#2 failed");
        }

        for (EGLConfig config : configs) {

            int d = findConfigAttrib(egl, display, config, EGL10.EGL_DEPTH_SIZE);
            int s = findConfigAttrib(egl, display, config, EGL10.EGL_STENCIL_SIZE);

            // 我们需要至少指定数量的位数
            if (d < mDepthSize || s < mStencilSize)
                continue;

            // 我们想要确切的大小或更大
            int r = findConfigAttrib(egl, display, config, EGL10.EGL_RED_SIZE);
            int g = findConfigAttrib(egl, display, config, EGL10.EGL_GREEN_SIZE);
            int b = findConfigAttrib(egl, display, config, EGL10.EGL_BLUE_SIZE);
            int a = findConfigAttrib(egl, display, config, EGL10.EGL_ALPHA_SIZE);

            if (r == mRedSize && g == mGreenSize && b == mBlueSize && a == mAlphaSize) {
                return config;
            }
        }

        // 如果找不到完全匹配的，返回第一个
        return configs[0];
    }

    private int findConfigAttrib(EGL10 egl, EGLDisplay display, EGLConfig config,
                                 int attribute) {
        int[] value = new int[1];
        if (egl.eglGetConfigAttrib(display, config, attribute, value)) {
            return value[0];
        }
        return 0;
    }
}
