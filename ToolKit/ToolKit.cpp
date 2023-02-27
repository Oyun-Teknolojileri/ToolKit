#include "ToolKit.h"

#include <algorithm>
#include <filesystem>
#include <memory>
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
    m_entityFactory   = new EntityFactory();

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
    SafeDel(m_entityFactory);
  }

  void Main::SetConfigPath(StringView cfgPath) { m_cfgPath = cfgPath; }

  StringView Main::GetConfigPath() { return m_cfgPath; }

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

  Logger* GetLogger() { return Main::GetInstance()->m_logger; }

  RenderSystem* GetRenderSystem() { return Main::GetInstance()->m_renderSys; }

  AnimationManager* GetAnimationManager()
  {
    return Main::GetInstance()->m_animationMan;
  }

  AnimationPlayer* GetAnimationPlayer()
  {
    return Main::GetInstance()->m_animationPlayer;
  }

  AudioManager* GetAudioManager() { return Main::GetInstance()->m_audioMan; }

  MaterialManager* GetMaterialManager()
  {
    return Main::GetInstance()->m_materialManager;
  }

  MeshManager* GetMeshManager() { return Main::GetInstance()->m_meshMan; }

  ShaderManager* GetShaderManager() { return Main::GetInstance()->m_shaderMan; }

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
    case ResourceType::Skeleton:
      return nullptr;
    case ResourceType::Base:
    default:
      assert(false);
      break;
    }

    return nullptr;
  }

  UIManager* GetUIManager() { return Main::GetInstance()->m_uiManager; }

  HandleManager* GetHandleManager()
  {
    return &Main::GetInstance()->m_handleManager;
  }

  SkeletonManager* GetSkeletonManager()
  {
    return Main::GetInstance()->m_skeletonManager;
  }

  FileManager* GetFileManager() { return Main::GetInstance()->m_fileManager; }

  EntityFactory* GetEntityFactory()
  {
    return Main::GetInstance()->m_entityFactory;
  }

  EngineSettings& GetEngineSettings()
  {
    return Main::GetInstance()->m_engineSettings;
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
      String modified      = file.substr(length);
      String path = ConcatPaths({ResourcePath(true), prefix, modified});
      return path;
    }

    String path = ConcatPaths({ResourcePath(def), prefix, file});
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

  String LayerPath(const String& file, bool def)
  {
    return ProcessPath(file, "Layers", def);
  }

  void EngineSettings::Serialize(XmlDocument* doc, XmlNode* node) const
  {
    XmlNode* settings = doc->allocate_node(rapidxml::node_element, "Settings");
    doc->append_node(settings);

    XmlNode* window = doc->allocate_node(rapidxml::node_element, "Window");
    settings->append_node(window);

    XmlNode* graphics = doc->allocate_node(rapidxml::node_element, "Graphics");

    using namespace std;
    const EngineSettings::GraphicSettings& gfx = Graphics;

    const auto writeAttr1 = [&](StringView name, StringView val)
    { WriteAttr(window, doc, name.data(), val.data()); };
    // serialize window
    writeAttr1("width", to_string(Window.Width));
    writeAttr1("height", to_string(Window.Height));
    writeAttr1(XmlNodeName, Window.Name);
    writeAttr1("fullscreen", to_string(Window.FullScreen));

    settings->append_node(graphics);

    const auto writeAttr = [&](StringView name, StringView val)
    { WriteAttr(graphics, doc, name.data(), val.data()); };

    // serialize Graphics struct
    writeAttr("MSAA", to_string(gfx.MSAA));
    writeAttr("FPS", to_string(gfx.FPS));
    writeAttr("TonemappingEnabled", to_string(gfx.TonemappingEnabled));
    writeAttr("TonemapperMode", to_string((int) gfx.TonemapperMode));
    writeAttr("EnableBloom", to_string((int) gfx.BloomEnabled));
    writeAttr("BloomIntensity", to_string(gfx.BloomIntensity));
    writeAttr("BloomThreshold", to_string(gfx.BloomThreshold));
    writeAttr("BloomIterationCount", to_string(gfx.BloomIterationCount));
    writeAttr("GammaCorrectionEnabled", to_string(gfx.GammaCorrectionEnabled));
    writeAttr("Gamma", to_string(gfx.Gamma));
    writeAttr("SSAOEnabled", to_string(gfx.SSAOEnabled));
    writeAttr("SSAORadius", to_string(gfx.SSAORadius));
    writeAttr("SSAOBias", to_string(gfx.SSAOBias));
    writeAttr("DepthOfFieldEnabled", to_string(gfx.DepthOfFieldEnabled));
    writeAttr("FocusPoint", to_string(gfx.FocusPoint));
    writeAttr("FocusScale", to_string(gfx.FocusScale));
    writeAttr("DofQuality", to_string((int) gfx.DofQuality));
    writeAttr("FXAAEnabled", to_string(gfx.FXAAEnabled));
  }

  void EngineSettings::DeSerialize(XmlDocument* doc, XmlNode* node)
  {
    assert(doc && "doc must not be null");

    XmlNode* settings = doc->first_node("Settings");
    XmlNode* node2    = settings->first_node("Window");

    const auto getInt = [&](int& val, StringView name) -> void
    {
      if (XmlAttribute* attr = node2->first_attribute(name.data()))
      {
        val = atoi(attr->value());
      }
    };

    const auto getFloat = [&](float& value, StringView name) -> void
    {
      if (XmlAttribute* attr = node2->first_attribute(name.data()))
      {
        value = (float) atof(attr->value());
      }
    };

    const auto getBool = [&](bool& value, StringView name) -> void
    {
      if (XmlAttribute* attr = node2->first_attribute(name.data()))
      {
        value = atoi(attr->value()) != 0;
      }
    };

    if (node2 != nullptr)
    {
      if (XmlAttribute* attr = node2->first_attribute("width"))
      {
        Window.Width = atoi(attr->value());
      }
      if (XmlAttribute* attr = node2->first_attribute("height"))
      {
        Window.Height = atoi(attr->value());
      }
      if (XmlAttribute* attr = node2->first_attribute(XmlNodeName.data()))
      {
        Window.Name = attr->value();
      }
      getBool(Window.FullScreen, "fullscreen");
    }

    if (node2 = settings->first_node("Graphics"))
    {
      getInt(Graphics.MSAA, "MSAA");
      getInt(Graphics.FPS, "FPS");
      getBool(Graphics.TonemappingEnabled, "TonemappingEnabled");
      getBool(Graphics.BloomEnabled, "BloomEnabled");
      getFloat(Graphics.BloomIntensity, "BloomIntensity");
      getFloat(Graphics.BloomThreshold, "BloomThreshold");
      getInt(Graphics.BloomIterationCount, "BloomIterationCount");
      getBool(Graphics.GammaCorrectionEnabled, "GammaCorrectionEnabled");
      getFloat(Graphics.Gamma, "Gamma");
      getBool(Graphics.SSAOEnabled, "SSAOEnabled");
      getFloat(Graphics.SSAORadius, "SSAORadius");
      getFloat(Graphics.SSAOBias, "SSAOBias");
      getBool(Graphics.DepthOfFieldEnabled, "DepthOfFieldEnabled");
      getFloat(Graphics.FocusPoint, "FocusPoint");
      getFloat(Graphics.FocusScale, "FocusScale");
      getBool(Graphics.FXAAEnabled, "FXAAEnabled");

      int toneMapperMode = (int) Graphics.TonemapperMode;
      int dofQuality     = (int) Graphics.DofQuality;
      getInt(toneMapperMode, "TonemapperMode");
      getInt(dofQuality, "DofQuality");
      Graphics.TonemapperMode = (TonemapMethod) toneMapperMode;
      Graphics.DofQuality     = (DoFQuality) dofQuality;
    }
  }
} //  namespace ToolKit
