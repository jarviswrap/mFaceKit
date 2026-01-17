package com.jarvis.facedemo

import android.opengl.GLSurfaceView
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.enableEdgeToEdge
import android.view.WindowManager

class SurfaceActivity : ComponentActivity() {
    private lateinit var glSurfaceView: GLSurfaceView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        
        // 创建全屏的GLSurfaceView
        glSurfaceView = GLSurfaceView(this)
        glSurfaceView.setRenderer(object : GLSurfaceView.Renderer {
            override fun onSurfaceCreated(gl: javax.microedition.khronos.opengles.GL10?, config: javax.microedition.khronos.egl.EGLConfig?) {
                // OpenGL ES初始化
            }

            override fun onSurfaceChanged(gl: javax.microedition.khronos.opengles.GL10?, width: Int, height: Int) {
                // 处理Surface大小变化
            }

            override fun onDrawFrame(gl: javax.microedition.khronos.opengles.GL10?) {
                // 渲染帧
            }
        })
        
        setContentView(glSurfaceView)
        
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
