package com.liej6799.opendashcam.ui;

import androidx.appcompat.app.AppCompatActivity;
import androidx.camera.view.PreviewView;
import androidx.constraintlayout.widget.ConstraintLayout;

import android.graphics.PixelFormat;
import android.os.Bundle;
import android.view.SurfaceView;
import android.widget.ImageView;

import com.liej6799.opendashcam.R;
import com.liej6799.opendashcam.di.ViewModelProviderFactory;
import com.liej6799.opendashcam.di.sharedpref.CustomSharedPreferences;
import com.liej6799.opendashcam.ui.camera.CameraBackgroundActivity;
import com.liej6799.opendashcam.util.NativeCall;

import javax.inject.Inject;

import butterknife.BindView;
import butterknife.ButterKnife;

public class MainActivity extends CameraBackgroundActivity {

    @Inject
    CustomSharedPreferences customShaedPreferences;

    @BindView(R.id.cameraview)
    SurfaceView cameraview;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ButterKnife.bind(this);

        try {
            NativeCall.Init(getAssets(), false);

        } catch (Exception e) {
            e.printStackTrace();
        }


        surfaceViewMutableLiveData.setValue(cameraview);
        HandleCameraPermission();
    }
}