# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

# Declares the project name. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project("facekit")

# Collect source files using glob
file(GLOB_RECURSE ANDROID_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/android/*.cpp")
file(GLOB_RECURSE COMMON_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp")
file(GLOB_RECURSE RENDER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/render/*.cpp")
file(GLOB_RECURSE CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp")
file(GLOB_RECURSE IO_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/io/*.cpp")

# Import prebuilt libraries from jniLibs
add_library(MNN SHARED IMPORTED)
set_target_properties(MNN PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libMNN.so)

add_library(MNNOpenCV SHARED IMPORTED)
set_target_properties(MNNOpenCV PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libMNNOpenCV.so)

add_library(${CMAKE_PROJECT_NAME} SHARED
        ${COMMON_SOURCES}
        ${RENDER_SOURCES}
        ${CORE_SOURCES}
        ${IO_SOURCES}
        android/egl/ImagePreviewer.cpp
        )
if(ANDROID)
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${ANDROID_SOURCES})
endif()

target_include_directories(${CMAKE_PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)
# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library
        android
        log
        MNN
        MNNOpenCV)