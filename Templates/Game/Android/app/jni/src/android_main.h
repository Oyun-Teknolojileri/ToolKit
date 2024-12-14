/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "EngineSettings.h"
#include "Game.h"
#include "Logger.h"
#include "SDL.h"
#include "Util.h"

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include <chrono>
#include <iostream>

#define ANDROID_LOG(format, ...) __android_log_print(ANDROID_LOG_DEBUG, "TK_LOG", format, ##__VA_ARGS__)

#define PLATFORM_SDL_FLAGS       (SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN)

#define TK_PLATFORM              PLATFORM::TKAndroid

namespace ToolKit
{
  AAssetManager* assetManager = nullptr;

  inline void CopyFileToDataPath(String& internalDataPath, const char* file)
  {
    AAsset* asset = AAssetManager_open(assetManager, file, 0);

    if (!asset)
    {
      ANDROID_LOG("cannot open MinResources.pak!\n");
      return;
    }
    FILE* fileHandle = fopen(ConcatPaths({internalDataPath, file}).c_str(), "wb");
    off_t size       = AAsset_getLength(asset);
    std::vector<char> buffer;
    buffer.resize(size + 1, '\0');
    AAsset_read(asset, buffer.data(), size);
    fwrite(buffer.data(), 1, size + 1, fileHandle);
    memset(buffer.data(), 0, buffer.capacity());
    fclose(fileHandle);
    AAsset_close(asset);
  }

  // copies all of the engine assets to internal data folder if already not copied
  inline void CopyAllAssetsToDataPath(String& internalDataPath)
  {
    mkdir(ConcatPaths({internalDataPath, "Resources"}).c_str(), 0777);
    mkdir(ConcatPaths({internalDataPath, "Config"}).c_str(), 0777);

    CopyFileToDataPath(internalDataPath, "MinResources.pak");
    CopyFileToDataPath(internalDataPath, ConcatPaths({"Config", "Engine.settings"}).c_str());
  }

  inline void PlatformPreInit(Main* g_proxy)
  {
    String internalPath     = SDL_AndroidGetInternalStoragePath();

    g_proxy->m_resourceRoot = ConcatPaths({internalPath, "Resources"});
    g_proxy->m_cfgPath      = ConcatPaths({internalPath, "Config"});

    // Set log function
    GetLogger()->SetWriteConsoleFn([](LogType lt, String ms) -> void { ANDROID_LOG("%s", ms.c_str()); });

    CopyAllAssetsToDataPath(internalPath);
  }

  inline void PlatformMainLoop(Game* g_game, bool g_running, void (*TK_Loop)())
  {
    while (g_game->m_currentState != PluginState::Stop && g_running)
    {
      TK_Loop();
    }
  }

  inline void PlatformAdjustEngineSettings(int availableWidth, int availableHeight, EngineSettings* engineSettings)
  {
    engineSettings->Window.Width  = (uint) availableWidth;
    engineSettings->Window.Height = (uint) availableHeight;
  }
} // namespace ToolKit

extern "C"
{
  JNIEXPORT void JNICALL Java_com_otyazilim_toolkit_ToolKitAndroid_load(JNIEnv* env, jclass clazz, jobject mgr)
  {
    ToolKit::assetManager = AAssetManager_fromJava(env, mgr);
    if (ToolKit::assetManager == nullptr)
    {
      __android_log_print(ANDROID_LOG_ERROR, "ToolKit_Android", "error loading asset manager");
    }
    else
    {
      __android_log_print(ANDROID_LOG_VERBOSE, "ToolKit_Android", "Asset manager loaded successfully");
    }
  }
}
