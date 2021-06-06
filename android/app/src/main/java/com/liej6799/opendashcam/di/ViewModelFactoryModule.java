package com.liej6799.opendashcam.di;

import androidx.lifecycle.ViewModelProvider;

import dagger.Binds;
import dagger.Module;
import dagger.Provides;

@Module
public abstract class ViewModelFactoryModule {

    @Binds
    public abstract ViewModelProvider.Factory bindViewModelFactory(ViewModelProviderFactory viewModelProviderFactory);

    @Provides
    static ViewModelProvider.Factory bindFactory(ViewModelProviderFactory factory) {
        return factory;
    }


}
