# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.18.1)

# Declares and names the project.

project("ffmpeglearning")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

#config include file path
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/ffmpeg_lib)
include_directories(${CMAKE_SOURCE_DIR}/src/common/template_impl) #模板声明和实现文件分开
include_directories(${CMAKE_SOURCE_DIR}/include/common)

#config c++ source files path
file(GLOB src_files
        ${CMAKE_SOURCE_DIR}/src/dynamic_register/*.cpp
        ${CMAKE_SOURCE_DIR}/src/ffmpeg_learning/*.cpp
        ${CMAKE_SOURCE_DIR}/src/common/thread/*.cpp
        ${CMAKE_SOURCE_DIR}/src/ffmpeg_thread/*.cpp
        ${CMAKE_SOURCE_DIR}/src/ffmpeg_queue/*.cpp
        )

#config libs path
#this way need config "sourceSets {
#main {
#jniLibs.srcDirs = ['src/main/jni/libs']
#}
#}" in app build.gradle
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/libs/ffmpeg/${ANDROID_ABI}")

#message( [STATUS|WARNING|AUTHOR_WARNING|FATAL_ERROR|SEND_ERROR] "message to display" ...)
#(无) = 重要消息；
#STATUS = 非重要消息；
#WARNING = CMake 警告, 会继续执行；
#AUTHOR_WARNING = CMake 警告 (dev), 会继续执行；
#SEND_ERROR = CMake 错误, 继续执行，但是会跳过生成的步骤；
#FATAL_ERROR = CMake 错误, 终止所有处理过程；
message(CMAKE_SOURCE_DIR---->${CMAKE_SOURCE_DIR})
message(ANDROID_ABI---->${ANDROID_ABI})
message(src files---->${src_files})


add_library( # Sets the name of the library.
        ffmpeglearning

        # Sets the library as a shared library.
        SHARED
        # Provides a relative path to your source file(s).
        ${src_files}
        )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        liblog
        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        ffmpeglearning
        # Links the target library to the log library
        # included in the NDK.
        ${liblog}
        #ffmpeg libs
        avcodec
        avutil
        avformat
        )