package com.jarvis.facedemo

import android.Manifest
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.mutableStateOf
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.core.content.ContextCompat
import com.jarvis.facedemo.ui.theme.FaceDemoTheme
import com.jarvis.facekit.FaceKit

class SplashActivity : ComponentActivity() {
    private val requiredPermissions = listOf(
        Manifest.permission.CAMERA
    )

    private val permissionStatus = mutableStateOf(mapOf<String, Boolean>())

    private val requestPermissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestMultiplePermissions()
    ) { permissions ->
        updatePermissionStatus()
        if (allPermissionsGranted()) {
            navigateToSurfaceActivity()
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        
        setContent {
            FaceDemoTheme {
                Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                    SplashScreen(
                        modifier = Modifier.padding(innerPadding),
                        permissionStatus = permissionStatus.value,
                        activity = this
                    )
                }
            }
        }


        updatePermissionStatus()
        if (allPermissionsGranted()) {
            // 权限已授予，2秒后跳转
            startAutoNavigationTimer()
        } else {
            // 请求权限
            requestPermissions()
        }
    }

    private fun updatePermissionStatus() {
        val statusMap = mutableMapOf<String, Boolean>()
        for (permission in requiredPermissions) {
            statusMap[permission] = ContextCompat.checkSelfPermission(
                this, permission
            ) == PackageManager.PERMISSION_GRANTED
        }
        permissionStatus.value = statusMap
    }

    private fun allPermissionsGranted(): Boolean {
        return requiredPermissions.all {
            ContextCompat.checkSelfPermission(this, it) == PackageManager.PERMISSION_GRANTED
        }
    }

    private fun requestPermissions() {
        requestPermissionLauncher.launch(requiredPermissions.toTypedArray())
    }

    private fun startAutoNavigationTimer() {
        window.decorView.postDelayed({
            if (allPermissionsGranted()) {
                navigateToSurfaceActivity()
            }
        }, 2000)
    }

    private fun navigateToSurfaceActivity() {
        startActivity(Intent(this, SurfaceActivity::class.java))
        finish()
    }
}

@Composable
fun SplashScreen(
    modifier: Modifier = Modifier,
    permissionStatus: Map<String, Boolean> = emptyMap(),
    activity: SplashActivity? = null
) {
    Box(
        modifier = modifier.fillMaxSize(),
        contentAlignment = Alignment.Center
    ) {
        Column(
            horizontalAlignment = Alignment.CenterHorizontally,
            modifier = Modifier.padding(16.dp)
        ) {
            Text(
                text = "FaceKit Demo",
                fontSize = 32.sp
            )
            Text(
                text = "NativeMessage: ${FaceKit.Instance.stringFromJNI()}",
                fontSize = 14.sp,
                modifier = Modifier.padding(top = 16.dp)
            )
            
            Text(
                text = "权限状态",
                fontSize = 18.sp,
                modifier = Modifier.padding(top = 32.dp)
            )
            
            permissionStatus.forEach { (permission, isGranted) ->
                val permissionName = when (permission) {
                    android.Manifest.permission.CAMERA -> "相机权限"
                    else -> permission
                }
                val statusText = if (isGranted) "✓ 已授予" else "✗ 未授予"
                val statusColor = if (isGranted) Color.Green else Color.Red
                
                Text(
                    text = "$permissionName: $statusText",
                    fontSize = 14.sp,
                    color = statusColor,
                    modifier = Modifier.padding(top = 8.dp)
                )
            }
            
            Text(
                text = if (permissionStatus.values.all { it }) "权限已授予，2秒后跳转..." else "请授予所需权限",
                fontSize = 12.sp,
                modifier = Modifier.padding(top = 16.dp),
                color = if (permissionStatus.values.all { it }) Color.Green else Color.Blue
            )
        }
    }
}

@Preview(showBackground = true)
@Composable
fun SplashScreenPreview() {
    FaceDemoTheme {
        SplashScreen(
            permissionStatus = mapOf(
                android.Manifest.permission.CAMERA to true
            )
        )
    }
}