package com.jarvis.facedemo

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.enableEdgeToEdge
import android.view.WindowManager
import com.jarvis.facekit.egl.EGLSurfaceView
import com.jarvis.facekit.utils.FileUtils
import java.io.File

class SurfaceActivity : ComponentActivity() {
    private lateinit var glSurfaceView: EGLSurfaceView
    private lateinit var imagePath: String

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()

        // 创建全屏的GLSurfaceView
        glSurfaceView = EGLSurfaceView(this)


        setContentView(glSurfaceView)
        val image1Path = filesDir.absolutePath + File.separator + "test_image.png"
        val image2Path = filesDir.absolutePath + File.separator + "body_face.png"

        if (!File(image1Path).exists()) {
            FileUtils.copyAssetResource2File(this, "test_image.png", image1Path)
        }
        if (!File(image2Path).exists()) {
            FileUtils.copyAssetResource2File(this, "body_face.png", image2Path)
        }

        imagePath = image1Path
        glSurfaceView.nativeShowImage(imagePath)

        val runnable = object : Runnable {
            override fun run() {
                imagePath = if (imagePath == image1Path) image2Path else image1Path
                glSurfaceView.nativeShowImage(imagePath)
                glSurfaceView.postDelayed(this, 2000)
            }
        }
        glSurfaceView.postDelayed(runnable, 2000)

        // 全屏显示
        window.setFlags(
            WindowManager.LayoutParams.FLAG_FULLSCREEN,
            WindowManager.LayoutParams.FLAG_FULLSCREEN
        )
    }

    override fun onResume() {
        super.onResume()
        glSurfaceView.onResume()
    }

    override fun onPause() {
        super.onPause()
        glSurfaceView.onPause()
    }
}
