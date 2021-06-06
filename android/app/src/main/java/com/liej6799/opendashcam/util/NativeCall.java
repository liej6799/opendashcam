package com.liej6799.opendashcam.util;

import android.content.res.AssetManager;
import android.graphics.Bitmap;

public class NativeCall {
    static {
        System.loadLibrary("opendashcam_native");
    }
    public static native void Init(AssetManager mgr, boolean use_gpu);
    public static native Bitmap Run(Bitmap bitmap, double threshold, double nms_threshold);
}
