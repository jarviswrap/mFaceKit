package com.jarvis.facekit;

import android.util.Log;

import com.jarvis.facekit.egl.EGLEnvironment;

public enum FaceKit {
    Instance;

    private static final String TAG = "FaceKitJava";
    // Used to load the 'facekit' library on application startup.
    static void loadGpuLibrary(String name) {
        try {
            System.loadLibrary(name);
        } catch (Throwable ce) {
            Log.e(TAG, "load " + name + " so exception=%s", ce);
        }
    }
    static {
//        NativeLib.test();
        System.loadLibrary("facekit");
        System.loadLibrary("MNN");
        System.loadLibrary("MNNOpenCV");
        System.loadLibrary("MNN_Express");
    }

    public EGLEnvironment touchEGL() {
        return new EGLEnvironment();
    }
    /**
     * A native method that is implemented by the 'facekit' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public native void setModelDir(String modelPath);

    public native void showImage(String imagePath);

    public native int showVideo(int trackId, String videoPath, long start, long duration);

    public native int showVideoAfter(int videoId, String videoPath);

    public native void touchVideo(int videoId);

    public native void tick();

}