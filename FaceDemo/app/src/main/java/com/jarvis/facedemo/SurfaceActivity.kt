package com.jarvis.facedemo

import android.os.Bundle
import android.view.View
import androidx.activity.ComponentActivity
import androidx.activity.enableEdgeToEdge
import android.view.WindowManager
import android.widget.Button
import com.jarvis.facekit.FaceKit
import com.jarvis.facekit.egl.EGLSurfaceView
import com.jarvis.facekit.utils.FileUtils
import java.io.File

import android.widget.SeekBar
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.jarvis.facekit.Log

class SurfaceActivity : ComponentActivity() {
    private val TAG = "SurfaceActivity"
    private lateinit var glSurfaceView: EGLSurfaceView
    private lateinit var rvVideoClips: RecyclerView
    private lateinit var videoClipAdapter: VideoClipAdapter
    private lateinit var seekBar: SeekBar

    private lateinit var imagePath: String
    private lateinit var image1Path: String
    private lateinit var image2Path: String
    private var videoClipIds = ArrayList<Int>()

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

        setContentView(R.layout.activity_surface)
        val root = findViewById<View>(android.R.id.content)
        ViewCompat.setOnApplyWindowInsetsListener(root) { v, insets ->
            val bars = insets.getInsets(WindowInsetsCompat.Type.systemBars())
            v.setPadding(bars.left, bars.top, bars.right, bars.bottom)
            insets
        }
        glSurfaceView = findViewById(R.id.gl_surface_view)
        rvVideoClips = findViewById(R.id.rv_video_clips)
        rvVideoClips.layoutManager = LinearLayoutManager(this, LinearLayoutManager.HORIZONTAL, false)
        videoClipAdapter = VideoClipAdapter(videoClipIds) {
            clipId -> selectVideo(clipId)
        }
        rvVideoClips.adapter = videoClipAdapter
        seekBar = findViewById(R.id.seek_bar)

        seekBar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                glSurfaceView.setFaceLift(progress)
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {}
            override fun onStopTrackingTouch(seekBar: SeekBar?) {}
        })

        // Initialize with current progress
        glSurfaceView.post {
            glSurfaceView.setFaceLift(seekBar.progress)
        }
        showRetinaFace()

        // 全屏显示
        window.setFlags(
            WindowManager.LayoutParams.FLAG_FULLSCREEN,
            WindowManager.LayoutParams.FLAG_FULLSCREEN
        )
    }

    fun showRetinaFace() {
        val assetFiles = arrayOf(
            "test_image.png",
            "body_face.png",
            "retinaface.mnn",
            "pfld.mnn",
            "zqlandmark.mnn",
            "time.mov",
            "test_face.mp4"
        )

        for (fileName in assetFiles) {
            val file = File(filesDir, fileName)
            if (!file.exists()) {
                FileUtils.copyAssetResource2File(this, fileName, file.absolutePath)
            }
        }

        image1Path = File(filesDir, "test_image.png").absolutePath
        image2Path = File(filesDir, "body_face.png").absolutePath
        imagePath = image1Path

    }

    fun destroyImagePreview() {
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

    fun addVideo(button: View) {
        val size = videoClipIds.size
        var videoPath = "";

        Log.e(TAG, "addVideo:${videoPath}")
        var clipId = 0
        if (size >= 2) {
            if (size % 2 == 0) {
                videoPath = File(filesDir, "time.mov").absolutePath
            } else {
                videoPath = File(filesDir, "test_face.mp4").absolutePath
            }
            clipId = FaceKit.Instance.showVideoAfter(videoClipIds[size - 2], videoPath)
        } else {
            if (size % 2 == 1) {
                videoPath = File(filesDir, "time.mov").absolutePath
            } else {
                videoPath = File(filesDir, "test_face.mp4").absolutePath
            }
            clipId = FaceKit.Instance.showVideo(-1, videoPath, -1, 10000)
        }
        videoClipIds.add(clipId)
        videoClipAdapter.notifyItemInserted(videoClipIds.size - 1)
        videoClipAdapter.setSelected(videoClipIds.size - 1)
    }

    fun setScaleType(button: View) {
     
    }

    fun selectVideo(videoClipId: Int) {
        // Implementation for selecting video will be added here
        Log.d("SurfaceActivity", "Selected video clip id: $videoClipId")
        FaceKit.Instance.touchVideo(videoClipId)
    }
}
