#include "ToolKit.h"

#define GLAD_GLES2_IMPLEMENTATION
#include "gles2.h"

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
    m_timing.Initialize(m_engineSettings.Graphics.FPS);

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

  void EngineSettings::SerializeWindow(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* window = doc->allocate_node(rapidxml::node_element, "Window");
    doc->append_node(window);

    using namespace std;
    const EngineSettings::GraphicSettings& gfx = Graphics;

    const auto writeAttr1 = [&](StringView name, StringView val)
    { WriteAttr(window, doc, name.data(), val.data()); };
    // serialize window.
    writeAttr1("width", to_string(Window.Width));
    writeAttr1("height", to_string(Window.Height));
    writeAttr1(XmlNodeName, Window.Name);
    writeAttr1("fullscreen", to_string(Window.FullScreen));
  }

  void EngineSettings::DeSerializeWindow(XmlDocument* doc, XmlNode* parent)
  {
    XmlNode* node = doc->first_node("Window");
    if (node == nullptr)
    {
      return;
    }
    ReadAttr(node, "width", Window.Width);
    ReadAttr(node, "height", Window.Height);
    ReadAttr(node, "name", Window.Name);
    ReadAttr(node, "fullscreen", Window.FullScreen);
  }

  void EngineSettings::SerializePostProcessing(XmlDocument* doc,
                                               XmlNode* parent) const
  {
    XmlNode* settings =
        doc->allocate_node(rapidxml::node_element, "PostProcessing");
    doc->append_node(settings);

    const EngineSettings::PostProcessingSettings& gfx = PostProcessing;

    const auto writeAttrFn = [&](StringView name, StringView val) -> void
    { WriteAttr(settings, doc, name.data(), val.data()); };

    // Serialize Graphics struct.
    using namespace std;
    writeAttrFn("TonemappingEnabled", to_string(gfx.TonemappingEnabled));
    writeAttrFn("TonemapperMode", to_string((int) gfx.TonemapperMode));
    writeAttrFn("EnableBloom", to_string((int) gfx.BloomEnabled));
    writeAttrFn("BloomIntensity", to_string(gfx.BloomIntensity));
    writeAttrFn("BloomThreshold", to_string(gfx.BloomThreshold));
    writeAttrFn("BloomIterationCount", to_string(gfx.BloomIterationCount));
    writeAttrFn("GammaCorrectionEnabled",
                to_string(gfx.GammaCorrectionEnabled));
    writeAttrFn("Gamma", to_string(gfx.Gamma));
    writeAttrFn("SSAOEnabled", to_string(gfx.SSAOEnabled));
    writeAttrFn("SSAORadius", to_string(gfx.SSAORadius));
    writeAttrFn("SSAOBias", to_string(gfx.SSAOBias));
    writeAttrFn("DepthOfFieldEnabled", to_string(gfx.DepthOfFieldEnabled));
    writeAttrFn("FocusPoint", to_string(gfx.FocusPoint));
    writeAttrFn("FocusScale", to_string(gfx.FocusScale));
    writeAttrFn("DofQuality", to_string((int) gfx.DofQuality));
    writeAttrFn("FXAAEnabled", to_string(gfx.FXAAEnabled));
  }

  void EngineSettings::DeSerializePostProcessing(XmlDocument* doc,
                                                 XmlNode* parent)
  {
    XmlNode* node  = doc->first_node("PostProcessing");
    // if post processing settings is not exist use default settings
    PostProcessing = PostProcessingSettings {};
    if (node == nullptr)
    {
      return;
    }

    ReadAttr(node, "TonemappingEnabled", PostProcessing.TonemappingEnabled);
    ReadAttr(node, "BloomEnabled", PostProcessing.BloomEnabled);
    ReadAttr(node, "BloomIntensity", PostProcessing.BloomIntensity);
    ReadAttr(node, "BloomThreshold", PostProcessing.BloomThreshold);
    ReadAttr(node, "BloomIterationCount", PostProcessing.BloomIterationCount);
    ReadAttr(node,
             "GammaCorrectionEnabled",
             PostProcessing.GammaCorrectionEnabled);
    ReadAttr(node, "Gamma", PostProcessing.Gamma);
    ReadAttr(node, "SSAOEnabled", PostProcessing.SSAOEnabled);
    ReadAttr(node, "SSAORadius", PostProcessing.SSAORadius);
    ReadAttr(node, "SSAOBias", PostProcessing.SSAOBias);
    ReadAttr(node, "DepthOfFieldEnabled", PostProcessing.DepthOfFieldEnabled);
    ReadAttr(node, "FocusPoint", PostProcessing.FocusPoint);
    ReadAttr(node, "FocusScale", PostProcessing.FocusScale);
    ReadAttr(node, "FXAAEnabled", PostProcessing.FXAAEnabled);
    ReadAttr(node, "TonemapperMode", *(int*) &PostProcessing.TonemapperMode);
    ReadAttr(node, "DofQuality", *(int*) &PostProcessing.DofQuality);
  }

  void EngineSettings::SerializeGraphics(XmlDocument* doc,
                                         XmlNode* parent) const
  {
    XmlNode* settings = doc->allocate_node(rapidxml::node_element, "Graphics");
    doc->append_node(settings);
    const EngineSettings::GraphicSettings& gfx = Graphics;

    WriteAttr(settings, doc, "MSAA", std::to_string(gfx.MSAA));
    WriteAttr(settings, doc, "FPS", std::to_string(gfx.FPS));
  }

  void EngineSettings::DeSerializeGraphics(XmlDocument* doc, XmlNode* parent)
  {
    XmlNode* node = doc->first_node("Graphics");
    if (node == nullptr)
    {
      return;
    }
    ReadAttr(node, "MSAA", Graphics.MSAA);
    ReadAttr(node, "FPS", Graphics.FPS);
  }

  void EngineSettings::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    SerializeGraphics(doc, parent);
    SerializePostProcessing(doc, parent);
    SerializeWindow(doc, parent);
  }

  void EngineSettings::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    assert(doc && "doc must not be null");
    DeSerializeWindow(doc, parent);
    DeSerializeGraphics(doc, parent);
    DeSerializePostProcessing(doc, parent);
  }
} //  namespace ToolKit
