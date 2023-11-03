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

#include "ToolKit.h"

#include "Audio.h"
#include "EngineSettings.h"
#include "FileManager.h"
#include "Logger.h"
#include "Material.h"
#include "Mesh.h"
#include "Meta.h"
#include "Object.h"
#include "ObjectFactory.h"
#include "PluginManager.h"
#include "RenderSystem.h"
#include "Scene.h"
#include "Shader.h"
#include "TKOpenGL.h"
#include "UIManager.h"

#include "DebugNew.h"

namespace ToolKit
{
  HandleManager::HandleManager()
  {
    uint64 seed = time(nullptr) + ((uint64) (this) ^ m_randomXor[0]);
    Xoroshiro128PlusSeed(m_randomXor, seed);
  }

  ULongID HandleManager::GenerateHandle()
  {
    //TODO delete here
    if (m_uniqueIDs.size() > 10000000)
    {
      volatile int x = 5;
    }

    ULongID id = Xoroshiro128Plus(m_randomXor);
    // If collision happens, change generate new id
    while (m_uniqueIDs.find(id) != m_uniqueIDs.end() || id == 0)
    {
      id = Xoroshiro128Plus(m_randomXor);
    }
    m_uniqueIDs.insert(id);
    return id; // non zero
  }

  void HandleManager::ReleaseHandle(ULongID val)
  {
    //TODO delete here
    if (m_uniqueIDs.size() > 10000000)
    {
      volatile int x = 5;
    }

    auto it = m_uniqueIDs.find(val);
    if (it != m_uniqueIDs.end())
    {
      m_uniqueIDs.erase(it);
    }
  }

  Main* Main::m_proxy = nullptr;

  Main::Main()
  {
    // Start Timer.
    GetElapsedMilliSeconds();

    m_logger = new Logger();
    m_logger->Log("Main Constructed");
  }

  Main::~Main()
  {
    assert(m_initiated == false && "Uninitiate before destruct");

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

    m_engineSettings = new EngineSettings();
    m_objectFactory  = new ObjectFactory();
    m_objectFactory->Init();

    m_renderSys       = new RenderSystem();
    m_pluginManager   = new PluginManager();
    m_animationMan    = new AnimationManager();
    m_animationPlayer = new AnimationPlayer();
    m_textureMan      = new TextureManager();
    m_meshMan         = new MeshManager();
    m_spriteSheetMan  = new SpriteSheetManager();
    m_audioMan        = new AudioManager();
    m_shaderMan       = new ShaderManager();
    m_materialManager = new MaterialManager();
    m_sceneManager    = new SceneManager();
    m_uiManager       = new UIManager();
    m_skeletonManager = new SkeletonManager();
    m_fileManager     = new FileManager();

    m_preInitiated    = true;
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

    m_pluginManager->Init();
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

  void Main::Uninit()
  {
    m_logger->Log("Main Uninit");

    m_animationPlayer->m_records.clear();
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
  }

  void Main::SetConfigPath(StringView cfgPath) { m_cfgPath = cfgPath; }

  StringView Main::GetConfigPath() { return m_cfgPath; }

  Main* Main::GetInstance()
  {
    assert(m_proxy && "ToolKit is not initialized.");
    return m_proxy;
  }

  void Main::SetProxy(Main* proxy)
  {
    bool singular = m_proxy == nullptr || m_proxy == proxy;
    assert(singular && "You can only have one instance of the main");
    if (singular)
    {
      m_proxy = proxy;
    }
  }

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

  HandleManager* GetHandleManager() { return &Main::GetInstance()->m_handleManager; }

  SkeletonManager* GetSkeletonManager() { return Main::GetInstance()->m_skeletonManager; }

  FileManager* GetFileManager() { return Main::GetInstance()->m_fileManager; }

  TK_API ObjectFactory* GetObjectFactory() { return Main::GetInstance()->m_objectFactory; }

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

  void Timing::Init(uint fps)
  {
    LastTime    = GetElapsedMilliSeconds();
    CurrentTime = 0.0f;
    DeltaTime   = 1000.0f / float(fps);
    FrameCount  = 0;
    TimeAccum   = 0.0f;
  }

} //  namespace ToolKit
