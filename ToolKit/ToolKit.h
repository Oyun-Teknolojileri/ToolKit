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

#include "Animation.h"
#include "Audio.h"
#include "Camera.h"
#include "Canvas.h"
#include "DofPass.h"
#include "Drawable.h"
#include "Entity.h"
#include "Events.h"
#include "FileManager.h"
#include "Material.h"
#include "Mesh.h"
#include "Node.h"
#include "Pass.h"
#include "PluginManager.h"
#include "PostProcessPass.h"
#include "Primative.h"
#include "RenderState.h"
#include "RenderSystem.h"
#include "Renderer.h"
#include "Scene.h"
#include "Shader.h"
#include "Skeleton.h"
#include "SpriteSheet.h"
#include "StateMachine.h"
#include "Surface.h"
#include "Texture.h"
#include "ToneMapPass.h"
#include "Types.h"
#include "UIManager.h"
#include "StringId.h"

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
    /**
     * Creates a handle that has not been allocated within the current runtime.
     * Increments the m_baseHandle by one.
     * @return A unique handle for the current runtime.
     */
    ULongID GetNextHandle();

    /**
     * This function allows to set the m_baseHandle to maximum of
     * val and m_baseHandle. That is m_baseHandle = max(m_baseHandle, val).
     * @param val The value to compare with current handle.
     */
    void SetMaxHandle(ULongID val);

   private:
    ULongID m_baseHandle = 1000; //!< Starting value of the handles.
    ULongID m_maxIdLimit = (std::numeric_limits<uint64_t>::max() / 10) * 9;
  };

  class TK_API EngineSettings : public Serializable
  {
   public:
    struct WindowSettings
    {
      String Name     = "ToolKit";
      uint Width      = 1024;
      uint Height     = 768;
      bool FullScreen = false;
    } Window;

    struct GraphicSettings
    {
      int MSAA = 2;
      int FPS  = 60;
    } Graphics;

    struct PostProcessingSettings
    {
      bool TonemappingEnabled      = true;
      TonemapMethod TonemapperMode = TonemapMethod::Aces;
      bool BloomEnabled            = true;
      float BloomIntensity         = 1.0f;
      float BloomThreshold         = 1.0f;
      int BloomIterationCount      = 5;
      bool GammaCorrectionEnabled  = true;
      float Gamma                  = 2.2f;
      bool SSAOEnabled             = true;
      float SSAORadius             = 0.5f;
      float SSAOBias               = 0.025f;
      float SSAOSpread             = 1.0f;
      bool DepthOfFieldEnabled     = false;
      float FocusPoint             = 10.0f;
      float FocusScale             = 5.0f;
      DoFQuality DofQuality        = DoFQuality::Normal;
      bool FXAAEnabled             = false;
    } PostProcessing;

    void SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerializeImp(XmlDocument* doc, XmlNode* parent) override;

    void SerializeWindow(XmlDocument* doc, XmlNode* parent) const;
    void DeSerializeWindow(XmlDocument* doc, XmlNode* parent);
    void SerializePostProcessing(XmlDocument* doc, XmlNode* parent) const;
    void DeSerializePostProcessing(XmlDocument* doc, XmlNode* parent);
    void SerializeGraphics(XmlDocument* doc, XmlNode* parent) const;
    void DeSerializeGraphics(XmlDocument* doc, XmlNode* parent);
  };

  struct Timing
  {
    void Initialize(uint fps)
    {
      LastTime    = GetElapsedMilliSeconds();
      CurrentTime = 0.0f;
      DeltaTime   = 1000.0f / float(fps);
      FrameCount  = 0;
      TimeAccum   = 0.0f;
    }

    float LastTime    = 0.0f;
    float CurrentTime = 0.0f;
    float DeltaTime   = 0.0f;
    float TimeAccum   = 0.0f;
    int FrameCount    = 0;
  };

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
    static void SetProxy(Main* proxy);

   public:
    Timing m_timing;
    AnimationManager* m_animationMan     = nullptr;
    AnimationPlayer* m_animationPlayer   = nullptr;
    AudioManager* m_audioMan             = nullptr;
    MaterialManager* m_materialManager   = nullptr;
    MeshManager* m_meshMan               = nullptr;
    ShaderManager* m_shaderMan           = nullptr;
    SpriteSheetManager* m_spriteSheetMan = nullptr;
    TextureManager* m_textureMan         = nullptr;
    SceneManager* m_sceneManager         = nullptr;
    PluginManager* m_pluginManager       = nullptr;
    Logger* m_logger                     = nullptr;
    UIManager* m_uiManager               = nullptr;
    SkeletonManager* m_skeletonManager   = nullptr;
    FileManager* m_fileManager           = nullptr;
    EntityFactory* m_entityFactory       = nullptr;
    RenderSystem* m_renderSys            = nullptr;
    HandleManager m_handleManager;

    bool m_preInitiated = false;
    bool m_initiated    = false;
    String m_resourceRoot;
    String m_cfgPath;
    EventPool m_eventPool;
    EngineSettings m_engineSettings;

   private:
    static Main* m_proxy;
  };

  // Accessors.
  TK_API Logger* GetLogger();
  TK_API RenderSystem* GetRenderSystem();
  TK_API AnimationManager* GetAnimationManager();
  TK_API AnimationPlayer* GetAnimationPlayer();
  TK_API AudioManager* GetAudioManager();
  TK_API MaterialManager* GetMaterialManager();
  TK_API MeshManager* GetMeshManager();
  TK_API ShaderManager* GetShaderManager();
  TK_API SpriteSheetManager* GetSpriteSheetManager();
  TK_API TextureManager* GetTextureManager();
  TK_API SceneManager* GetSceneManager();
  TK_API PluginManager* GetPluginManager();
  TK_API ResourceManager* GetResourceManager(ResourceType type);
  TK_API UIManager* GetUIManager();
  TK_API HandleManager* GetHandleManager();
  TK_API SkeletonManager* GetSkeletonManager();
  TK_API FileManager* GetFileManager();
  TK_API EntityFactory* GetEntityFactory();
  TK_API EngineSettings& GetEngineSettings();

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
