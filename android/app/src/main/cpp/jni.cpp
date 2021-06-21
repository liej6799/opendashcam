#include <opencv2/imgproc/types_c.h>
#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <android/log.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <chrono>
#include "jni.h"

#include <android/bitmap.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include "Common/Input/NdkCamera.h"
#include "Ncnn/Model/NanoDet.h"

#define ASSERT(status, ret)     if (!(status)) { return ret; }
#define ASSERT_FALSE(status)    ASSERT(status, false)


class MyNdkCamera : public NdkCameraWindow
{
public:
    virtual void on_image_render(cv::Mat& rgb) const;
};
void MyNdkCamera::on_image_render(cv::Mat& rgb) const
{
    if (NanoDet::detector)
    {
        auto result = NanoDet::detector->detect(rgb, 0.4, 0.5);

        NanoDet::detector->Draw(rgb, result.result, result.effect_area);
    }
}


static MyNdkCamera* g_camera = 0;


void BitmapToMatrix(JNIEnv * env, jobject obj_bitmap, cv::Mat & matrix) {
    void * bitmapPixels;                                            // Save picture pixel data
    AndroidBitmapInfo bitmapInfo;                                   // Save picture parameters

    AndroidBitmap_getInfo(env, obj_bitmap, &bitmapInfo);        // Get picture parameters
    AndroidBitmap_lockPixels(env, obj_bitmap, &bitmapPixels);  // Get picture pixels (lock memory block)

    if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC4, bitmapPixels);    // Establish temporary mat
        tmp.copyTo(matrix);
        cvtColor(matrix, matrix , cv::COLOR_RGBA2RGB);
    } else {
        cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC2, bitmapPixels);
        cv::cvtColor(tmp, matrix, cv::COLOR_BGR5652RGB);
    }
    AndroidBitmap_unlockPixels(env, obj_bitmap);            // Unlock
}


void MatrixToBitmap(JNIEnv * env, cv::Mat matrix, jobject obj_bitmap) {
    void * bitmapPixels;                                            // Save picture pixel data
    AndroidBitmapInfo bitmapInfo;                                   // Save picture parameters
    AndroidBitmap_getInfo(env, obj_bitmap, &bitmapInfo);
    AndroidBitmap_lockPixels(env, obj_bitmap, &bitmapPixels);

    if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC3, bitmapPixels);
        switch (matrix.type()) {
            case CV_8UC1:   cv::cvtColor(matrix, tmp, cv::COLOR_GRAY2RGBA);     break;
            case CV_8UC3:   cv::cvtColor(matrix, tmp, cv::COLOR_RGB2RGBA);      break;
            case CV_8UC4:   matrix.copyTo(tmp);                                 break;
            default:        AndroidBitmap_unlockPixels(env, obj_bitmap);
        }
    } else {
        cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC2, bitmapPixels);
        switch (matrix.type()) {
            case CV_8UC1:   cv::cvtColor(matrix, tmp, cv::COLOR_GRAY2BGR565);   break;
            case CV_8UC3:   cv::cvtColor(matrix, tmp, cv::COLOR_RGB2BGR565);    break;
            case CV_8UC4:   cv::cvtColor(matrix, tmp, cv::COLOR_RGBA2BGR565);   break;
            default:        AndroidBitmap_unlockPixels(env, obj_bitmap);
        }
    }
    AndroidBitmap_unlockPixels(env, obj_bitmap);                // Unlock
}



extern "C" {

JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *reserved) {
    delete NanoDet::detector;
    delete g_camera;
}

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    g_camera = new MyNdkCamera;
    return JNI_VERSION_1_4;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_liej6799_opendashcam_util_NativeCall_Init(JNIEnv *env, jclass clazz, jobject assetManager,
                                                   jboolean use_gpu) {
    AAssetManager *mgr = AAssetManager_fromJava(env, assetManager);

    if (NanoDet::detector != nullptr) {
        delete NanoDet::detector;
        NanoDet::detector = nullptr;
    }
    if (NanoDet::detector == nullptr) {
        NanoDet::detector = new NanoDet(mgr, "nanodet_m.param", "nanodet_m.bin", true);
    }
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_liej6799_opendashcam_util_NativeCall_openCamera(JNIEnv *env, jclass clazz) {

    g_camera->open(0);

    return JNI_TRUE;
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_liej6799_opendashcam_util_NativeCall_closeCamera(JNIEnv *env, jclass clazz) {
    g_camera->close();
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_liej6799_opendashcam_util_NativeCall_setOutputWindow(JNIEnv *env, jclass clazz,
                                                              jobject surface) {
    ANativeWindow *win = ANativeWindow_fromSurface(env, surface);
    g_camera->set_window(win);

    return JNI_TRUE;
}
}