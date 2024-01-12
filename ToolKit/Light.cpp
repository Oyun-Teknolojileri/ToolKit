/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Light.h"

#include "Camera.h"
#include "Component.h"
#include "DirectionComponent.h"
#include "EngineSettings.h"
#include "Material.h"
#include "MathUtil.h"
#include "Mesh.h"
#include "Pass.h"
#include "Renderer.h"
#include "Shader.h"
#include "TKProfiler.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

  // Light
  //////////////////////////////////////////

  TKDefineClass(Light, Entity);

  Light::Light()
  {
    m_shadowCamera = MakeNewPtr<Camera>();
    m_shadowCamera->SetOrthographicScaleVal(1.0f);
  }

  Light::~Light() {}

  void Light::ParameterConstructor()
  {
    Super::ParameterConstructor();

    Color_Define(Vec3(1.0f), "Light", 0, true, true, {true});
    Intensity_Define(1.0f, "Light", 90, true, true, {false, true, 0.0f, 100000.0f, 0.1f});
    CastShadow_Define(false, "Light", 90, true, true);
    ShadowRes_Define(512.0f, "Light", 90, true, true, {false, true, 32.0f, 4096.0f, 2.0f});
    PCFSamples_Define(32, "Light", 90, true, true, {false, true, 0, 128, 1});
    PCFRadius_Define(0.01f, "Light", 90, true, true, {false, true, 0.0f, 5.0f, 0.0001f});
    ShadowBias_Define(0.1f, "Light", 90, true, true, {false, true, 0.0f, 20000.0f, 0.01f});
    BleedingReduction_Define(0.1f, "Light", 90, true, true, {false, true, 0.0f, 1.0f, 0.001f});
  }

  void Light::ParameterEventConstructor()
  {
    Super::ParameterEventConstructor();

    ParamShadowRes().m_onValueChangedFn.clear();
    ParamShadowRes().m_onValueChangedFn.push_back(
        [this](Value& oldVal, Value& newVal) -> void
        {
          const float val = std::get<float>(newVal);

          if (val > -0.5f && val < Renderer::RHIConstants::ShadowAtlasTextureSize + 0.1f)
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

  MaterialPtr Light::GetShadowMaterial() { return m_shadowMapMaterial; }

  void Light::UpdateShadowCamera()
  {
    const Mat4& proj                      = m_shadowCamera->GetProjectionMatrix();
    Mat4 view                             = m_shadowCamera->GetViewMatrix();

    m_shadowMapCameraProjectionViewMatrix = proj * view;
    m_shadowMapCameraFar                  = m_shadowCamera->Far();
  }

  float Light::AffectDistance() { return 1000.0f; }

  void Light::InitShadowMapDepthMaterial()
  {
    // Create shadow material
    ShaderPtr vert = GetShaderManager()->Create<Shader>(ShaderPath("orthogonalDepthVert.shader", true));
    ShaderPtr frag = GetShaderManager()->Create<Shader>(ShaderPath("orthogonalDepthFrag.shader", true));

    if (m_shadowMapMaterial == nullptr)
    {
      m_shadowMapMaterial = MakeNewPtr<Material>();
    }
    m_shadowMapMaterial->UnInit();
    m_shadowMapMaterial->m_vertexShader   = vert;
    m_shadowMapMaterial->m_fragmentShader = frag;
    m_shadowMapMaterial->Init();
  }

  int Light::ComparableType()
  {
    if (IsA<DirectionalLight>())
    {
      return 0;
    }

    if (IsA<PointLight>())
    {
      return 1;
    }

    if (IsA<SpotLight>())
    {
      return 2;
    }

    return 3;
  }

  void Light::UpdateShadowCameraTransform()
  {
    Mat4 lightTs = m_node->GetTransform();
    m_shadowCamera->m_node->SetTransform(lightTs);
  }

  XmlNode* Light::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

  XmlNode* Light::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    ClearComponents(); // Read from file.
    XmlNode* nttNode = Super::DeSerializeImp(info, parent);

    ParameterEventConstructor();

    return nttNode->first_node(StaticClass()->Name.c_str());
  }

  // DirectionalLight
  //////////////////////////////////////////

  TKDefineClass(DirectionalLight, Light);

  DirectionalLight::DirectionalLight() {}

  DirectionalLight::~DirectionalLight() {}

  void DirectionalLight::NativeConstruct()
  {
    Super::NativeConstruct();
    AddComponent<DirectionComponent>();
  }

  void DirectionalLight::UpdateShadowFrustum(const RenderJobArray& jobs, const CameraPtr cameraView)
  {
    // FitEntitiesBBoxIntoShadowFrustum(m_shadowCamera, jobs);
    FitViewFrustumIntoLightFrustum(m_shadowCamera, cameraView);

    UpdateShadowCamera();
  }

  XmlNode* DirectionalLight::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

  void DirectionalLight::FitEntitiesBBoxIntoShadowFrustum(CameraPtr lightCamera, const RenderJobArray& jobs)
  {
    // Calculate all scene's bounding box
    BoundingBox totalBBox;
    for (const RenderJob& job : jobs)
    {
      if (!job.ShadowCaster)
      {
        continue;
      }

      totalBBox.UpdateBoundary(job.BoundingBox.max);
      totalBBox.UpdateBoundary(job.BoundingBox.min);
    }
    Vec3 center = totalBBox.GetCenter();

    // Set light transformation
    lightCamera->m_node->SetTranslation(center);
    lightCamera->m_node->SetOrientation(m_node->GetOrientation());
    Mat4 lightView   = lightCamera->GetViewMatrix();

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

  void DirectionalLight::FitViewFrustumIntoLightFrustum(CameraPtr lightCamera, CameraPtr viewCamera)
  {
    // Set far for view frustum
    float lastCameraFar  = viewCamera->GetFarClipVal();
    float shadowDistance = GetEngineSettings().Graphics.ShadowDistance;
    viewCamera->SetFarClipVal(shadowDistance);

    Vec3Array frustum = viewCamera->ExtractFrustumCorner();
    viewCamera->SetFarClipVal(lastCameraFar);

    Vec3 center = ZERO;
    for (int i = 0; i < 8; ++i)
    {
      center += frustum[i];
    }
    center /= 8.0f;

    lightCamera->m_node->SetOrientation(m_node->GetOrientation());
    Vec3 dir = lightCamera->Direction();

    // TODO: Hand crafted values. Must be calculated from scene boundary.
    lightCamera->m_node->SetTranslation(center + dir * -50.0f);

    const Mat4 lightView = lightCamera->GetViewMatrix();

    // Calculate bounding box
    BoundingBox shadowBBox;
    for (int i = 0; i < 8; ++i)
    {
      const Vec4 vertex = lightView * Vec4(frustum[i], 1.0f);
      shadowBBox.UpdateBoundary(vertex);
    }

    // TODO: Z Hand crafted values. Must be calculated from scene boundary.
    lightCamera->SetLens(shadowBBox.min.x, shadowBBox.max.x, shadowBBox.min.y, shadowBBox.max.y, -100.0f, 100.0f);
  }

  // PointLight
  //////////////////////////////////////////

  TKDefineClass(PointLight, Light);

  PointLight::PointLight() {}

  PointLight::~PointLight() {}

  void PointLight::UpdateShadowCamera()
  {
    m_shadowCamera->SetLens(glm::half_pi<float>(), 1.0f, 0.01f, AffectDistance());

    Light::UpdateShadowCamera();

    UpdateShadowCameraTransform();

    m_boundingSphereCache = {m_node->GetTranslation(), GetRadiusVal()};
  }

  float PointLight::AffectDistance() { return GetRadiusVal(); }

  void PointLight::InitShadowMapDepthMaterial()
  {
    // Create shadow material
    ShaderPtr vert = GetShaderManager()->Create<Shader>(ShaderPath("perspectiveDepthVert.shader", true));
    ShaderPtr frag = GetShaderManager()->Create<Shader>(ShaderPath("perspectiveDepthFrag.shader", true));

    if (m_shadowMapMaterial == nullptr)
    {
      m_shadowMapMaterial = MakeNewPtr<Material>();
    }
    m_shadowMapMaterial->UnInit();
    m_shadowMapMaterial->m_vertexShader   = vert;
    m_shadowMapMaterial->m_fragmentShader = frag;
    m_shadowMapMaterial->Init();
  }

  XmlNode* PointLight::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

  void PointLight::ParameterConstructor()
  {
    Super::ParameterConstructor();
    Radius_Define(3.0f, "Light", 90, true, true, {false, true, 0.1f, 100000.0f, 0.3f});
    ParamPCFRadius().m_hint.increment = 0.02f;
  }

  // SpotLight
  //////////////////////////////////////////

  TKDefineClass(SpotLight, Light);

  SpotLight::SpotLight() {}

  SpotLight::~SpotLight() {}

  void SpotLight::UpdateShadowCamera()
  {
    m_shadowCamera->SetLens(glm::radians(GetOuterAngleVal()), 1.0f, 0.01f, AffectDistance());

    Light::UpdateShadowCamera();

    UpdateShadowCameraTransform();

    m_frustumCache = ExtractFrustum(m_shadowMapCameraProjectionViewMatrix, false);
  }

  float SpotLight::AffectDistance() { return GetRadiusVal(); }

  void SpotLight::InitShadowMapDepthMaterial()
  {
    // Create shadow material
    ShaderPtr vert = GetShaderManager()->Create<Shader>(ShaderPath("perspectiveDepthVert.shader", true));
    ShaderPtr frag = GetShaderManager()->Create<Shader>(ShaderPath("perspectiveDepthFrag.shader", true));

    if (m_shadowMapMaterial == nullptr)
    {
      m_shadowMapMaterial = MakeNewPtr<Material>();
    }
    m_shadowMapMaterial->UnInit();
    m_shadowMapMaterial->m_vertexShader   = vert;
    m_shadowMapMaterial->m_fragmentShader = frag;
    m_shadowMapMaterial->Init();
  }

  XmlNode* SpotLight::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);
    return node;
  }

  XmlNode* SpotLight::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    XmlNode* node = Super::DeSerializeImp(info, parent);
    MeshGenerator::GenerateConeMesh(m_volumeMesh, GetRadiusVal(), 32, GetOuterAngleVal());
    return node;
  }

  void SpotLight::NativeConstruct()
  {
    Super::NativeConstruct();

    AddComponent<DirectionComponent>();
    m_volumeMesh = MakeNewPtr<Mesh>();

    MeshGenerator::GenerateConeMesh(m_volumeMesh, GetRadiusVal(), 32, GetOuterAngleVal());
  }

  void SpotLight::ParameterConstructor()
  {
    Super::ParameterConstructor();

    Radius_Define(10.0f, "Light", 90, true, true, {false, true, 0.1f, 100000.0f, 0.5f});
    OuterAngle_Define(35.0f, "Light", 90, true, true, {false, true, 0.5f, 179.8f, 1.0f});
    InnerAngle_Define(30.0f, "Light", 90, true, true, {false, true, 0.5f, 179.8f, 1.0f});

    ParamRadius().m_onValueChangedFn.clear();
    ParamRadius().m_onValueChangedFn.push_back(
        [this](Value& oldVal, Value& newVal) -> void
        {
          const float radius = std::get<float>(newVal);
          MeshGenerator::GenerateConeMesh(m_volumeMesh, radius, 32, GetOuterAngleVal());
        });

    ParamOuterAngle().m_onValueChangedFn.clear();
    ParamOuterAngle().m_onValueChangedFn.push_back(
        [this](Value& oldVal, Value& newVal) -> void
        {
          const float outerAngle = std::get<float>(newVal);
          MeshGenerator::GenerateConeMesh(m_volumeMesh, GetRadiusVal(), 32, outerAngle);
        });
  }
} // namespace ToolKit
