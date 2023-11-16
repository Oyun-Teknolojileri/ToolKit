/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

/**
 * @file ToolKit.h Header for Main, the manager class to access all the
 * functionalities of the ToolKit framework.
 */

#include "Object.h"
#include "Types.h"

/**
 * Base name space for all the ToolKit functionalities.
 */
namespace ToolKit
{

  /**
   * A class that Provides a unique handle when needed.
   */
  class TK_API HandleManager
  {
   public:
    HandleManager();

    ULongID GenerateHandle();
    void AddHandle(ULongID val);
    void ReleaseHandle(ULongID val);
    bool IsHandleUnique(ULongID val);

   private:
    uint64 m_randomXor[2];
    std::unordered_set<uint64> m_uniqueIDs;
  };

  /**
   * Structure that holds time related data.
   * Main has one of this which gives current application time.
   */
  struct TK_API Timing
  {
    void Init(uint fps);

    float LastTime    = 0.0f;
    float CurrentTime = 0.0f;
    float DeltaTime   = 0.0f;
    float TimeAccum   = 0.0f;
    int FrameCount    = 0;
  };

  /**
   * Main class that provides access to all sorts of manager and utility functionalities provided by the Engine.
   */
  class TK_API Main
  {
   public:
    Main();
    virtual ~Main();

    Main(const Main&)           = delete;
    void operator=(const Main&) = delete;

    virtual void PreInit();
    virtual void Init();
    virtual void Uninit();
    virtual void PostUninit();

    float TimeSinceStartup() { return m_timing.CurrentTime; }

    /**
     * Overrides the default configPath, if not changed relative to editor.exe.
     * ../Config
     * @param cfgPath is the new path for default config files. For windows it
     * could be %appdata%ToolKit.
     */
    void SetConfigPath(StringView cfgPath);

    /**
     * Returns config path if set. It won't return any default path in case if
     * nothing is set. To access the proper configpath see ConfigPath().
     */
    StringView GetConfigPath();

    static Main* GetInstance();

    static Main* GetInstance_noexcep() { return m_proxy; };

    static void SetProxy(Main* proxy);

   public:
    Timing m_timing;
    class AnimationManager* m_animationMan     = nullptr;
    class AnimationPlayer* m_animationPlayer   = nullptr;
    class AudioManager* m_audioMan             = nullptr;
    class MaterialManager* m_materialManager   = nullptr;
    class MeshManager* m_meshMan               = nullptr;
    class ShaderManager* m_shaderMan           = nullptr;
    class SpriteSheetManager* m_spriteSheetMan = nullptr;
    class TextureManager* m_textureMan         = nullptr;
    class SceneManager* m_sceneManager         = nullptr;
    class PluginManager* m_pluginManager       = nullptr;
    class Logger* m_logger                     = nullptr;
    class UIManager* m_uiManager               = nullptr;
    class SkeletonManager* m_skeletonManager   = nullptr;
    class FileManager* m_fileManager           = nullptr;
    class ObjectFactory* m_objectFactory       = nullptr;
    class RenderSystem* m_renderSys            = nullptr;
    class EngineSettings* m_engineSettings     = nullptr;
    HandleManager m_handleManager;

    bool m_preInitiated = false;
    bool m_initiated    = false;
    String m_resourceRoot;
    String m_cfgPath;
    EventPool m_eventPool;

   private:
    static Main* m_proxy;
  };

  // Accessors.
  TK_API class Logger* GetLogger();
  TK_API class RenderSystem* GetRenderSystem();
  TK_API class AnimationManager* GetAnimationManager();
  TK_API class AnimationPlayer* GetAnimationPlayer();
  TK_API class AudioManager* GetAudioManager();
  TK_API class MaterialManager* GetMaterialManager();
  TK_API class MeshManager* GetMeshManager();
  TK_API class ShaderManager* GetShaderManager();
  TK_API class SpriteSheetManager* GetSpriteSheetManager();
  TK_API class TextureManager* GetTextureManager();
  TK_API class SceneManager* GetSceneManager();
  TK_API class PluginManager* GetPluginManager();
  TK_API class UIManager* GetUIManager();
  TK_API class HandleManager* GetHandleManager();
  TK_API class SkeletonManager* GetSkeletonManager();
  TK_API class FileManager* GetFileManager();
  TK_API class EngineSettings& GetEngineSettings();
  TK_API class ObjectFactory* GetObjectFactory();

  // Path.
  TK_API String DefaultPath();
  TK_API String DefaultAbsolutePath();
  TK_API String ConfigPath();
  TK_API String ResourcePath(bool def = false);
  TK_API String TexturePath(const String& file, bool def = false);
  TK_API String MeshPath(const String& file, bool def = false);
  TK_API String FontPath(const String& file, bool def = false);
  TK_API String SpritePath(const String& file, bool def = false);
  TK_API String AudioPath(const String& file, bool def = false);
  TK_API String AnimationPath(const String& file, bool def = false);
  TK_API String SkeletonPath(const String& file, bool def = false);
  TK_API String ShaderPath(const String& file, bool def = false);
  TK_API String MaterialPath(const String& file, bool def = false);
  TK_API String ScenePath(const String& file, bool def = false);
  TK_API String PrefabPath(const String& file, bool def = false);
  TK_API String LayerPath(const String& file, bool def = false);

} // namespace ToolKit
