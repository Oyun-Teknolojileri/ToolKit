#pragma once

/**
* @file ToolKit.h Header for Main, the manager class to access all the 
* functionalities of the ToolKit framework.
*/

#include "Animation.h"
#include "Audio.h"
#include "Camera.h"
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
#include "UIManager.h"
#include "Scene.h"
#include "Renderer.h"
#include "Material.h"
#include "Types.h"
#include "Skeleton.h"
#include "FileManager.h"

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
    ULongID m_baseHandle = 1000;  //!< Starting value of the handles.
  };

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
    UIManager* m_uiManager = nullptr;
    SkeletonManager* m_skeletonManager = nullptr;
    HandleManager m_handleManager;
    FileManager* m_fileManager = nullptr;

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
  TK_API UIManager* GetUIManager();
  TK_API HandleManager* GetHandleManager();
  TK_API SkeletonManager* GetSkeletonManager();
  TK_API FileManager* GetFileManager();

  TK_API String DefaultPath();
  TK_API String DefaultAbsolutePath();
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

}  // namespace ToolKit
