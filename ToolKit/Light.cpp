
#include "Light.h"

#include "Component.h"
#include "DirectionComponent.h"
#include "GL/glew.h"
#include "ToolKit.h"

#include <memory>
#include <string>

namespace ToolKit
{
  Light::Light()
  {
    Color_Define(Vec3(1.0f), "Light", 0, true, true, {true});
    Intensity_Define(
        1.0f, "Light", 90, true, true, {false, true, 0.0f, 100000.0f, 0.1f});
    CastShadow_Define(false, "Light", 90, true, true);
    NormalBias_Define(
        0.1f, "Light", 80, true, true, {false, true, 0.0f, 5.0f, 0.005f});
    FixedBias_Define(
        0.0f, "Light", 80, true, true, {false, true, 0, 100.0f, 0.1f});
    SlopedBias_Define(
        2.0f, "Light", 80, true, true, {false, true, 0.0f, 100.0f, 0.1f});
    ShadowResolution_Define(Vec2(1024.0f, 1024.0f),
                            "Light",
                            90,
                            true,
                            true,
                            {false, true, 32.0f, 4096.0f, 2.0f});
    PCFSampleSize_Define(
        7.0f, "Light", 90, true, true, {false, true, 0.1f, 100.0f, 0.1f});
    PCFKernelSize_Define(5, "Light", 90, true, true, {false, true, 0, 20, 1});
    ParameterEventConstructor();
  }

  Light::~Light()
  {
    UnInitShadowMap();
  }

  void Light::ParameterEventConstructor()
  {
    ParamShadowResolution().m_onValueChangedFn = [this](Value& oldVal,
                                                        Value& newVal) -> void {
      const Vec2 val = std::get<Vec2>(newVal);
      if (val.x > 0.0f && val.y > 0.0f &&
          !glm::epsilonEqual(val.x, 0.0f, 0.9f) &&
          !glm::epsilonEqual(val.y, 0.0f, 0.9f))
      {
        ReInitShadowMap();
      }
    };

    ParamPCFSampleSize().m_onValueChangedFn = [](Value& oldVal,
                                                 Value& newVal) -> void {
      if (glm::epsilonEqual(std::get<float>(newVal), 0.0f, 0.00001f))
      {
        newVal = 1.0f;
      }
    };
  }

  EntityType Light::GetType() const
  {
    return EntityType::Entity_Light;
  }

  void Light::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    Entity::Serialize(doc, parent);
  }

  void Light::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    ClearComponents(); // Read from file.
    Entity::DeSerialize(doc, parent);
    ParameterEventConstructor();
  }

  void Light::InitShadowMap()
  {
    if (m_shadowMapInitialized && !m_shadowMapResolutionChanged)
    {
      return;
    }

    Vec2 res   = GetShadowResolutionVal();
    m_shadowRt = new RenderTarget((int) res.x,
                                  (int) res.y,
                                  {0,
                                   true,
                                   GraphicTypes::Target2D,
                                   GraphicTypes::UVClampToBorder,
                                   GraphicTypes::UVClampToBorder,
                                   GraphicTypes::UVClampToBorder,
                                   GraphicTypes::SampleNearest,
                                   GraphicTypes::SampleNearest,
                                   GraphicTypes::FormatDepthComponent,
                                   GraphicTypes::FormatDepthComponent,
                                   GraphicTypes::TypeFloat,
                                   Vec4(1.0f)});
    m_shadowRt->Init();

    m_depthFramebuffer = new Framebuffer();
    m_depthFramebuffer->Init({(uint) res.x, (uint) res.y, 0, false, false});
    m_depthFramebuffer->SetAttachment(Framebuffer::Attachment::DepthAttachment,
                                      m_shadowRt);

    InitShadowMapDepthMaterial();
    m_shadowMapInitialized = true;
  }

  void Light::UnInitShadowMap()
  {
    if (!m_shadowMapInitialized)
    {
      return;
    }

    SafeDel(m_depthFramebuffer);
    SafeDel(m_shadowRt);
    m_shadowMapInitialized = false;
  }

  Framebuffer* Light::GetShadowMapFramebuffer()
  {
    return m_depthFramebuffer;
  }

  RenderTarget* Light::GetShadowMapRenderTarget()
  {
    return m_shadowRt;
  }

  MaterialPtr Light::GetShadowMaterial()
  {
    return m_shadowMapMaterial;
  }

  void Light::InitShadowMapDepthMaterial()
  {
    // Create shadow material
    ShaderPtr vert = GetShaderManager()->Create<Shader>(
        ShaderPath("orthogonalDepthVert.shader", true));
    ShaderPtr frag = GetShaderManager()->Create<Shader>(
        ShaderPath("orthogonalDepthFrag.shader", true));

    m_shadowMapMaterial                  = std::make_shared<Material>();
    m_shadowMapMaterial->m_vertexShader  = vert;
    m_shadowMapMaterial->m_fragmetShader = frag;
    m_shadowMapMaterial->Init();
  }

  DirectionalLight::DirectionalLight()
  {
    SetNormalBiasVal(0.1f);
    SetFixedBiasVal(0.0f);
    SetSlopedBiasVal(2.0f);
    AddComponent(new DirectionComponent(this));
  }

  void Light::ReInitShadowMap()
  {
    UnInitShadowMap();
    InitShadowMap();
  }

  DirectionalLight::~DirectionalLight()
  {
  }

  EntityType DirectionalLight::GetType() const
  {
    return EntityType::Entity_DirectionalLight;
  }

  // Returns 8 sized array
  Vec3Array DirectionalLight::GetShadowFrustumCorners()
  {
    Vec3Array frustum = {Vec3(-1.0f, -1.0f, -1.0f),
                         Vec3(1.0f, -1.0f, -1.0f),
                         Vec3(1.0f, -1.0f, 1.0f),
                         Vec3(-1.0f, -1.0f, 1.0f),
                         Vec3(-1.0f, 1.0f, -1.0f),
                         Vec3(1.0f, 1.0f, -1.0f),
                         Vec3(1.0f, 1.0f, 1.0f),
                         Vec3(-1.0f, 1.0f, 1.0f)};

    const Mat4 inverseSpaceMatrix =
        glm::inverse(m_shadowMapCameraProjectionViewMatrix);
    for (int i = 0; i < 8; ++i)
    {
      const Vec4 t = inverseSpaceMatrix * Vec4(frustum[i], 1.0f);
      frustum[i]   = Vec3(t.x / t.w, t.y / t.w, t.z / t.w);
    }
    return frustum;
  }

  PointLight::PointLight()
  {
    SetNormalBiasVal(0.1f);
    SetFixedBiasVal(0.0f);
    SetSlopedBiasVal(2.0f);
    SetPCFSampleSizeVal(0.03f);
    ParamPCFSampleSize().m_hint = {false, true, 0.001f, 0.5f, 0.01f};
    PCFLevel_Define(1, "Light", 90, true, true, {false, true, 0, 2, 1});
    Radius_Define(
        3.0f, "Light", 90, true, true, {false, true, 0.1f, 100000.0f, 0.4f});
    ParamPCFKernelSize().m_exposed = false;
  }

  EntityType PointLight::GetType() const
  {
    return EntityType::Entity_PointLight;
  }

  void PointLight::InitShadowMap()
  {
    if (m_shadowMapInitialized && !m_shadowMapResolutionChanged)
    {
      return;
    }

    Vec2 res   = GetShadowResolutionVal();
    m_shadowRt = new RenderTarget((int) res.x,
                                  (int) res.y,
                                  {0,
                                   true,
                                   GraphicTypes::TargetCubeMap,
                                   GraphicTypes::UVClampToBorder,
                                   GraphicTypes::UVClampToBorder,
                                   GraphicTypes::UVClampToBorder,
                                   GraphicTypes::SampleNearest,
                                   GraphicTypes::SampleNearest,
                                   GraphicTypes::FormatDepthComponent,
                                   GraphicTypes::FormatDepthComponent,
                                   GraphicTypes::TypeFloat,
                                   Vec4(1.0f)});
    m_shadowRt->Init();

    m_depthFramebuffer = new Framebuffer();
    m_depthFramebuffer->Init({(uint) res.x, (uint) res.y, 0, false, false});
    m_depthFramebuffer->SetAttachment(Framebuffer::Attachment::DepthAttachment,
                                      m_shadowRt);

    InitShadowMapDepthMaterial();
    m_shadowMapInitialized = true;
  }

  void PointLight::InitShadowMapDepthMaterial()
  {
    // Create shadow material
    ShaderPtr vert = GetShaderManager()->Create<Shader>(
        ShaderPath("perspectiveDepthVert.shader", true));
    ShaderPtr frag = GetShaderManager()->Create<Shader>(
        ShaderPath("perspectiveDepthFrag.shader", true));

    m_shadowMapMaterial                  = std::make_shared<Material>();
    m_shadowMapMaterial->m_vertexShader  = vert;
    m_shadowMapMaterial->m_fragmetShader = frag;
    m_shadowMapMaterial->Init();
  }

  SpotLight::SpotLight()
  {
    SetNormalBiasVal(0.2f);
    SetFixedBiasVal(10.0f);
    SetSlopedBiasVal(5.0f);
    Radius_Define(
        10.0f, "Light", 90, true, true, {false, true, 0.1f, 100000.0f, 0.4f});
    OuterAngle_Define(
        35.0f, "Light", 90, true, true, {false, true, 0.5f, 89.8f, 1.0f});
    InnerAngle_Define(
        30.0f, "Light", 90, true, true, {false, true, 0.5f, 89.8f, 1.0f});

    AddComponent(new DirectionComponent(this));
  }

  EntityType SpotLight::GetType() const
  {
    return EntityType::Entity_SpotLight;
  }

  void SpotLight::InitShadowMapDepthMaterial()
  {
    // Create shadow material
    ShaderPtr vert = GetShaderManager()->Create<Shader>(
        ShaderPath("perspectiveDepthVert.shader", true));
    ShaderPtr frag = GetShaderManager()->Create<Shader>(
        ShaderPath("perspectiveDepthFrag.shader", true));

    m_shadowMapMaterial                  = std::make_shared<Material>();
    m_shadowMapMaterial->m_vertexShader  = vert;
    m_shadowMapMaterial->m_fragmetShader = frag;
    m_shadowMapMaterial->Init();
  }
} // namespace ToolKit
