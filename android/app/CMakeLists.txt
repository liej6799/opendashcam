cmake_minimum_required(VERSION 3.4.1)
set(TARGET opendashcam_native)
project(${TARGET} CXX)
set(CMAKE_CXX_STANDARD 14)

# Load NCNN
set(ncnn_DIR ${CMAKE_SOURCE_DIR}/src/main/cpp/ncnn-20210525-android/${ANDROID_ABI}/lib/cmake/ncnn)
find_package(ncnn REQUIRED)

# Load OpenCV
set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/src/main/cpp/opencv-mobile-4.5.1-android/sdk/native/jni)
find_package(OpenCV REQUIRED)

# Native Code
file(GLOB NATIVE_SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/src/main/cpp/*.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/main/cpp/Ncnn/Model/*.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/main/cpp/Common/Input/*.cpp")
file(GLOB NATIVE_INCLUDE_DIRS
        "${CMAKE_CURRENT_LIST_DIR}/src/main/cpp/Common/"
        "${CMAKE_CURRENT_LIST_DIR}/src/main/cpp/Ncnn/Common/"
        "${CMAKE_CURRENT_LIST_DIR}/src/main/cpp/Ncnn/Model/"
        "${CMAKE_CURRENT_LIST_DIR}/src/main/cpp/Common/Input")

add_library(${TARGET} SHARED
        ${NATIVE_SOURCES}
        )

target_compile_options(${TARGET} PRIVATE
        -fexceptions
        )


# Final - Target Include Library
target_include_directories(${TARGET} PRIVATE
        ${OPENCV_INCLUDE_DIRS}
        ${NATIVE_INCLUDE_DIRS})


target_link_libraries(${TARGET}
        ncnn
        ${OpenCV_LIBS}
        ${FBJNI_LIBRARY}
        log
        camera2ndk
        mediandk)