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

#include <assert.h>
#include <string>
#include <unordered_map>
#include <vector>

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

	String TexturePath(String file);
	String MeshPath(String file);
	String FontPath(String file);
	String SpritePath(String file);
	String AudioPath(String file);
	String AnimationPath(String file);
	String SkeletonPath(String file);
	String ShaderPath(String file);
	String MaterialPath(String file);

}
