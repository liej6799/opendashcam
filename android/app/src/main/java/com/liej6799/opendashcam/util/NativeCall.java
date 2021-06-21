package com.liej6799.opendashcam.util;

import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.view.Surface;

public class NativeCall {
    static {
        System.loadLibrary("opendashcam_native");
    }
    public static native void Init(AssetManager mgr, boolean use_gpu);
    public static native boolean openCamera();
    public static native boolean closeCamera();
    public static native boolean setOutputWindow(Surface surface);
}
