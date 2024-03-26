/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

/**
 * @file ToolKit.h Header for Main, the manager class to access all the
 * functionalities of the ToolKit framework.
 */

#include "Object.h"
#include "TKPlatform.h"
#include "Types.h"

/**
 * Base name space for all the ToolKit functionalities.
 */
namespace ToolKit
{
  /**
   * Callback for registering to frame updates.
   * Use Main::RegisterPreUpdateFunction and Main::RegisterPostUpdateFunction for receiving updates.
   */
  typedef std::function<void(float deltaTime)> TKUpdateFn;

  /**
   * A class that Provides a unique handle when needed.
   */
  class TK_API HandleManager
  {
   public:
    HandleManager(); //!< Default constructor, initializes the handle manager with a random seed.

    /**
     * Random id that guarantees uniqueness on runtime. Collisions are resolved during deserialize, if any.
     * These ids, freed when using of it completed. So ids are reused and do not overflow.
     */
    ULongID GenerateHandle();

    void AddHandle(ULongID val);     //!< Add record for the random id. Prevent it from getting acquired multiple times.
    void ReleaseHandle(ULongID val); //!< Free the id for reuse.
    bool IsHandleUnique(ULongID val); //!< Test if id acquired.

   private:
    ULongID m_randomXor[2];                  //!< Random seed.
    std::unordered_set<ULongID> m_uniqueIDs; //!< Container for all acquired handles.
  };

  /**
   * Structure that holds time related data.
   * Main has one of this which gives current application time.
   */
  struct TK_API Timing
  {
    void Init(uint fps);
    float GetDeltaTime();

    float CurrentTime     = 0.0f; //!< Total elapsed time in milliseconds. Updated after every frame.
    float TargetDeltaTime = 0.0f; //!< Target delta time in milliseconds calculated as (1000 / Target Fps)
    int FramesPerSecond   = 0;    //!< Number of frames drawn within 1 second.
    int FrameCount        = 0;    //!< Internally used to count number of frames per second.
    float LastTime        = 0.0f; //!< Internally used to determine if enough time has passed for a new frame.
    float TimeAccum       = 0.0f; //!< Internally used to determine if enough time has passed for a new frame.
  };

  /**
   * Main class that provides access to all sorts of manager and utility functionalities provided by the Engine.
   */
  class TK_API Main
  {
   public:
    /**
     * Default constructor. Does not initialize the Main.
     */
    Main();

    /**
     * Default destructor. Does not uninitialie the main.
     */
    virtual ~Main();

    Main(const Main&)           = delete;
    void operator=(const Main&) = delete;

    virtual void PreInit();    //!< Creates all the managers and systems for the engine.
    virtual void Init();       //!< Initialize all the managers and systems. Engine fully functions at this point.
    virtual void Uninit();     //!< Uninitialize all the managers and systems.
    virtual void PostUninit(); //!< Destroy all the engine allocated resources. Nothing is accessible from this on.

    /**
     * Overrides the default configPath, if not changed, its relative to editor.exe. Default: ../Config
     * @param cfgPath is the new path for default config files. For windows it could be %appdata%/ToolKit.
     */
    void SetConfigPath(StringView cfgPath);

    /**
     * Returns config path if set. It won't return any default path in case if
     * nothing is set. To access the proper config path see ConfigPath().
     */
    StringView GetConfigPath();

    static Main* GetInstance();         //!< Access function for the instance of Main.
    static Main* GetInstance_noexcep(); //!< Access function for the instance of Main. Does not perform debug checks.
    static void SetProxy(Main* proxy);  //!< Sets the instance of Main that will be used afterwards.

    /**
     * This function should be called at the beginning of frame.
     */
    void FrameBegin();

    /**
     * This function updates data that ToolKit handles.
     * This function also calls registered PreUpdate and PostUpdate functions.
     */
    void FrameUpdate();

    /**
     * This function should be called at the end of frame.
     */
    void FrameEnd();

    /**
     * This function registers function that should be called before ToolKit update every frame.
     */
    void RegisterPreUpdateFunction(TKUpdateFn preUpdateFn);

    /**
     * This function registers function that should be called after ToolKit update every frame.
     */
    void RegisterPostUpdateFunction(TKUpdateFn postUpdateFn);

    /**
     * This function clears registered pre-update functions.
     */
    void ClearPreUpdateFunctions();

    /**
     * This function clears registered post-update functions.
     */
    void ClearPostUpdateFunctions();

    /**
     * @return Current frame count per second.
     */
    int GetCurrentFPS() const;

    /**
     * @return Total elapsed time in millisecond since the initialization.
     */
    float TimeSinceStartup() const;

    /**
     * @return true if enough time have passed from previous frame
     */
    bool SyncFrameTime();

   private:
    void Frame(float deltaTime); //!< Performs an update for all engine components.

   public:
    Timing m_timing; //!< Timer that keeps time related data since the Initialization.
    class AnimationManager* m_animationMan       = nullptr;
    class AnimationPlayer* m_animationPlayer     = nullptr;
    class AudioManager* m_audioMan               = nullptr;
    class MaterialManager* m_materialManager     = nullptr;
    class MeshManager* m_meshMan                 = nullptr;
    class ShaderManager* m_shaderMan             = nullptr;
    class SpriteSheetManager* m_spriteSheetMan   = nullptr;
    class TextureManager* m_textureMan           = nullptr;
    class SceneManager* m_sceneManager           = nullptr;
    class PluginManager* m_pluginManager         = nullptr;
    class Logger* m_logger                       = nullptr;
    class UIManager* m_uiManager                 = nullptr;
    class SkeletonManager* m_skeletonManager     = nullptr;
    class FileManager* m_fileManager             = nullptr;
    class ObjectFactory* m_objectFactory         = nullptr;
    class RenderSystem* m_renderSys              = nullptr;
    class EngineSettings* m_engineSettings       = nullptr;
    class TKStats* m_tkStats                     = nullptr;
    class WorkerManager* m_workerManager         = nullptr;
    class GpuProgramManager* m_gpuProgramManager = nullptr;
    HandleManager m_handleManager;

    bool m_preInitiated = false;
    bool m_initiated    = false;
    bool m_threaded     = true;
    String m_resourceRoot;
    String m_cfgPath;
    EventPool m_eventPool;

   private:
    static Main* m_proxy;

    std::vector<TKUpdateFn> m_preUpdateFunctions;
    std::vector<TKUpdateFn> m_postUpdateFunctions;
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
  TK_API class TKStats* GetTKStats();
  TK_API class WorkerManager* GetWorkerManager();
  TK_API class GpuProgramManager* GetGpuProgramManager();

  // Path.
  TK_API String DefaultPath();
  TK_API String DefaultAbsolutePath();
  TK_API String ConfigPath();
  TK_API String EngineSettingsPath();
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
