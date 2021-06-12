#include <opencv2/imgproc/types_c.h>
#include <android/asset_manager_jni.h>
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

#include "Ncnn/Model/NanoDet.h"

#define ASSERT(status, ret)     if (!(status)) { return ret; }
#define ASSERT_FALSE(status)    ASSERT(status, false)

void BitmapToMatrix(JNIEnv * env, jobject obj_bitmap, cv::Mat & matrix) {
    void * bitmapPixels;                                            // Save picture pixel data
    AndroidBitmapInfo bitmapInfo;                                   // Save picture parameters

    AndroidBitmap_getInfo(env, obj_bitmap, &bitmapInfo);        // Get picture parameters
    AndroidBitmap_lockPixels(env, obj_bitmap, &bitmapPixels);  // Get picture pixels (lock memory block)

    if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC4, bitmapPixels);    // Establish temporary mat
        tmp.copyTo(matrix);
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
        cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC4, bitmapPixels);
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
}extern "C"
JNIEXPORT jobject JNICALL
Java_com_liej6799_opendashcam_util_NativeCall_Run(JNIEnv *env, jclass clazz, jobject bitmap,
                                                  jdouble threshold, jdouble nms_threshold) {
    cv::Mat matBitmap;
    BitmapToMatrix(env, bitmap, matBitmap);

    auto result = NanoDet::detector->detect(matBitmap, 0.4, 0.5);

    NanoDet::detector->Draw(matBitmap, result.result, result.effect_area);
    MatrixToBitmap(env, matBitmap, bitmap);
    return bitmap;
}
