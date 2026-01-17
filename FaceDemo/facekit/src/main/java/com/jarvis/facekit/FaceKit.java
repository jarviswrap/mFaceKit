package com.jarvis.facekit;

import android.util.Log;

public class FaceKit {
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
        System.loadLibrary("facekit");
        System.loadLibrary("MNN");
        System.loadLibrary("MNNOpenCV");
    }

    /**
     * A native method that is implemented by the 'facekit' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public native void setModelDir(String filePath);
}