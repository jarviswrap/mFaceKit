package com.jarvis.facedemo

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.enableEdgeToEdge
import android.view.WindowManager
import com.jarvis.facekit.FaceKit
import com.jarvis.facekit.egl.EGLSurfaceView
import com.jarvis.facekit.utils.FileUtils
import java.io.File

class SurfaceActivity : ComponentActivity() {
    private lateinit var glSurfaceView: EGLSurfaceView

    private lateinit var imagePath: String
    private lateinit var image1Path: String
    private lateinit var image2Path: String
    val runnable = object : Runnable {
        override fun run() {
            imagePath = if (imagePath == image1Path) image2Path else image1Path
            FaceKit.Instance.showImage(imagePath)
            glSurfaceView.postDelayed(this, 2000)
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()

        // 创建全屏的GLSurfaceView
        glSurfaceView = EGLSurfaceView(this)
        setContentView(glSurfaceView)
        switchImagesAuto()
        showRetinaFace()

        // 全屏显示
        window.setFlags(
            WindowManager.LayoutParams.FLAG_FULLSCREEN,
            WindowManager.LayoutParams.FLAG_FULLSCREEN
        )
    }

    fun showRetinaFace() {
        image1Path = filesDir.absolutePath + File.separator + "test_image.png"
        image2Path = filesDir.absolutePath + File.separator + "body_face.png"
        if (!File(image1Path).exists()) {
            FileUtils.copyAssetResource2File(this, "test_image.png", image1Path)
        }
        if (!File(image2Path).exists()) {
            FileUtils.copyAssetResource2File(this, "body_face.png", image2Path)
        }
        imagePath = image1Path
        val modelPath = filesDir.absolutePath + File.separator + "retinaface.mnn"
        if (!File(modelPath).exists()) {
            FileUtils.copyAssetResource2File(this, "retinaface.mnn", modelPath)
        }
        FaceKit.Instance.setModelDir(modelPath)
        FaceKit.Instance.showImage(imagePath)
        glSurfaceView.postDelayed(runnable, 2000)
    }

    fun switchImagesAuto() {
//        image1Path = filesDir.absolutePath + File.separator + "test_image.png"
//        image2Path = filesDir.absolutePath + File.separator + "body_face.png"
//        if (!File(image1Path).exists()) {
//            FileUtils.copyAssetResource2File(this, "test_image.png", image1Path)
//        }
//        if (!File(image2Path).exists()) {
//            FileUtils.copyAssetResource2File(this, "body_face.png", image2Path)
//        }
//        imagePath = image1Path
//        glSurfaceView.demoShowImage(imagePath)
//        glSurfaceView.postDelayed(runnable, 2000)
    }

    fun destroyImagePreview() {
//        glSurfaceView.removeCallbacks { runnable }
//        glSurfaceView.demoShowImage(null)
    }

    override fun onResume() {
        super.onResume()
        glSurfaceView.onResume()
    }

    override fun onPause() {
        super.onPause()
        glSurfaceView.onPause()
    }

    override fun onDestroy() {
        destroyImagePreview()
        super.onDestroy()
    }
}
