package com.jarvis.facedemo

import android.opengl.GLSurfaceView
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
        imagePath = filesDir.absolutePath + File.separator + "test_image.png"
        FileUtils.copyAssetResource2File(this, "test_image.png", imagePath)
        // 全屏显示
        window.setFlags(
            WindowManager.LayoutParams.FLAG_FULLSCREEN,
            WindowManager.LayoutParams.FLAG_FULLSCREEN
        )
    }

    override fun onResume() {
        super.onResume()
        glSurfaceView.onResume()
        glSurfaceView.nativeShowImage(imagePath)
    }

    override fun onPause() {
        super.onPause()
        glSurfaceView.onPause()
    }
}
