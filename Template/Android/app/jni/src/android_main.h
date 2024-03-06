#include "Game.h"
#include "Common/SDLEventPool.h"
#include "EngineSettings.h"
#include "GlErrorReporter.h"
#include "SDL.h"
#include "Scene.h"
#include "ToolKit.h"
#include "Types.h"
#include "UIManager.h"
#include "GameRenderer.h"
#include "Plugin.h"

#include <stdio.h>
#include <chrono>
#include <iostream>

#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <sys/stat.h>
#include <unistd.h>

#define ANDROID_LOG(format, ...) __android_log_print(ANDROID_LOG_DEBUG, "TK_LOG", format, ##__VA_ARGS__)

#define TK_PLATFORM PLATFORM::TKAndroid

namespace ToolKit
{
  AAssetManager* assetManager = nullptr;

  // copies all of the engine assets to internal data folder if already not copied
  inline void CopyAllAssetsToDataPath(String& internalDataPath)
  {
    const char* MinResourcesPak = "MinResources.pak";
    AAsset* asset = AAssetManager_open(assetManager, MinResourcesPak, 0);

    if (!asset)
    {
      ANDROID_LOG("cannot open MinResources.pak!\n");
      return;
    }
    FILE* fileHandle = fopen(ConcatPaths({ internalDataPath, MinResourcesPak }).c_str(), "wb");
    mkdir(ConcatPaths({ internalDataPath, "Resources" }).c_str(), 0777);

    off_t size = AAsset_getLength(asset);
    std::vector<char> buffer;
    buffer.resize(size + 1, '\0');
    AAsset_read(asset, buffer.data(), size);
    fwrite(buffer.data(), 1, size + 1, fileHandle);
    memset(buffer.data(), 0, buffer.capacity());
    fclose(fileHandle);
    AAsset_close(asset);
  }

  inline void PlatformPreInit(Main* g_proxy)
  {
    String internalPath = SDL_AndroidGetInternalStoragePath();

    g_proxy->m_resourceRoot = ConcatPaths({ internalPath, "Resources" });
    g_proxy->m_cfgPath = ConcatPaths({ internalPath, "Config" });

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
}

extern "C"
{
  JNIEXPORT void JNICALL
    Java_com_otyazilim_toolkit_ToolKitAndroid_load(JNIEnv* env, jclass clazz, jobject mgr) {
    ToolKit::assetManager = AAssetManager_fromJava(env, mgr);
    if (ToolKit::assetManager == nullptr) {
      __android_log_print(ANDROID_LOG_ERROR, "ToolKit_Android", "error loading asset manager");
    }
    else {
      __android_log_print(ANDROID_LOG_VERBOSE, "ToolKit_Android", "Asset manager loaded successfully");
    }
  }
}
