#include "ToolKit.h"

#include <algorithm>
#include <filesystem>
#include <string>

#include "DebugNew.h"

namespace ToolKit
{
  ULongID HandleManager::GetNextHandle()
  {
    assert(m_baseHandle < m_maxIdLimit && "Generated id is too long.");
    return ++m_baseHandle;
  }

  void HandleManager::SetMaxHandle(ULongID val)
  {
    m_baseHandle = glm::max(m_baseHandle, val);
    assert(m_baseHandle < m_maxIdLimit && "Generated id is too long.");
  }

  Main* Main::m_proxy = nullptr;

  Main::Main()
  {
    m_logger = new Logger();

    m_renderer = new Renderer();
    m_pluginManager = new PluginManager();
    m_animationMan = new AnimationManager();
    m_animationPlayer = new AnimationPlayer();
    m_textureMan = new TextureManager();
    m_meshMan = new MeshManager();
    m_spriteSheetMan = new SpriteSheetManager();
    m_audioMan = new AudioManager();
    m_shaderMan = new ShaderManager();
    m_materialManager = new MaterialManager();
    m_sceneManager = new SceneManager();
    m_uiManager = new UIManager();
    m_skeletonManager = new SkeletonManager();
    m_fileManager = new FileManager();

    m_entityFactory = new EntityFactory();

    m_logger->Log("Main Constructed");
  }

  Main::~Main()
  {
    assert(m_initiated == false && "Uninitiate before deconstruct");

    SafeDel(m_renderer);
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

    SafeDel(m_entityFactory);

    m_logger->Log("Main Deconstructed");
    SafeDel(m_logger);
  }

  void Main::Init()
  {
    // Start Timer.
    GetElapsedMilliSeconds();

    m_logger->Log("ToolKit Initialization");

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

    m_initiated = true;
  }

  void Main::Uninit()
  {
    m_logger->Log("ToolKit Unitialization");

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

    // After all the resources, we can safely free modules.
    m_pluginManager->UnInit();

    m_initiated = false;
  }

  Main* Main::GetInstance()
  {
    assert(m_proxy);
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

  Logger* GetLogger()
  {
    return Main::GetInstance()->m_logger;
  }

  Renderer* GetRenderer()
  {
    return Main::GetInstance()->m_renderer;
  }

  AnimationManager* GetAnimationManager()
  {
    return Main::GetInstance()->m_animationMan;
  }

  AnimationPlayer* GetAnimationPlayer()
  {
    return Main::GetInstance()->m_animationPlayer;
  }

  AudioManager* GetAudioManager()
  {
    return Main::GetInstance()->m_audioMan;
  }

  MaterialManager* GetMaterialManager()
  {
    return Main::GetInstance()->m_materialManager;
  }

  MeshManager* GetMeshManager()
  {
    return Main::GetInstance()->m_meshMan;
  }

  ShaderManager* GetShaderManager()
  {
    return Main::GetInstance()->m_shaderMan;
  }

  SpriteSheetManager* GetSpriteSheetManager()
  {
    return Main::GetInstance()->m_spriteSheetMan;
  }

  TextureManager* GetTextureManager()
  {
    return Main::GetInstance()->m_textureMan;
  }

  SceneManager* GetSceneManager()
  {
    return Main::GetInstance()->m_sceneManager;
  }

  PluginManager* GetPluginManager()
  {
    return Main::GetInstance()->m_pluginManager;
  }

  ResourceManager* GetResourceManager(ResourceType type)
  {
    switch (type)
    {
    case ResourceType::Animation:
      return GetAnimationManager();
    case ResourceType::Audio:
      return GetAudioManager();
    case ResourceType::Material:
      return GetMaterialManager();
    case ResourceType::SkinMesh:
    case ResourceType::Mesh:
      return GetMeshManager();
    case ResourceType::Shader:
      return GetShaderManager();
    case ResourceType::SpriteSheet:
      return GetSpriteSheetManager();
    case ResourceType::Texture:
    case ResourceType::CubeMap:
    case ResourceType::RenderTarget:
      return GetTextureManager();
    case ResourceType::Scene:
      return GetSceneManager();
    case ResourceType::Base:
    default:
      assert(false);
      break;
    }

    return nullptr;
  }

  UIManager* GetUIManager()
  {
    return Main::GetInstance()->m_uiManager;
  }

  HandleManager* GetHandleManager()
  {
    return &Main::GetInstance()->m_handleManager;
  }

  TK_API SkeletonManager* GetSkeletonManager()
  {
    return Main::GetInstance()->m_skeletonManager;
  }

  TK_API FileManager* GetFileManager()
  {
    return Main::GetInstance()->m_fileManager;
  }

  TK_API EntityFactory* GetEntityFactory()
  {
    return Main::GetInstance()->m_entityFactory;
  }

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

  String DefaultPath()
  {
    static String res = ConcatPaths({ "..", "Resources", "Engine" });

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

  /* 
  * When dynamically created resources refer to default assets,
  * they got saved with an altered relative path which starts with ToolKit.
  * Check Util.h GetRelativeResourcePath() for more.
  * So here, we try to detect defaul assets.
  */
  bool CheckForRelative(const String& file)
  {
    return file.find("ToolKit") != String::npos;
  }

  String ProcessPath(const String& file, const String& prefix, bool def)
  {
    if (CheckForRelative(file))
    {
      constexpr int length = sizeof("ToolKit");
      String modified = file.substr(length);
      String path = ConcatPaths({ ResourcePath(true), prefix, modified });
      return path;
    }

    String path = ConcatPaths({ ResourcePath(def), prefix, file });
    return path;
  }

  String TexturePath(const String& file, bool def)
  {
    return ProcessPath(file, "Textures", def);
  }

  String MeshPath(const String& file, bool def)
  {
    return ProcessPath(file, "Meshes", def);
  }

  String FontPath(const String& file, bool def)
  {
    return ProcessPath(file, "Fonts", def);
  }

  String SpritePath(const String& file, bool def)
  {
    return ProcessPath(file, "Sprites", def);
  }

  String AudioPath(const String& file, bool def)
  {
    return ProcessPath(file, "Audio", def);
  }

  String AnimationPath(const String& file, bool def)
  {
    return ProcessPath(file, "Meshes", def);
  }

  String SkeletonPath(const String& file, bool def)
  {
    return ProcessPath(file, "Meshes", def);
  }

  String ShaderPath(const String& file, bool def)
  {
    return ProcessPath(file, "Shaders", def);
  }

  String MaterialPath(const String& file, bool def)
  {
    return ProcessPath(file, "Materials", def);
  }

  String ScenePath(const String& file, bool def)
  {
    return ProcessPath(file, "Scenes", def);
  }

  String PrefabPath(const String& file, bool def)
  {
    return ProcessPath(file, "Prefabs", def);
  }

}  //  namespace ToolKit
