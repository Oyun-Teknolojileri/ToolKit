
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
    m_shadowCamera = new Camera();

    Color_Define(Vec3(1.0f), "Light", 0, true, true, {true});
    Intensity_Define(
        1.0f, "Light", 90, true, true, {false, true, 0.0f, 100000.0f, 0.1f});
    CastShadow_Define(false, "Light", 90, true, true);
    ShadowRes_Define(
        1024.0f, "Light", 90, true, true, {false, true, 32.0f, 4096.0f, 2.0f});
    PCFSamples_Define(32, "Light", 90, true, true, {false, true, 0, 128, 1});
    PCFRadius_Define(
        0.01f, "Light", 90, true, true, {false, true, 0.0f, 5.0f, 0.0001f});
    ShadowThickness_Define(
        0.5f, "Light", 90, true, true, {false, true, 0.0f, 5.0f, 0.05f});
    LightBleedingReduction_Define(
        0.1f, "Light", 90, true, true, {false, true, 0.0f, 1.0f, 0.001f});
    ParameterEventConstructor();
  }

  Light::~Light()
  {
    SafeDel(m_shadowCamera);
  }

  void Light::ParameterEventConstructor()
  {
    ParamShadowRes().m_onValueChangedFn.clear();
    ParamShadowRes().m_onValueChangedFn.push_back(
        [this](Value& oldVal, Value& newVal) -> void {
          const float val = std::get<float>(newVal);

          if (val > -0.5f &&
              val < Renderer::m_rhiSettings::g_shadowAtlasTextureSize + 0.1f)
          {
            if (GetCastShadowVal())
            {
              m_shadowResolutionUpdated = true;
            }
          }
          else
          {
            newVal = oldVal;
          }
        });
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

  MaterialPtr Light::GetShadowMaterial()
  {
    return m_shadowMapMaterial;
  }

  void Light::UpdateShadowCamera()
  {
    Mat4 proj = m_shadowCamera->GetProjectionMatrix();
    Mat4 view = m_shadowCamera->GetViewMatrix();

    m_shadowMapCameraProjectionViewMatrix = proj * view;
    m_shadowMapCameraFar                  = m_shadowCamera->Far();
  }

  float Light::AffectDistance()
  {
    return 1000.0f;
  }

  void Light::InitShadowMapDepthMaterial()
  {
    // Create shadow material
    ShaderPtr vert = GetShaderManager()->Create<Shader>(
        ShaderPath("orthogonalDepthVert.shader", true));
    ShaderPtr frag = GetShaderManager()->Create<Shader>(
        ShaderPath("orthogonalDepthFrag.shader", true));

    m_shadowMapMaterial                   = std::make_shared<Material>();
    m_shadowMapMaterial->m_vertexShader   = vert;
    m_shadowMapMaterial->m_fragmentShader = frag;
    m_shadowMapMaterial->Init();
  }

  void Light::UpdateShadowCameraTransform()
  {
    m_shadowCamera->m_node->SetTransform(m_node->GetTransform());
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

  void DirectionalLight::UpdateShadowFrustum(const EntityRawPtrArray& entities)
  {
    FitEntitiesBBoxIntoShadowFrustum(m_shadowCamera, entities);
    UpdateShadowCamera();
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

  void DirectionalLight::FitEntitiesBBoxIntoShadowFrustum(
      Camera* lightCamera, const EntityRawPtrArray& entities)
  {
    // Calculate all scene's bounding box
    BoundingBox totalBBox;
    for (Entity* ntt : entities)
    {
      if (!(ntt->IsDrawable() && ntt->GetVisibleVal()))
      {
        continue;
      }

      if (!ntt->GetMeshComponent()->GetCastShadowVal())
      {
        continue;
      }

      BoundingBox bb = ntt->GetAABB(true);
      totalBBox.UpdateBoundary(bb.max);
      totalBBox.UpdateBoundary(bb.min);
    }
    Vec3 center = totalBBox.GetCenter();

    // Set light transformation
    lightCamera->m_node->SetTranslation(center);
    lightCamera->m_node->SetOrientation(m_node->GetOrientation());
    Mat4 lightView = lightCamera->GetViewMatrix();

    // Bounding box of the scene
    Vec3 min         = totalBBox.min;
    Vec3 max         = totalBBox.max;
    Vec4 vertices[8] = {Vec4(min.x, min.y, min.z, 1.0f),
                        Vec4(min.x, min.y, max.z, 1.0f),
                        Vec4(min.x, max.y, min.z, 1.0f),
                        Vec4(max.x, min.y, min.z, 1.0f),
                        Vec4(min.x, max.y, max.z, 1.0f),
                        Vec4(max.x, min.y, max.z, 1.0f),
                        Vec4(max.x, max.y, min.z, 1.0f),
                        Vec4(max.x, max.y, max.z, 1.0f)};

    // Calculate bounding box in light space
    BoundingBox shadowBBox;
    for (int i = 0; i < 8; ++i)
    {
      Vec4 vertex = lightView * vertices[i];
      shadowBBox.UpdateBoundary(vertex);
    }

    lightCamera->SetLens(shadowBBox.min.x,
                         shadowBBox.max.x,
                         shadowBBox.min.y,
                         shadowBBox.max.y,
                         shadowBBox.min.z,
                         shadowBBox.max.z);
  }

  void DirectionalLight::FitViewFrustumIntoLightFrustum(Camera* lightCamera,
                                                        Camera* viewCamera)
  {
    assert(false && "Experimental.");
    // Fit view frustum into light frustum
    Vec3 frustum[8] = {Vec3(-1.0f, -1.0f, -1.0f),
                       Vec3(1.0f, -1.0f, -1.0f),
                       Vec3(1.0f, -1.0f, 1.0f),
                       Vec3(-1.0f, -1.0f, 1.0f),
                       Vec3(-1.0f, 1.0f, -1.0f),
                       Vec3(1.0f, 1.0f, -1.0f),
                       Vec3(1.0f, 1.0f, 1.0f),
                       Vec3(-1.0f, 1.0f, 1.0f)};

    const Mat4 inverseViewProj = glm::inverse(
        viewCamera->GetProjectionMatrix() * viewCamera->GetViewMatrix());

    for (int i = 0; i < 8; ++i)
    {
      const Vec4 t = inverseViewProj * Vec4(frustum[i], 1.0f);
      frustum[i]   = Vec3(t.x / t.w, t.y / t.w, t.z / t.w);
    }

    Vec3 center = ZERO;
    for (int i = 0; i < 8; ++i)
    {
      center += frustum[i];
    }
    center /= 8.0f;

    TransformationSpace ts = TransformationSpace::TS_WORLD;
    lightCamera->m_node->SetTranslation(center, ts);
    lightCamera->m_node->SetOrientation(m_node->GetOrientation(ts), ts);
    const Mat4 lightView = lightCamera->GetViewMatrix();

    // Calculate bounding box
    BoundingBox shadowBBox;
    for (int i = 0; i < 8; ++i)
    {
      const Vec4 vertex = lightView * Vec4(frustum[i], 1.0f);
      shadowBBox.UpdateBoundary(vertex);
    }

    lightCamera->SetLens(shadowBBox.min.x,
                         shadowBBox.max.x,
                         shadowBBox.min.y,
                         shadowBBox.max.y,
                         shadowBBox.min.z,
                         shadowBBox.max.z);
  }

  PointLight::PointLight()
  {
    Radius_Define(
        3.0f, "Light", 90, true, true, {false, true, 0.1f, 100000.0f, 0.4f});
    ParamShadowThickness().m_exposed  = false;
    ParamPCFRadius().m_hint.increment = 0.02f;
  }

  PointLight::~PointLight()
  {
  }

  EntityType PointLight::GetType() const
  {
    return EntityType::Entity_PointLight;
  }

  void PointLight::UpdateShadowCamera()
  {
    m_shadowCamera->SetLens(
        glm::half_pi<float>(), 1.0f, 0.01f, AffectDistance());

    Light::UpdateShadowCamera();
    UpdateShadowCameraTransform();
  }

  float PointLight::AffectDistance()
  {
    return GetRadiusVal();
  }

  void PointLight::InitShadowMapDepthMaterial()
  {
    // Create shadow material
    ShaderPtr vert = GetShaderManager()->Create<Shader>(
        ShaderPath("perspectiveDepthVert.shader", true));
    ShaderPtr frag = GetShaderManager()->Create<Shader>(
        ShaderPath("perspectiveDepthFrag.shader", true));

    m_shadowMapMaterial                   = std::make_shared<Material>();
    m_shadowMapMaterial->m_vertexShader   = vert;
    m_shadowMapMaterial->m_fragmentShader = frag;
    m_shadowMapMaterial->Init();
  }

  SpotLight::SpotLight()
  {
    Radius_Define(
        10.0f, "Light", 90, true, true, {false, true, 0.1f, 100000.0f, 0.4f});
    OuterAngle_Define(
        35.0f, "Light", 90, true, true, {false, true, 0.5f, 179.8f, 1.0f});
    InnerAngle_Define(
        30.0f, "Light", 90, true, true, {false, true, 0.5f, 179.8f, 1.0f});

    AddComponent(new DirectionComponent(this));
  }

  SpotLight::~SpotLight()
  {
  }

  EntityType SpotLight::GetType() const
  {
    return EntityType::Entity_SpotLight;
  }

  void SpotLight::UpdateShadowCamera()
  {
    m_shadowCamera->SetLens(
        glm::radians(GetOuterAngleVal()), 1.0f, 0.01f, AffectDistance());

    Light::UpdateShadowCamera();
    UpdateShadowCameraTransform();
  }

  float SpotLight::AffectDistance()
  {
    return GetRadiusVal();
  }

  void SpotLight::InitShadowMapDepthMaterial()
  {
    // Create shadow material
    ShaderPtr vert = GetShaderManager()->Create<Shader>(
        ShaderPath("perspectiveDepthVert.shader", true));
    ShaderPtr frag = GetShaderManager()->Create<Shader>(
        ShaderPath("perspectiveDepthFrag.shader", true));

    m_shadowMapMaterial                   = std::make_shared<Material>();
    m_shadowMapMaterial->m_vertexShader   = vert;
    m_shadowMapMaterial->m_fragmentShader = frag;
    m_shadowMapMaterial->Init();
  }
} // namespace ToolKit
