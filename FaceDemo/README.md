# 预编译说明
## MNN接入
1. git clone https://github.com/alibaba/MNN.git
2. git checkout 3.3.0
3. 设置环境变量ANDROID_NDK（为了与ffmpeg兼容需要指定ndk版本为16.1.4479499）
4. 拷贝"build_mnn.sh"到"mnn/project/android"目录，并且cd到该目录
5. rm -rf CMakeCache.txt CMakeFiles
6. ./build_mnn.sh（生成不同的架构需要修改脚本中对应的"arm64-v8a"和"armeabi-v7a"）
7. 拷贝"mnn/project/android/"下的libMNN.so,libMNN_Express.so,libMNNOpenCV.so到jniLibs下使用
8. 拷贝3.3.0对应的头文件

