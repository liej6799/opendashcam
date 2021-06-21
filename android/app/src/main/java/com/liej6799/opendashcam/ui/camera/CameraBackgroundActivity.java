package com.liej6799.opendashcam.ui.camera;

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.PixelFormat;
import android.os.Bundle;
import android.util.Size;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.widget.ImageView;

import androidx.annotation.NonNull;
import androidx.camera.core.AspectRatio;
import androidx.camera.core.CameraSelector;
import androidx.camera.core.ImageAnalysis;
import androidx.camera.core.ImageProxy;
import androidx.camera.core.Preview;
import androidx.camera.lifecycle.ProcessCameraProvider;
import androidx.camera.view.PreviewView;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModelProvider;

import com.google.common.util.concurrent.ListenableFuture;
import com.liej6799.opendashcam.ui.LandscapeDaggerAppCompatActivity;
import com.liej6799.opendashcam.util.ImageHelper;
import com.liej6799.opendashcam.util.NativeCall;

import java.util.Objects;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import javax.inject.Inject;

public class CameraBackgroundActivity extends LandscapeDaggerAppCompatActivity implements SurfaceHolder.Callback {
    public static double threshold = 0.3, nms_threshold = 0.7;
    public MutableLiveData<SurfaceView> surfaceViewMutableLiveData = new MutableLiveData<>();


    private static final int REQUEST_CODE_PERMISSIONS = 10;
    private static final String[] REQUIRED_PERMISSIONS = { Manifest.permission.CAMERA };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    public void startCamera() {
        if (surfaceViewMutableLiveData.getValue() != null)
        {
            surfaceViewMutableLiveData.getValue().post(() -> {
                surfaceViewMutableLiveData.getValue().getHolder().setFormat(PixelFormat.RGBA_8888);
                surfaceViewMutableLiveData.getValue().getHolder().addCallback(this);

//               NativeCall.closeCamera();
                NativeCall.openCamera();
            });
        }

    }

    public void stopCamera() {
        NativeCall.closeCamera();
    }


    // Permission
    public void HandleCameraPermission()
    {
        if (allPermissionsGranted())
        {
            startCamera();
        }
        else{
            ActivityCompat.requestPermissions(
                    this, REQUIRED_PERMISSIONS, REQUEST_CODE_PERMISSIONS);
        }
    }
    private boolean allPermissionsGranted() {
        for (String permission : REQUIRED_PERMISSIONS) {
            if (ContextCompat.checkSelfPermission(getBaseContext(), permission) != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        NativeCall.setOutputWindow(holder.getSurface());
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {

    }
}