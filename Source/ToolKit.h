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
    SkinMeshManager m_skinMeshMan;
    SpriteSheetManager m_spriteSheetMan;
    TextureManager m_textureMan;

    bool m_initiated = false;
  };

	AnimationManager* GetAnimationManager();
  AnimationPlayer* GetAnimationPlayer();
	AudioManager* GetAudioManager();
  MaterialManager* GetMaterialManager();
  MeshManager* GetMeshManager();
  ShaderManager* GetShaderManager();
  SkinMeshManager* GetSkinMeshManager();
  SpriteSheetManager* GetSpriteSheetManager();
  TextureManager* GetTextureManager();

  String ResourcePath();
	String TexturePath(const String& file);
	String MeshPath(const String& file);
	String FontPath(const String& file);
	String SpritePath(const String& file);
	String AudioPath(const String& file);
	String AnimationPath(const String& file);
	String SkeletonPath(const String& file);
	String ShaderPath(const String& file);
	String MaterialPath(const String& file);
  String ScenePath(const String& file);

}
