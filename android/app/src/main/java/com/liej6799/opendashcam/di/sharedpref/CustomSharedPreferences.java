package com.liej6799.opendashcam.di.sharedpref;

import android.content.SharedPreferences;

import javax.inject.Inject;

public class CustomSharedPreferences {
    private SharedPreferences mSharedPreferences;

    @Inject
    public CustomSharedPreferences(SharedPreferences mSharedPreferences) {
        this.mSharedPreferences = mSharedPreferences;
    }

    public void putData(String key, int data) {
        mSharedPreferences.edit().putInt(key,data).apply();
    }

    public int getData(String key) {
        return mSharedPreferences.getInt(key,0);
    }

    public void putAppAuth(String data) {
        mSharedPreferences.edit().putString("AUTH_STATE",data).apply();
    }

    public String getAppAuth() {
        return mSharedPreferences.getString("AUTH_STATE", "");
    }

    public void clearAppAuth() {
        mSharedPreferences.edit().clear().apply();
    }
}