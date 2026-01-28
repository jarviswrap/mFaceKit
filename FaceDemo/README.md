
## MNN接入
1. git clone https://github.com/alibaba/MNN.git
2. git checkout 3.3.0
3. 设置环境变量ANDROID_NDK（为了与ffmpeg兼容需要指定ndk版本为16.1.4479499）
4. 拷贝"build_mnn.sh"到"mnn/project/android"目录，并且cd到该目录
5. rm -rf CMakeCache.txt CMakeFiles
6. ./build_mnn.sh（生成不同的架构需要修改脚本中对应的"arm64-v8a"和"armeabi-v7a"）
7. 拷贝"mnn/project/android/"下的libMNN.so,libMNN_Express.so,libMNNOpenCV.so到jniLibs下使用
8. 拷贝3.3.0对应的头文件

## addr2line堆栈符号化
- 64位：$ANDROID_NDK/toolchains/aarch64-linux-android-4.9/prebuilt/darwin-x86_64/bin/aarch64-linux-android-addr2line
- 32位：$ANDROID_NDK/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-addr2line

## ubuntu24.04上编译ndk16问题
ndk依赖的部分库找不到导致编译失败，解决方案是在系统创建链接到新版本的库
- error while loading shared libraries: libncurses.so.5: cannot open shared object file: No such file or directory
  - 解决方案：sudo ln -s /lib/x86_64-linux-gnu/libncurses.so.6 /lib/x86_64-linux-gnu/libncurses.so.5
- error while loading shared libraries: libtinfo.so.5: cannot open shared object file: No such file or directory
  - 解决方案：sudo ln -s /lib/x86_64-linux-gnu/libtinfo.so.6 /lib/x86_64-linux-gnu/libtinfo.so.5
