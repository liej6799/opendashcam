package com.liej6799.opendashcam.ui.camera;

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.util.Size;
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

public class CameraBackgroundActivity extends LandscapeDaggerAppCompatActivity {
    public static double threshold = 0.3, nms_threshold = 0.7;
    public MutableLiveData<PreviewView> previewViewMutableLiveData = new MutableLiveData<>();
    public MutableLiveData<ImageView> imageViewMutableLiveData = new MutableLiveData<>();


    private static final int REQUEST_CODE_PERMISSIONS = 10;
    private static final String[] REQUIRED_PERMISSIONS = { Manifest.permission.CAMERA };

    private ListenableFuture<ProcessCameraProvider> mCameraProviderFuture;
    private ExecutorService executor = Executors.newSingleThreadExecutor();
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

    }

    public void startCamera() {
        if (imageViewMutableLiveData.getValue() != null && previewViewMutableLiveData.getValue() != null)
        {
            imageViewMutableLiveData.getValue().post(() -> {
                mCameraProviderFuture = ProcessCameraProvider.getInstance(this);
                mCameraProviderFuture.addListener(() -> {
                    try {
                        ProcessCameraProvider cameraProvider = mCameraProviderFuture.get();
                        bindPreview(cameraProvider);
                    } catch (ExecutionException | InterruptedException e) {
                        // No errors need to be handled for this Future.
                        // This should never be reached.
                    }
                }, ContextCompat.getMainExecutor(this));
            });
        }

    }

    public void stopCamera() {
        mCameraProviderFuture = ProcessCameraProvider.getInstance(this);
        try {
            mCameraProviderFuture.get().unbindAll();
        } catch (ExecutionException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private void bindPreview(@NonNull ProcessCameraProvider cameraProvider) {
        Preview preview = new Preview.Builder().build();
        CameraSelector cameraSelector = new CameraSelector.Builder()
                .requireLensFacing(CameraSelector.LENS_FACING_BACK)
                .build();
//
        preview.setSurfaceProvider(previewViewMutableLiveData.getValue().getSurfaceProvider());
        //surfaceViewMutableLiveData
        // Image Analysis
        @SuppressLint("RestrictedApi") ImageAnalysis imageAnalysis =
                new ImageAnalysis.Builder()
                        .setTargetAspectRatio(AspectRatio.RATIO_16_9)
                        .setDefaultResolution(new Size(1280, 720))
                        .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
                        .build();
        imageAnalysis.setAnalyzer(executor, image -> {
            analyzeImage(image, image.getImageInfo().getRotationDegrees());

        });

        cameraProvider.bindToLifecycle(this, cameraSelector, imageAnalysis);
    }

    @SuppressLint("UnsafeExperimentalUsageError")
    private  void analyzeImage(ImageProxy image, int rotationDegrees) {
        // Convert Image to ByteArray
        @SuppressLint("UnsafeOptInUsageError")
        Bitmap bitmap = ImageHelper.ConvertImageToBitmap(Objects.requireNonNull(image.getImage()));
        runOnUiThread(() -> imageViewMutableLiveData.getValue().setImageBitmap(bitmap));

        image.close();
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

}