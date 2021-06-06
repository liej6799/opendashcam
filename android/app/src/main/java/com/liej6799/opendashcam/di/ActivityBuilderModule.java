package com.liej6799.opendashcam.di;

import com.liej6799.opendashcam.ui.MainActivity;

import dagger.Module;
import dagger.android.ContributesAndroidInjector;

@Module
public abstract class ActivityBuilderModule {
    @ContributesAndroidInjector
    abstract MainActivity contributeMainActivity();
}
