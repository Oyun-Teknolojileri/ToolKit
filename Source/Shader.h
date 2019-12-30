#pragma once

#include "Resource.h"
#include "ResourceManager.h"

namespace ToolKit
{

  enum class Uniform
  {
    PROJECT_MODEL_VIEW,
    MODEL,
    INV_TR_MODEL,
    LIGHT_DATA,
    CAM_DATA,
    COLOR,
    FRAME_COUNT
  };

  class Shader : public Resource
  {
  public:
    Shader();
    Shader(std::string file);
    ~Shader();

    void Load();
    void Init(bool flushClientSideArray = true);

  public:
    GLuint m_type = GL_VERTEX_SHADER;
    GLuint m_shaderHandle = 0;
    std::vector<Uniform> m_uniforms;
    std::string m_source;
  };

  class ShaderManager : public ResourceManager<Shader>
  {
  public:
    void Init();
  };

}