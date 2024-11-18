/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "ToolKit.h"

#include "Audio.h"
#include "EngineSettings.h"
#include "FileManager.h"
#include "GpuProgram.h"
#include "Logger.h"
#include "Material.h"
#include "Mesh.h"
#include "Object.h"
#include "ObjectFactory.h"
#include "PluginManager.h"
#include "RHI.h"
#include "RenderSystem.h"
#include "Scene.h"
#include "Shader.h"
#include "TKOpenGL.h"
#include "TKStats.h"
#include "Threads.h"
#include "UIManager.h"

namespace ToolKit
{
  HandleManager::HandleManager()
  {
    ULongID seed = time(nullptr) + ((ULongID) (this) ^ m_randomXor[0]);
    Xoroshiro128PlusSeed(m_randomXor, seed);
  }

  ULongID HandleManager::GenerateHandle()
  {
    ULongID id = Xoroshiro128Plus(m_randomXor);
    // If collision happens, change generate new id
    while (m_uniqueIDs.find(id) != m_uniqueIDs.end() || id == NULL_HANDLE)
    {
      id = Xoroshiro128Plus(m_randomXor);
    }
    m_uniqueIDs.insert(id);

    return id; // non zero
  }

  void HandleManager::AddHandle(ULongID val) { m_uniqueIDs.insert(val); }

  void HandleManager::ReleaseHandle(ULongID val)
  {

    auto it = m_uniqueIDs.find(val);
    if (it != m_uniqueIDs.end())
    {
      m_uniqueIDs.erase(it);
    }
  }

  bool HandleManager::IsHandleUnique(ULongID val) { return m_uniqueIDs.find(val) == m_uniqueIDs.end(); }

  Main* Main::m_proxy = nullptr;

  Main::Main()
  {
    // Start Timer.
    GetElapsedMilliSeconds();

    m_logger = new Logger();
    m_logger->Log("Main Constructed");

    m_tkStats = new TKStats();
    m_tkStats->ResetVRAMUsage();
  }

  Main::~Main()
  {
    ClearPreUpdateFunctions();
    ClearPostUpdateFunctions();

    assert(m_initiated == false && "Uninitiate before destruct");
    m_proxy = nullptr;

    SafeDel(m_tkStats);

    m_logger->Log("Main Destructed");
    SafeDel(m_logger);
  }

  void Main::PreInit()
  {
    assert(m_preInitiated == false && "Main already preInitialized");

    if (m_preInitiated)
    {
      return;
    }

    m_logger->Log("Main PreInit");

    m_workerManager  = new WorkerManager();
    m_engineSettings = new EngineSettings();
    m_objectFactory  = new ObjectFactory();
    m_objectFactory->Init();

    m_renderSys         = new RenderSystem();
    m_gpuProgramManager = new GpuProgramManager();
    m_pluginManager     = new PluginManager();
    m_animationMan      = new AnimationManager();
    m_animationPlayer   = new AnimationPlayer();
    m_textureMan        = new TextureManager();
    m_meshMan           = new MeshManager();
    m_spriteSheetMan    = new SpriteSheetManager();
    m_audioMan          = new AudioManager();
    m_shaderMan         = new ShaderManager();
    m_materialManager   = new MaterialManager();
    m_sceneManager      = new SceneManager();
    m_uiManager         = new UIManager();
    m_skeletonManager   = new SkeletonManager();
    m_fileManager       = new FileManager();

    m_preInitiated      = true;
  }

  void Main::Init()
  {
    assert(m_preInitiated && "Preinitialize first");
    assert(m_initiated == false && "Main already initialized");

    if (m_initiated)
    {
      return;
    }

    m_logger->Log("Main Init");

    RHI::m_initialized = true;

    m_workerManager->Init();
    m_animationMan->Init();
    m_textureMan->Init();
    m_meshMan->Init();
    m_spriteSheetMan->Init();
    m_audioMan->Init();
    m_shaderMan->Init();
    m_materialManager->Init();
    m_sceneManager->Init();
    m_skeletonManager->Init();
    m_renderSys->Init();
    m_timing.Init(m_engineSettings->Graphics.FPS);

    m_initiated = true;
  }

  void Main::PostInit() { m_pluginManager->Init(); }

  void Main::PreUninit() { m_pluginManager->UnInit(); }

  void Main::Uninit()
  {
    m_logger->Log("Main Uninit");

    RHI::m_initialized = false;
    m_animationPlayer->Destroy();
    m_animationMan->Uninit();
    m_textureMan->Uninit();
    m_meshMan->Uninit();
    m_spriteSheetMan->Uninit();
    m_audioMan->Uninit();
    m_shaderMan->Uninit();
    m_materialManager->Uninit();
    m_sceneManager->Uninit();
    m_skeletonManager->Uninit();

    m_initiated    = false;
    m_preInitiated = false;
  }

  void Main::PostUninit()
  {
    m_logger->Log("Main PostUninit");

    // After all the resources, we can safely free modules.
    m_pluginManager->UnInit();

    SafeDel(m_gpuProgramManager);
    SafeDel(m_renderSys);
    SafeDel(m_pluginManager);
    SafeDel(m_animationMan);
    SafeDel(m_animationPlayer);
    SafeDel(m_textureMan);
    SafeDel(m_meshMan);
    SafeDel(m_spriteSheetMan);
    SafeDel(m_audioMan);
    SafeDel(m_shaderMan);
    SafeDel(m_materialManager);
    SafeDel(m_sceneManager);
    SafeDel(m_uiManager);
    SafeDel(m_skeletonManager);
    SafeDel(m_fileManager);
    SafeDel(m_objectFactory);
    SafeDel(m_engineSettings);
    SafeDel(m_workerManager);
  }

  void Main::SetConfigPath(StringView cfgPath) { m_cfgPath = cfgPath; }

  StringView Main::GetConfigPath() { return m_cfgPath; }

  Main* Main::GetInstance()
  {
    assert(m_proxy && "ToolKit is not initialized.");
    return m_proxy;
  }

  Main* Main::GetInstance_noexcep() { return m_proxy; }

  void Main::SetProxy(Main* proxy)
  {
    bool singular = m_proxy == nullptr || m_proxy == proxy;
    assert(singular && "You can only have one instance of the main");
    if (singular)
    {
      m_proxy = proxy;
    }
  }

  bool Main::SyncFrameTime()
  {
    m_timing.CurrentTime = GetElapsedMilliSeconds();
    return m_timing.CurrentTime > m_timing.LastTime + m_timing.TargetDeltaTime;
  }

  void Main::FrameBegin()
  {
    Stats::ResetDrawCallCounter();
    Stats::ResetHWRenderPassCounter();
    Stats::ResetLightCacheInvalidationPerFrame();
  }

  void Main::FrameUpdate()
  {
    float deltaTime = m_timing.CurrentTime - m_timing.LastTime;

    // Call pre update callbacks
    for (const TKUpdateFn& updateFn : m_preUpdateFunctions)
    {
      updateFn(deltaTime);
    }

    Frame(deltaTime);

    // Call post update callbacks
    for (const TKUpdateFn& updateFn : m_postUpdateFunctions)
    {
      updateFn(deltaTime);
    }
  }

  void Main::FrameEnd()
  {
    m_timing.FrameCount++;
    m_timing.TimeAccum += m_timing.GetDeltaTime();
    if (m_timing.TimeAccum >= 1000.0f)
    {
      m_timing.TimeAccum       = 0;
      m_timing.FramesPerSecond = m_timing.FrameCount;
      m_timing.FrameCount      = 0;
    }

    m_timing.LastTime = m_timing.CurrentTime;
    GetRenderSystem()->EndFrame();

    // Display stat times.
    for (auto& timeStat : TKStatTimerMap)
    {
      TKStats::TimeArgs& args = timeStat.second;
      if (args.enabled)
      {
        TK_LOG("%s avg t: %f -- t: %f", timeStat.first.data(), args.accumulatedTime / args.hitCount, args.elapsedTime);
      }
    }
  }

  void Main::Frame(float deltaTime)
  {
    // Update animations.
    GetAnimationPlayer()->Update(MillisecToSec(deltaTime));
    GetUIManager()->Update(deltaTime);

    if (ScenePtr scene = GetSceneManager()->GetCurrentScene())
    {
      scene->Update(deltaTime);
    }

    GetPluginManager()->Update(deltaTime);

    GetRenderSystem()->DecrementSkipFrame();
    GetRenderSystem()->ExecuteRenderTasks();
  }

  void Main::RegisterPreUpdateFunction(TKUpdateFn preUpdateFn) { m_preUpdateFunctions.push_back(preUpdateFn); }

  void Main::RegisterPostUpdateFunction(TKUpdateFn postUpdateFn) { m_postUpdateFunctions.push_back(postUpdateFn); }

  void Main::ClearPreUpdateFunctions() { m_preUpdateFunctions.clear(); }

  void Main::ClearPostUpdateFunctions() { m_postUpdateFunctions.clear(); }

  int Main::GetCurrentFPS() const { return m_timing.FramesPerSecond; }

  float Main::TimeSinceStartup() const { return m_timing.CurrentTime; }

  Logger* GetLogger() { return Main::GetInstance()->m_logger; }

  RenderSystem* GetRenderSystem() { return Main::GetInstance()->m_renderSys; }

  AnimationManager* GetAnimationManager() { return Main::GetInstance()->m_animationMan; }

  AnimationPlayer* GetAnimationPlayer() { return Main::GetInstance()->m_animationPlayer; }

  AudioManager* GetAudioManager() { return Main::GetInstance()->m_audioMan; }

  MaterialManager* GetMaterialManager() { return Main::GetInstance()->m_materialManager; }

  MeshManager* GetMeshManager() { return Main::GetInstance()->m_meshMan; }

  ShaderManager* GetShaderManager() { return Main::GetInstance()->m_shaderMan; }

  SpriteSheetManager* GetSpriteSheetManager() { return Main::GetInstance()->m_spriteSheetMan; }

  TextureManager* GetTextureManager() { return Main::GetInstance()->m_textureMan; }

  SceneManager* GetSceneManager() { return Main::GetInstance()->m_sceneManager; }

  PluginManager* GetPluginManager() { return Main::GetInstance()->m_pluginManager; }

  ResourceManager* GetResourceManager(ClassMeta* Class)
  {
    if (Class->IsSublcassOf(Animation::StaticClass()))
    {
      return GetAnimationManager();
    }
    else if (Class->IsSublcassOf(Audio::StaticClass()))
    {
      return GetAudioManager();
    }
    else if (Class->IsSublcassOf(Material::StaticClass()))
    {
      return GetMaterialManager();
    }
    else if (Class->IsSublcassOf(SkinMesh::StaticClass()))
    {
      return GetMeshManager();
    }
    else if (Class->IsSublcassOf(Mesh::StaticClass()))
    {
      return GetMeshManager();
    }
    else if (Class->IsSublcassOf(Shader::StaticClass()))
    {
      return GetShaderManager();
    }
    else if (Class->IsSublcassOf(SpriteSheet::StaticClass()))
    {
      return GetSpriteSheetManager();
    }
    else if (Class->IsSublcassOf(Texture::StaticClass()))
    {
      return GetTextureManager();
    }
    else if (Class->IsSublcassOf(CubeMap::StaticClass()))
    {
      return GetTextureManager();
    }
    else if (Class->IsSublcassOf(RenderTarget::StaticClass()))
    {
      return GetTextureManager();
    }
    else if (Class->IsSublcassOf(Scene::StaticClass()))
    {
      return GetSceneManager();
    }

    return nullptr;
  }

  UIManager* GetUIManager() { return Main::GetInstance()->m_uiManager; }

  HandleManager* GetHandleManager()
  {
    if (Main* instance = Main::GetInstance_noexcep())
    {
      return &instance->m_handleManager;
    }

    return nullptr;
  }

  SkeletonManager* GetSkeletonManager() { return Main::GetInstance()->m_skeletonManager; }

  FileManager* GetFileManager() { return Main::GetInstance()->m_fileManager; }

  ObjectFactory* GetObjectFactory() { return Main::GetInstance()->m_objectFactory; }

  TKStats* GetTKStats()
  {
    if (Main* main = Main::GetInstance_noexcep())
    {
      return main->m_tkStats;
    }
    else
    {
      return nullptr;
    }
  }

  WorkerManager* GetWorkerManager() { return Main::GetInstance()->m_workerManager; }

  GpuProgramManager* GetGpuProgramManager() { return Main::GetInstance()->m_gpuProgramManager; }

  EngineSettings& GetEngineSettings() { return *Main::GetInstance()->m_engineSettings; }

  String DefaultAbsolutePath()
  {
    static String cur = std::filesystem::current_path().string();
    static StringArray splits;
    Split(cur, GetPathSeparatorAsStr(), splits);
    splits.erase(splits.end() - 1);
    splits.push_back("Resources");
    splits.push_back("Engine");
    static String res = ConcatPaths(splits);

    return res;
  }

  TK_API String ConfigPath()
  {
    StringView path = Main::GetInstance()->GetConfigPath();
    if (!path.empty())
    {
      return String(path);
    }

    return ConcatPaths({".", "..", "Config"});
  }

  TK_API String EngineSettingsPath() { return ConcatPaths({ConfigPath(), "Engine.settings"}); }

  String DefaultPath()
  {
    static const String res = ConcatPaths({"..", "Resources", "Engine"});
    return res;
  }

  String ResourcePath(bool def)
  {
    if (!def)
    {
      String& path = Main::GetInstance()->m_resourceRoot;
      if (!path.empty())
      {
        return path;
      }
    }

    return DefaultPath();
  }

  String ProcessPath(const String& file, const String& prefix, bool def)
  {
    if (HasToolKitRoot(file))
    {
      // Restore the ToolKit with a proper relative path.
      constexpr int length = sizeof("ToolKit");
      String modified      = file.substr(length);
      String path          = ConcatPaths({ResourcePath(true), prefix, modified});
      return path;
    }

    String path = ConcatPaths({ResourcePath(def), prefix, file});
    return path;
  }

  String TexturePath(const String& file, bool def) { return ProcessPath(file, "Textures", def); }

  String MeshPath(const String& file, bool def) { return ProcessPath(file, "Meshes", def); }

  String FontPath(const String& file, bool def) { return ProcessPath(file, "Fonts", def); }

  String SpritePath(const String& file, bool def) { return ProcessPath(file, "Sprites", def); }

  String AudioPath(const String& file, bool def) { return ProcessPath(file, "Audio", def); }

  String AnimationPath(const String& file, bool def) { return ProcessPath(file, "Meshes", def); }

  String SkeletonPath(const String& file, bool def) { return ProcessPath(file, "Meshes", def); }

  String ShaderPath(const String& file, bool def) { return ProcessPath(file, "Shaders", def); }

  String MaterialPath(const String& file, bool def) { return ProcessPath(file, "Materials", def); }

  String ScenePath(const String& file, bool def) { return ProcessPath(file, "Scenes", def); }

  String PrefabPath(const String& file, bool def) { return ProcessPath(file, "Prefabs", def); }

  String LayerPath(const String& file, bool def) { return ProcessPath(file, "Layers", def); }

  TK_API String PluginPath(const String& file, bool def)
  {
    String path        = ConcatPaths({"Plugins", file, "Codes", "Bin"});
    path               = ProcessPath(file, path, def);

    String resourceStr = "Resources" + GetPathSeparatorAsStr();
    RemoveString(path, resourceStr);

    return path;
  }

  TK_API String PluginConfigPath(const String& file, bool def)
  {
    String path        = ConcatPaths({"Plugins", file, "Config"});
    path               = ProcessPath("Plugin.settings", path, def);

    String resourceStr = "Resources" + GetPathSeparatorAsStr();
    RemoveString(path, resourceStr);

    return path;
  }

  void Timing::Init(uint fps)
  {
    LastTime        = GetElapsedMilliSeconds();
    CurrentTime     = 0.0f;
    TargetDeltaTime = 1000.0f / float(fps);
    FramesPerSecond = fps;
    FrameCount      = 0;
    TimeAccum       = 0.0f;
  }

  float Timing::GetDeltaTime() { return CurrentTime - LastTime; }

} //  namespace ToolKit
