#pragma once

#include "ParameterBlock.h"
#include "Resource.h"
#include "ResourceManager.h"

#include <unordered_map>
#include <vector>

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
    FRAME_COUNT,
    GRID_SETTINGS,
    EXPOSURE,
    PROJECTION_VIEW_NO_TR,
    USE_IBL,
    IBL_INTENSITY,
    IBL_IRRADIANCE,
    DIFFUSE_TEXTURE_IN_USE,
    COLOR_ALPHA
  };

  class TK_API Shader : public Resource
  {
   public:
    TKResourceType(Shader)

    Shader();
    explicit Shader(String file);
    virtual ~Shader();

    void Load() override;
    void Init(bool flushClientSideArray = true) override;
    void UnInit() override;
    void SetShaderParameter(String param, const ParameterVariant& val);

    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

   public:
    std::unordered_map<String, ParameterVariant> m_shaderParams;

    String m_tag;
    GraphicTypes m_shaderType = GraphicTypes::VertexShader;
    uint m_shaderHandle       = 0;
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
    void Init() override;
    bool CanStore(ResourceType t) override;
    ResourcePtr CreateLocal(ResourceType type) override;
  };

} // namespace ToolKit
