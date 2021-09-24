#include "stdafx.h"
#include "ToolKit.h"
#include "DebugNew.h"

namespace ToolKit
{

  Main Main::m_instance;

  Main::~Main()
  {
  }

  void Main::Init()
  {
    Logger::GetInstance()->Log("ToolKit Initialization");
    PluginManager::GetInstance()->Init();
    
    m_animationMan.Init();
    m_textureMan.Init();
    m_meshMan.Init();
    m_spriteSheetMan.Init();
    m_audioMan.Init();
    m_shaderMan.Init();
    m_materialManager.Init();
    m_sceneManager.Init();

    m_initiated = true;
  }

  void Main::Uninit()
  {
    PluginManager::GetInstance()->UnInit();

    m_animationPlayer.m_records.clear();
    m_animationMan.Uninit();
    m_textureMan.Uninit();
    m_meshMan.Uninit();
    m_spriteSheetMan.Uninit();
    m_audioMan.Uninit();
    m_shaderMan.Uninit();
    m_materialManager.Uninit();
    m_materialManager.Uninit();
    m_sceneManager.Uninit();

    m_initiated = false;
  }

  Main::Main()
  {
  }

  Main* Main::GetInstance()
  {
    return &m_instance;
  }

  AnimationManager* GetAnimationManager()
  {
    return &Main::GetInstance()->m_animationMan;
  }

  AnimationPlayer* GetAnimationPlayer()
  {
    return &Main::GetInstance()->m_animationPlayer;
  }

  AudioManager* GetAudioManager()
  {
    return &Main::GetInstance()->m_audioMan;
  }

  MaterialManager* GetMaterialManager()
  {
    return &Main::GetInstance()->m_materialManager;
  }

  MeshManager* GetMeshManager()
  {
    return &Main::GetInstance()->m_meshMan;
  }

  ShaderManager* GetShaderManager()
  {
    return &Main::GetInstance()->m_shaderMan;
  }

  SpriteSheetManager* GetSpriteSheetManager()
  {
    return &Main::GetInstance()->m_spriteSheetMan;
  }

  TextureManager* GetTextureManager()
  {
    return &Main::GetInstance()->m_textureMan;
  }

  SceneManager* GetSceneManager()
  {
    return &Main::GetInstance()->m_sceneManager;
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
    case ResourceType::Base:
    default:
      assert(false);
      break;
    }

    return nullptr;
  }

  PluginManager* GetPluginManager()
  {
    return PluginManager::GetInstance();
  }

  String DefaultPath()
  {
    static String def = ConcatPaths({ ".", "..", "Resources" });
    return def;
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

}
