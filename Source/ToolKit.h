#pragma once

#include "Animation.h"
#include "Audio.h"
#include "Directional.h"
#include "Drawable.h"
#include "Entity.h"
#include "Material.h"
#include "Mesh.h"
#include "Node.h"
#include "Primative.h"
#include "RenderState.h"
#include "Renderer.h"
#include "Shader.h"
#include "SpriteSheet.h"
#include "StateMachine.h"
#include "Surface.h"
#include "Texture.h"
#include "Scene.h"
#include "PluginManager.h"
#include "Types.h"

namespace ToolKit
{

  class Main
  {
  public:
    Main();
    virtual ~Main();

    Main(Main const&) = delete;
    void operator=(Main const&) = delete;

    virtual void Init();
    virtual void Uninit();
    static Main* GetInstance();
    static void SetProxy(Main* proxy);

  public:
    AnimationManager* m_animationMan = nullptr;
    AnimationPlayer* m_animationPlayer = nullptr;
    AudioManager* m_audioMan = nullptr;
    MaterialManager* m_materialManager = nullptr;
    MeshManager* m_meshMan = nullptr;
    ShaderManager* m_shaderMan = nullptr;
    SpriteSheetManager* m_spriteSheetMan = nullptr;
    TextureManager* m_textureMan = nullptr;
    SceneManager* m_sceneManager = nullptr;
    PluginManager* m_pluginManager = nullptr;
    Renderer* m_renderer = nullptr;
    Logger* m_logger = nullptr;

    bool m_initiated = false;
    String m_resourceRoot;

  private:
    static Main* m_proxy;
  };

  // Accessors.
  Logger* GetLogger();
  Renderer* GetRenderer();
  AnimationManager* GetAnimationManager();
  AnimationPlayer* GetAnimationPlayer();
  AudioManager* GetAudioManager();
  MaterialManager* GetMaterialManager();
  MeshManager* GetMeshManager();
  ShaderManager* GetShaderManager();
  SpriteSheetManager* GetSpriteSheetManager();
  TextureManager* GetTextureManager();
  SceneManager* GetSceneManager();
  PluginManager* GetPluginManager();
  ResourceManager* GetResourceManager(ResourceType type);

  String DefaultPath();
  String ResourcePath(bool def = false);
  String TexturePath(const String& file, bool def = false);
  String MeshPath(const String& file, bool def = false);
  String FontPath(const String& file, bool def = false);
  String SpritePath(const String& file, bool def = false);
  String AudioPath(const String& file, bool def = false);
  String AnimationPath(const String& file, bool def = false);
  String SkeletonPath(const String& file, bool def = false);
  String ShaderPath(const String& file, bool def = false);
  String MaterialPath(const String& file, bool def = false);
  String ScenePath(const String& file, bool def = false);

}
