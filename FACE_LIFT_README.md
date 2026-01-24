# 瘦脸功能 (Face Lift) 技术文档

本文档详细介绍了 mFaceKit 项目中瘦脸（Face Lift）功能的实现原理、架构设计及处理流程。

## 1. 功能概述

瘦脸功能基于 OpenGL ES 3.0 实现，利用人脸关键点检测（Face Landmark Detection）定位脸部轮廓，并通过 Fragment Shader 对图像像素进行局部网格变形（Warping），从而在视觉上实现面部轮廓的收缩效果。支持实时调节瘦脸强度。

## 2. 核心架构

整个功能链路贯穿 Android Java 层、JNI 层以及 C++ 核心渲染层。

### 2.1 模块交互图

```mermaid
graph TD
    A[UI Layer (SurfaceActivity)] -->|SeekBar Progress| B[EGLSurfaceView (Java)]
    B -->|JNI Call| C[EGLSurfaceView (C++)]
    C -->|Update Intensity| D[PixelData]
    
    E[Camera/Image Source] -->|Raw Data| D
    D -->|Process| F[Landmark Detection (PFLD/ZQ)]
    F -->|Keypoints| D
    
    D -->|Render Frame| G[PixelRender]
    G -->|1. Draw to FBO| H[FBO Texture (Clean RGB)]
    H -->|2. Apply Effect| I[FaceLiftRender]
    I -->|3. Draw to Screen| J[Screen/Window]
```

## 3. 详细实现步骤

### 步骤 1：用户交互与参数传递 (UI & JNI)

**文件**: 
- `app/.../SurfaceActivity.kt`
- `facekit/.../EGLSurfaceView.java`
- `facekit/.../EGLSurfaceView.cpp`

1.  **UI 控制**: 用户拖动 SeekBar (0-100)。
2.  **归一化**: Java 层接收进度值，将其归一化为 `0.0f - 1.0f` 的浮点数。
3.  **JNI 传递**: 调用 native 方法 `setFaceLiftIntensity`，将强度值存储在 C++ 层的 `EGLSurfaceView` 对象中。
4.  **帧数据同步**: 在每一帧渲染开始时（`onConsumeData`），将当前的 `intensity` 注入到 `PixelData` 对象中，确保渲染线程能获取到最新的 UI 状态。

### 步骤 2：人脸关键点检测 (Core)

**文件**: 
- `facekit/.../PFLDLandmarker.cpp` (或 ZQLandmarker)

在渲染之前，必须先获取人脸关键点：
1.  **模型推理**: 使用 MNN 运行 PFLD (98点) 或 ZQ (106点) 模型。
2.  **坐标映射**: 将模型输出的归一化坐标映射回原图坐标系。
3.  **数据存储**: 关键点存储在 `PixelData::bboxes` 结构中。

### 步骤 3：渲染管线准备 (PixelRender)

**文件**: `facekit/.../PixelRender.cpp`

为了保证瘦脸效果的正确性，不能直接在 YUV 数据上操作，需要先转换为 RGB 纹理。

1.  **FBO 初始化**: 创建一个与图像尺寸一致的 Framebuffer Object (FBO)。
2.  **格式转换**: 将输入的 YUV (NV21/I420) 或 RGB 数据绘制到 FBO 上。
    - 这一步产生了一个纯净的、方向正确的 RGB 纹理 (`mFBOTexture`)。
    - 这一步通常绘制全屏四边形（Scale=1.0），不进行裁剪。

### 步骤 4：瘦脸特效渲染 (FaceLiftRender)

**文件**: 
- `facekit/.../FaceLiftRender.hpp`
- `facekit/.../FaceLiftRender.cpp`

这是核心算法所在。`PixelRender` 调用 `FaceLiftRender::draw`，传入 FBO 纹理和关键点数据。

#### 4.1 数据准备 (Uniforms)
对于检测到的每一张人脸，计算以下 Shader Uniform 参数：
*   **Left/Right Cheek**: 左右脸颊边缘的关键点坐标（归一化 UV 坐标）。
*   **Center**: 脸部中心点（通常取鼻尖或两眼中间）。
*   **Radius**: 变形的影响半径，通常根据人脸宽度动态计算。
*   **Intensity**: 用户设置的全局强度。

#### 4.2 Fragment Shader 变形算法
Shader 对每个像素进行处理（Pixel Shader）：

1.  **逆向映射原理**: 
    - 我们想要将脸颊像素“推”向中心（变瘦）。
    - 在 Fragment Shader 中，我们需要决定当前像素 `P` 应该显示什么颜色。
    - 如果 `P` 在脸颊区域，原本应该在更外侧 `P_out` 的像素现在移到了 `P`。
    - 因此，我们需要去采样 `P` 加上一个指向脸颊外侧的偏移量 `Offset`。
    - `Coord_Sample = Coord_Current - Direction_Inwards * Strength` (或者 `+ Direction_Outwards`)。

2.  **算法流程**:
    ```glsl
    vec2 coord = vTexCoord;
    for (each face) {
        // 计算当前像素到左/右脸颊目标点的距离
        float dist = distance(coord, cheekPoint);
        
        // 如果在影响半径内
        if (dist < radius) {
            // 计算变形权重 (平滑过渡，中心最强，边缘为0)
            float alpha = smoothstep(radius, 0.0, dist);
            
            // 计算位移向量：从脸颊指向中心
            vec2 dir = normalize(center - cheekPoint);
            
            // 偏移采样坐标：向反方向（脸颊外侧）偏移
            // 从而将外侧的纹理“拉”进来
            coord -= dir * alpha * intensity * CONSTANT_FACTOR;
        }
    }
    fragColor = texture(inputTexture, coord);
    ```

### 步骤 5：屏幕显示与比例适配

**文件**: `facekit/.../PixelRender.cpp`

1.  **视口计算**: 根据 `ScaleType` (FitCenter, CenterCrop, FitXY) 计算最终在屏幕上的 `glViewport` 区域。
2.  **上屏绘制**: 使用经过 Shader 处理后的纹理绘制到屏幕上。
    - 此时使用的是通过 `FaceLiftRender` 处理过的纹理（或者直接在这一步应用 Shader，如果在 FaceLiftRender 中直接上屏）。
    - *当前实现*: `FaceLiftRender` 直接负责最后一步的上屏绘制，它根据计算出的 Viewport 将 FBO 纹理画到屏幕上。

## 4. 关键点索引参考

算法依赖特定的关键点索引（以 PFLD 98点为例）：

*   **0-32**: 下颌轮廓线 (Jawline)
*   **16**: 下巴尖 (Chin)
*   **4**: 左脸颊边缘 (Left Cheek, approx)
*   **28**: 右脸颊边缘 (Right Cheek, approx)
*   **51-54**: 鼻子 (Nose)，其中 54 为鼻尖

## 5. 性能优化点

1.  **FBO 复用**: 避免每帧创建销毁 FBO。
2.  **Shader 计算**: 尽量减少 Shader 中的条件分支，使用 `step` 或 `mix` 函数。
3.  **纹理上传**: 仅在人脸数据变化时更新 Uniform 数组。
