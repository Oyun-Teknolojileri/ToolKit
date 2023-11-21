package com.otyazilim.toolkit;

import android.content.res.AssetManager;

public class ToolKitAndroid
{
    // this function is implemented in cpp
    private static native void load(AssetManager mgr);

    public void LoadAssetManagerToCpp(AssetManager mgr)
    {
        load(mgr);
    }
}
