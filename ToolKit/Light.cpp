
#include "Light.h"

#include <string>
#include <memory>

#include "ToolKit.h"
#include "GL/glew.h"
#include "Component.h"
#include "DirectionComponent.h"

namespace ToolKit
{
  Light::Light()
  {
    Color_Define(Vec3(1.0f), "Light", 0, true, true, { true });
    Intensity_Define(1.0f, "Light", 90, true, true);
    CastShadow_Define(false, "Light", 90, true, true);
    ShadowBias_Define(0.4f, "Light", 90, true, true);
    ShadowResolution_Define
    (
      Vec2(1024.0f, 1024.0f),
      "Light",
      90,
      true,
      true,
      {
        false,
        true,
        32.0f,
        4096.0f,
        2.0f
      }
    );
  }

  Light::~Light()
  {
    UnInitShadowMap();
  }

  void Light::ParameterEventConstructor()
  {
    ParamShadowResolution().m_onValueChangedFn =
    [this](Value& oldVal, Value& newVal) -> void
    {
      UnInitShadowMap();
      InitShadowMap();
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
    ClearComponents();  // Read from file.
    Entity::DeSerialize(doc, parent);
    ParameterEventConstructor();
  }

  void Light::InitShadowMap()
  {
    if (m_shadowMapInitialized && !m_shadowMapResolutionChanged)
    {
      return;
    }

    // Store current framebuffer
    GLint lastFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &lastFBO);

    m_depthRenderTarget = new RenderTarget
    (
      static_cast<uint>(GetShadowResolutionVal().x),
      static_cast<uint>(GetShadowResolutionVal().y),
      {
        0,
        false,
        true,
        GraphicTypes::UVClampToBorder,
        GraphicTypes::UVClampToBorder,
        GraphicTypes::SampleNearest,
        GraphicTypes::SampleNearest,
        GraphicTypes::FormatDepthComponent,
        GraphicTypes::FormatDepthComponent,
        GraphicTypes::TypeFloat,
        GraphicTypes::DepthAttachment,
        Vec4(1.0f)
      }
    );
    m_depthRenderTarget->Init();
    glBindFramebuffer(GL_FRAMEBUFFER, lastFBO);

    InitShadowMapDepthMaterial();
    m_shadowMapInitialized = true;
  }

  void Light::UnInitShadowMap()
  {
    SafeDel(m_depthRenderTarget);
    m_shadowMapInitialized = false;
  }

  void Light::SetShadowMapResolution(uint width, uint height)
  {
    m_shadowMapWidth = width;
    m_shadowMapHeight = height;
    m_shadowMapResolutionChanged = true;
  }

  Vec2 Light::GetShadowMapResolution()
  {
    return Vec2(m_shadowMapWidth, m_shadowMapHeight);
  }

  RenderTarget* Light::GetShadowMapRenderTarget()
  {
    return m_depthRenderTarget;
  }

  MaterialPtr Light::GetShadowMaterial()
  {
    return m_shadowMapMaterial;
  }

  void Light::InitShadowMapDepthMaterial()
  {
    // Create shadow material
    ShaderPtr vert = GetShaderManager()->Create<Shader>
    (
      ShaderPath("orthogonalDepthVert.shader", true)
    );
    ShaderPtr frag = GetShaderManager()->Create<Shader>
    (
      ShaderPath("orthogonalDepthFrag.shader", true)
    );

    m_shadowMapMaterial = std::make_shared<Material>();
    m_shadowMapMaterial->m_vertexShader = vert;
    m_shadowMapMaterial->m_fragmetShader = frag;
    m_shadowMapMaterial->Init();
  }

  DirectionalLight::DirectionalLight()
  {
    AddComponent(new DirectionComponent(this));
  }

  DirectionalLight::~DirectionalLight()
  {
  }

  EntityType DirectionalLight::GetType() const
  {
    return EntityType::Entity_DirectionalLight;
  }

  PointLight::PointLight()
  {
    Radius_Define(3.0f, "Light", 90, true, true);
  }

  EntityType PointLight::GetType() const
  {
    return EntityType::Entity_PointLight;
  }

  SpotLight::SpotLight()
  {
    Radius_Define(10.0f, "Light", 90, true, true);
    OuterAngle_Define(35.0f, "Light", 90, true, true);
    InnerAngle_Define(30.0f, "Light", 90, true, true);

    AddComponent(new DirectionComponent(this));
  }

  EntityType SpotLight::GetType() const
  {
    return EntityType::Entity_SpotLight;
  }

  void SpotLight::InitShadowMap()
  {
    Light::InitShadowMap();
  }

  void SpotLight::InitShadowMapDepthMaterial()
  {
    // Create shadow material
    ShaderPtr vert = GetShaderManager()->Create<Shader>
    (
      ShaderPath("perspectiveDepthVert.shader", true)
    );
    ShaderPtr frag = GetShaderManager()->Create<Shader>
    (
      ShaderPath("perspectiveDepthFrag.shader", true)
    );

    m_shadowMapMaterial = std::make_shared<Material>();
    m_shadowMapMaterial->m_vertexShader = vert;
    m_shadowMapMaterial->m_fragmetShader = frag;
    m_shadowMapMaterial->Init();
  }
}  // namespace ToolKit
