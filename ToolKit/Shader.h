#pragma once

#include "Resource.h"
#include "ResourceManager.h"
#include "ParameterBlock.h"

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

  class TK_API Shader : public Resource
  {
  public:
    TKResouceType(Shader)

    Shader();
    Shader(String file);
    virtual ~Shader();

    virtual void Load() override;
    virtual void Init(bool flushClientSideArray = true) override;
    virtual void UnInit() override;
    void SetShaderParameter(String param, const ParameterVariant& val);

    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

  public:
    std::unordered_map<String, ParameterVariant> m_shaderParams;

    String m_tag;
    GraphicTypes m_shaderType = GraphicTypes::VertexShader;
    uint m_shaderHandle = 0;
    std::vector<Uniform> m_uniforms;
    String m_source;
  };

  class TK_API Program
  {
  public:
    Program();
    Program(ShaderPtr vertex, ShaderPtr fragment);
    ~Program();

  public:
    uint m_handle = 0;
    String m_tag;
    ShaderPtrArray m_shaders;
  };

  class TK_API ShaderManager : public ResourceManager
  {
  public:
    ShaderManager();
    virtual ~ShaderManager();
    virtual void Init() override;
    virtual bool CanStore(ResourceType t);
    virtual ResourcePtr CreateLocal(ResourceType type);
  };

}