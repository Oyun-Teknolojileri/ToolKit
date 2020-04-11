#pragma once

#include "Types.h"
#include "Texture.h"
#include "Mesh.h"
#include "SpriteSheet.h"
#include "Audio.h"
#include "Animation.h"
#include "Material.h"
#include "Shader.h"

namespace ToolKit
{
  #define SafeDel(ptr) {delete ptr; ptr = nullptr;}
  #define SafeDelArray(ptr) {delete [] ptr; ptr = nullptr;}

  extern std::string TexturePath(std::string file);
  extern std::string MeshPath(std::string file);
  extern std::string FontPath(std::string file);
  extern std::string SpritePath(std::string file);
  extern std::string AudioPath(std::string file);
  extern std::string AnimationPath(std::string file);
  extern std::string SkeletonPath(std::string file);
  extern std::string ShaderPath(std::string file);
  extern std::string MaterialPath(std::string file);

  static const glm::vec3 X_AXIS = glm::vec3(1.0f, 0.0f, 0.0f);
  static const glm::vec3 Y_AXIS = glm::vec3(0.0f, 1.0f, 0.0f);
  static const glm::vec3 Z_AXIS = glm::vec3(0.0f, 0.0f, 1.0f);

  static const EntityId NULL_ENTITY = 0;

  enum class AxisLabel
  {
    None = -1, // Order matters. Don't change.
    X,
    Y,
    Z,
    XY,
    XZ,
    YZ
  };

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
    TextureManager m_textureMan;
    MeshManager m_meshMan;
    SkinMeshManager m_skinMeshMan;
    SpriteSheetManager m_spriteSheetMan;
    AudioManager m_audioMan;
    AnimationManager m_animationMan;
    AnimationPlayer m_animationPlayer;
    ShaderManager m_shaderMan;
    MaterialManager m_materialManager;

    bool m_initiated = false;
  };

}
