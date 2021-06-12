package com.liej6799.opendashcam.ui;

import androidx.appcompat.app.AppCompatActivity;
import androidx.camera.view.PreviewView;
import androidx.constraintlayout.widget.ConstraintLayout;

import android.os.Bundle;
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




    @BindView(R.id.preview_view)
    PreviewView mPreviewView;

    @BindView(R.id.image_view)
    ImageView mImageView;

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


        previewViewMutableLiveData.setValue(mPreviewView);
        imageViewMutableLiveData.setValue(mImageView);
        HandleCameraPermission();
    }
}