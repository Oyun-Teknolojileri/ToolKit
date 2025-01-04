package com.otyazilim.toolkit;

import android.content.res.AssetManager;
import android.os.Bundle;

import org.libsdl.app.SDLActivity;

public class ToolKitActivity extends SDLActivity
{
    // this function is implemented in cpp
    private static native void load(AssetManager mgr);

    public void loadAssetManagerToCpp(AssetManager mgr) {
        load(mgr);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        loadAssetManagerToCpp(getResources().getAssets());
    }
}
