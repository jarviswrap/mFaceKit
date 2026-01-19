package com.jarvis.facekit.egl;

import android.opengl.EGL14;
import android.opengl.GLSurfaceView;

import com.jarvis.facekit.FaceKit;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;

public class EGLConfigChooser implements GLSurfaceView.EGLConfigChooser {

    public enum ConfigIndex{
        RED_SIZE,
        GREEN_SIZE,
        BLUE_SIZE,
        ALPHA_SIZE,
        DEPTH_SIZE,
        STENCIL_SIZE
    }

    private final int[] mConfigs = new int[ConfigIndex.values().length];

    public EGLConfigChooser() {
        int[] configs = FaceKit.Instance.touchEGL().getEGLConfigs();
        if (configs != null && configs.length == mConfigs.length) {
            System.arraycopy(configs, 0, mConfigs, 0, mConfigs.length);
        }
    }

    @Override
    public EGLConfig chooseConfig(EGL10 egl10, EGLDisplay eglDisplay) {
        int[] configSpec = new int[] {
                EGL10.EGL_RED_SIZE, mConfigs[ConfigIndex.RED_SIZE.ordinal()],
                EGL10.EGL_GREEN_SIZE, mConfigs[ConfigIndex.GREEN_SIZE.ordinal()],
                EGL10.EGL_BLUE_SIZE, mConfigs[ConfigIndex.BLUE_SIZE.ordinal()],
                EGL10.EGL_ALPHA_SIZE, mConfigs[ConfigIndex.ALPHA_SIZE.ordinal()],
                EGL10.EGL_DEPTH_SIZE, mConfigs[ConfigIndex.DEPTH_SIZE.ordinal()],
                EGL10.EGL_STENCIL_SIZE, mConfigs[ConfigIndex.STENCIL_SIZE.ordinal()],
                EGL10.EGL_RENDERABLE_TYPE, EGL14.EGL_OPENGL_ES2_BIT,
                EGL14.EGL_NONE
        };
        int[] num_configs = new int[1];
        if (!egl10.eglChooseConfig(eglDisplay, configSpec, null, 0, num_configs)) { // 第一步只查询数量

        }
        return null;
    }


}
