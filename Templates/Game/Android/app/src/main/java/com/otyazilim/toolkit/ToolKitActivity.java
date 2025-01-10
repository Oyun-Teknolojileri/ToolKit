package com.otyazilim.toolkit;

import android.content.res.AssetManager;
import android.os.Bundle;
import org.libsdl.app.SDLActivity;

public class ToolKitActivity extends SDLActivity {
    private static native void load(AssetManager mgr); // Implemented in android_main.h

    public void loadAssetManagerToCpp(AssetManager mgr) {
        load(mgr);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        loadAssetManagerToCpp(getResources().getAssets());
    }
}
