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
#include "Events.h"
#include "Types.h"

namespace ToolKit
{

  class TK_API Main
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
    SurfaceObserver* m_surfaceObserver = nullptr;

    bool m_initiated = false;
    String m_resourceRoot;
    EventPool m_eventPool;

  private:
    static Main* m_proxy;
  };

  // Accessors.
  TK_API Logger* GetLogger();
  TK_API Renderer* GetRenderer();
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
  TK_API SurfaceObserver* GetSurfaceObserver();

  TK_API String DefaultPath();
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

}
