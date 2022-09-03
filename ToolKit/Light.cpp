
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
    Intensity_Define
    (
      1.0f,
      "Light",
      90,
      true,
      true,
      {
        false,
        true,
        0.0f,
        100000.0f,
        0.1f
      }
    );
    CastShadow_Define(false, "Light", 90, true, true);
    ShadowBias_Define
    (
      0.4f,
      "Light",
      90,
      true,
      true,
      {
        false,
        true,
        0.0f,
        100000.0f,
        0.01f
      }
    );
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
    PCFSampleSize_Define
    (
      7.0f,
      "Light",
      90,
      true,
      true,
      {
        false,
        true,
        0.1f,
        100.0f,
        0.1f
      }
    );
    PCFKernelSize_Define
    (
      5,
      "Light",
      90,
      true,
      true,
      {
        false,
        true,
        1,
        20,
        1
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
        GraphicTypes::Target2D,
        GraphicTypes::UVClampToBorder,
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
    ShadowFrustumSize_Define
    (
      Vec4(-20.0f, 20.0f, -20.0f, 20.0f),
      "Light",
      90,
      true,
      true,
      {
        false,
        true,
        -1000.0f,
        1000.0f,
        1.0f
      }
    );
    ShadowFrustumNearAndFar_Define
    (
      Vec2(0.01f, 100.0f),
      "Light",
      90,
      true,
      true,
      {
        false,
        true,
        0.01f,
        10000.0f,
        2.0f
      }
    );
    AddComponent(new DirectionComponent(this));
  }

  DirectionalLight::~DirectionalLight()
  {
  }

  EntityType DirectionalLight::GetType() const
  {
    return EntityType::Entity_DirectionalLight;
  }

  BoundingBox DirectionalLight::GetShadowMapCameraFrustumCorners()
  {
    BoundingBox box;
    Vec3 max = Vec3(1.0f, 1.0f, 1.0f);
    Vec3 min = Vec3(-1.0f, -1.0f, -1.0f);
    Vec4 frustumSize = GetShadowFrustumSizeVal();
    Vec2 frustumNearFar = GetShadowFrustumNearAndFarVal();
    Mat4 invProj = glm::inverse
    (
      glm::ortho
      (
        frustumSize.x,
        frustumSize.y,
        frustumSize.z,
        frustumSize.w,
        frustumNearFar.x,
        frustumNearFar.y
      )
    );
    Vec4 hmgMax = invProj * Vec4(max, 1.0f);
    Vec4 hmgMin = invProj * Vec4(min, 1.0f);
    max = Vec3(hmgMax) / hmgMax.w;
    min = Vec3(hmgMin) / hmgMin.w;
    box.max = max;
    box.min = min;
    return box;
  }

  PointLight::PointLight()
  {
    Radius_Define
    (
      3.0f,
      "Light",
      90,
      true,
      true,
      {
        false,
        true,
        0.1f,
        100000.0f,
        0.4f
      }
    );
    ParamPCFSampleSize().m_exposed = false;
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
        GraphicTypes::TargetCubeMap,
        GraphicTypes::UVClampToBorder,
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

  void PointLight::InitShadowMapDepthMaterial()
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

  SpotLight::SpotLight()
  {
    Radius_Define
    (
      10.0f,
      "Light",
      90,
      true,
      true,
      {
        false,
        true,
        0.1f,
        100000.0f,
        0.4f
      }
    );
    OuterAngle_Define
    (
      35.0f,
      "Light",
      90,
      true,
      true,
      {
        false,
        true,
        0.5f,
        89.8f,
        1.0f
      }
    );
    InnerAngle_Define
    (
      30.0f,
      "Light",
      90,
      true,
      true,
      {
        false,
        true,
        0.5f,
        89.8f,
        1.0f
      }
    );

    AddComponent(new DirectionComponent(this));
  }

  EntityType SpotLight::GetType() const
  {
    return EntityType::Entity_SpotLight;
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
