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
    virtual ~Main();

    Main(Main const&) = delete;
    void operator=(Main const&) = delete;

    virtual void Init();
    virtual void Uninit();
    static Main* GetInstance();

  private:
    Main();

  private:
    static Main m_instance;

  public:
    AnimationManager m_animationMan;
    AnimationPlayer m_animationPlayer;
    AudioManager m_audioMan;
    MaterialManager m_materialManager;
    MeshManager m_meshMan;
    ShaderManager m_shaderMan;
    SpriteSheetManager m_spriteSheetMan;
    TextureManager m_textureMan;
    SceneManager m_sceneManager;
    PluginManager m_pluginManager;
    Renderer m_renderer;

    bool m_initiated = false;
    String m_resourceRoot;
  };

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
