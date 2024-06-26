/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Light.h"

#include "AABBOverrideComponent.h"
#include "BVH.h"
#include "Camera.h"
#include "Component.h"
#include "DirectionComponent.h"
#include "EngineSettings.h"
#include "Material.h"
#include "MathUtil.h"
#include "Mesh.h"
#include "Pass.h"
#include "RenderSystem.h"
#include "Renderer.h"
#include "Scene.h"
#include "Shader.h"
#include "TKProfiler.h"
#include "ToolKit.h"

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

  void Light::NativeConstruct() { Super::NativeConstruct(); }

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

    ParamColor().m_onValueChangedFn.clear();
    ParamColor().m_onValueChangedFn.push_back([this](Value& oldVal, Value& newVal) -> void
                                              { m_invalidatedForLightCache = true; });

    ParamIntensity().m_onValueChangedFn.clear();
    ParamIntensity().m_onValueChangedFn.push_back([this](Value& oldVal, Value& newVal) -> void
                                                  { m_invalidatedForLightCache = true; });

    ParamCastShadow().m_onValueChangedFn.clear();
    ParamCastShadow().m_onValueChangedFn.push_back([this](Value& oldVal, Value& newVal) -> void
                                                   { m_invalidatedForLightCache = true; });

    ParamShadowRes().m_onValueChangedFn.clear();
    ParamShadowRes().m_onValueChangedFn.push_back(
        [this](Value& oldVal, Value& newVal) -> void
        {
          const float val = std::get<float>(newVal);

          if (val > 0.0f && val < RHIConstants::ShadowAtlasTextureSize + 0.1f)
          {
            if (GetCastShadowVal())
            {
              m_shadowResolutionUpdated  = true;
              m_invalidatedForLightCache = true;
            }
          }
          else
          {
            newVal = oldVal;
          }
        });

    ParamPCFSamples().m_onValueChangedFn.clear();
    ParamPCFSamples().m_onValueChangedFn.push_back([this](Value& oldVal, Value& newVal) -> void
                                                   { m_invalidatedForLightCache = true; });

    ParamPCFRadius().m_onValueChangedFn.clear();
    ParamPCFRadius().m_onValueChangedFn.push_back([this](Value& oldVal, Value& newVal) -> void
                                                  { m_invalidatedForLightCache = true; });

    ParamShadowBias().m_onValueChangedFn.clear();
    ParamShadowBias().m_onValueChangedFn.push_back([this](Value& oldVal, Value& newVal) -> void
                                                   { m_invalidatedForLightCache = true; });

    ParamBleedingReduction().m_onValueChangedFn.clear();
    ParamBleedingReduction().m_onValueChangedFn.push_back([this](Value& oldVal, Value& newVal) -> void
                                                          { m_invalidatedForLightCache = true; });
  }

  MaterialPtr Light::GetShadowMaterial() { return m_shadowMapMaterial; }

  void Light::UpdateShadowCamera()
  {
    m_shadowMapCameraProjectionViewMatrix = m_shadowCamera->GetProjectViewMatrix();
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
    m_shadowMapMaterial->m_vertexShader                  = vert;
    m_shadowMapMaterial->m_fragmentShader                = frag;
    m_shadowMapMaterial->GetRenderState()->blendFunction = BlendFunction::NONE;
    m_shadowMapMaterial->Init();
  }

  void Light::InvalidateSpatialCaches()
  {
    Super::InvalidateSpatialCaches();
    m_invalidatedForLightCache = true;
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

  void Light::UpdateLocalBoundingBox()
  {
    if (m_volumeMesh != nullptr)
    {
      m_localBoundingBoxCache = m_volumeMesh->m_boundingBox;
    }
    else
    {
      m_localBoundingBoxCache = infinitesimalBox;
    }
  }

  // DirectionalLight
  //////////////////////////////////////////

  TKDefineClass(DirectionalLight, Light);

  DirectionalLight::DirectionalLight()
  {
    m_shadowCamera->SetOrthographicVal(true);
    for (int i = 0; i < RHIConstants::MaxCascadeCount; ++i)
    {
      CameraPtr cam = MakeNewPtr<Camera>();
      cam->SetOrthographicVal(true);
      cam->SetOrthographicScaleVal(1.0f);
      cam->InvalidateSpatialCaches();
      m_cascadeShadowCameras.push_back(cam);
    }
    m_shadowMapCascadeCameraProjectionViewMatrices.resize(RHIConstants::MaxCascadeCount);
  }

  DirectionalLight::~DirectionalLight()
  {
    m_cascadeShadowCameras.clear();
    m_shadowMapCascadeCameraProjectionViewMatrices.clear();
  }

  void DirectionalLight::NativeConstruct()
  {
    Super::NativeConstruct();
    AddComponent<DirectionComponent>();
  }

  void DirectionalLight::UpdateShadowFrustum(CameraPtr cameraView, ScenePtr scene)
  {
    int cascades               = GetEngineSettings().Graphics.cascadeCount;
    float* cascadeDists        = GetEngineSettings().Graphics.cascadeDistances;

    const float lastCameraNear = cameraView->Near();
    const float lastCameraFar  = cameraView->Far();

    for (int i = 0; i < cascades; ++i)
    {
      float near = cascadeDists[i];
      float far;
      if (i == cascades - 1)
      {
        far = GetEngineSettings().PostProcessing.ShadowDistance;
      }
      else
      {
        far = cascadeDists[i + 1];
      }

      cameraView->SetNearClipVal(near);
      cameraView->SetFarClipVal(far);

      FitViewFrustumIntoLightFrustum(m_cascadeShadowCameras[i],
                                     cameraView,
                                     near,
                                     far);
    }

    cameraView->SetNearClipVal(lastCameraNear);
    cameraView->SetFarClipVal(lastCameraFar);

    UpdateShadowCamera();

    m_invalidatedForLightCache = true;
  }

  void DirectionalLight::UpdateShadowCamera()
  {
    for (int i = 0; i < GetEngineSettings().Graphics.cascadeCount; ++i)
    {
      m_shadowMapCascadeCameraProjectionViewMatrices[i] = m_cascadeShadowCameras[i]->GetProjectViewMatrix();
    }
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

  void DirectionalLight::FitViewFrustumIntoLightFrustum(CameraPtr lightCamera,
                                                        CameraPtr viewCamera,
                                                        float near,
                                                        float far)
  {
    const Vec3Array frustum = viewCamera->ExtractFrustumCorner();

    Vec3 center             = ZERO;
    for (int i = 0; i < 8; ++i)
    {
      center += frustum[i];
    }
    center /= 8.0f;

    lightCamera->m_node->SetOrientation(m_node->GetOrientation());
    lightCamera->m_node->SetTranslation(center);

    const Mat4 lightView = lightCamera->GetViewMatrix();

    // Calculate tight shadow volume.
    BoundingBox tightShadowVolume;
    for (int i = 0; i < 8; i++)
    {
      const Vec4 vertex = lightView * Vec4(frustum[i], 1.0f);
      tightShadowVolume.UpdateBoundary(vertex);
    }

    const float tightFar = tightShadowVolume.max.z - tightShadowVolume.min.z;

    lightCamera->SetLens(tightShadowVolume.min.x,
                         tightShadowVolume.max.x,
                         tightShadowVolume.min.y,
                         tightShadowVolume.max.y,
                         -0.5f * tightFar,
                         0.5f * tightFar);

    /*
    const float tightWidth  = tightShadowVolume.max.x - tightShadowVolume.min.x;
    const float tightHeight = tightShadowVolume.max.y - tightShadowVolume.min.y;
    lightCamera->SetLens(-tightWidth * 0.5f,
                         tightWidth * 0.5f,
                         -tightHeight * 0.5f,
                         tightHeight * 0.5f,
                         -tightFar * 0.5f,
                         tightFar * 0.5f);
    */
  }

  /*
  void DirectionalLight::FitViewFrustumIntoLightFrustum(CameraPtr lightCamera,
                                                        CameraPtr viewCamera,
                                                        const BoundingBox& shadowVolume,
                                                        float near,
                                                        float far)
  {
    // Set far for view frustum
    float lastCameraNear = viewCamera->GetNearClipVal();
    float lastCameraFar  = viewCamera->GetFarClipVal();
    viewCamera->SetNearClipVal(near);
    viewCamera->SetFarClipVal(far);

    Vec3Array frustum = viewCamera->ExtractFrustumCorner();
    viewCamera->SetNearClipVal(lastCameraNear);
    viewCamera->SetFarClipVal(lastCameraFar);

    const float worldUnitPerTexel = far * 25.0f / RHIConstants::ShadowAtlasTextureSize;
    const Vec3 vWorldUnitPerTexel = Vec3(worldUnitPerTexel, worldUnitPerTexel, worldUnitPerTexel);

    Vec3 center                   = ZERO;
    for (int i = 0; i < 8; ++i)
    {
      frustum[i] /= vWorldUnitPerTexel;
      frustum[i]  = glm::floor(frustum[i]);
      frustum[i] *= vWorldUnitPerTexel;

      center     += frustum[i];
    }
    center /= 8.0f;

    lightCamera->m_node->SetOrientation(m_node->GetOrientation());
    lightCamera->m_node->SetTranslation(center);

    Mat4 lightView = lightCamera->GetViewMatrix();

    // Calculate tight shadow volume.
    BoundingBox tightShadowVolume;
    for (int i = 0; i < 8; i++)
    {
      Vec4 vertex = lightView * Vec4(frustum[i], 1.0f);
      tightShadowVolume.UpdateBoundary(vertex);
    }

    // Fit the frustum into the scene only for far
    float width       = shadowVolume.max.x - shadowVolume.min.x;
    float height      = shadowVolume.max.y - shadowVolume.min.y;
    float depth       = shadowVolume.max.z - shadowVolume.min.z;
    float maxDistance = glm::sqrt(width * width + height * height + depth * depth);

    float tightFar    = tightShadowVolume.max.z - tightShadowVolume.min.z;
    tightFar          = glm::min(tightFar, maxDistance);

    lightCamera->SetLens(tightShadowVolume.min.x,
                         tightShadowVolume.max.x,
                         tightShadowVolume.min.y,
                         tightShadowVolume.max.y,
                         -tightFar * 0.5f,
                         tightFar * 0.5f);
  }
  */
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
    m_shadowMapMaterial->m_vertexShader                  = vert;
    m_shadowMapMaterial->m_fragmentShader                = frag;
    m_shadowMapMaterial->GetRenderState()->blendFunction = BlendFunction::NONE;
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
    ParamPCFRadius().m_hint.increment = 0.02f;
    Radius_Define(3.0f, "Light", 90, true, true, {false, true, 0.1f, 100000.0f, 0.3f});
    ParamRadius().m_onValueChangedFn.clear();
    ParamRadius().m_onValueChangedFn.push_back(
        [this](Value& oldVal, Value& newVal) -> void
        {
          if (BVHPtr bvh = m_bvh.lock())
          {
            EntityPtr self = Self<PointLight>();
            bvh->UpdateEntity(self);
          }
          m_invalidatedForLightCache = true;
        });
  }

  void PointLight::UpdateLocalBoundingBox()
  {
    float radius            = GetRadiusVal();
    m_localBoundingBoxCache = BoundingBox(Vec3(radius), Vec3(radius));
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

    // Calculate frustum.
    m_frustumCache           = ExtractFrustum(m_shadowMapCameraProjectionViewMatrix, false);

    // Calculate bounding box for the frustum.
    Vec3Array frustumCorners = m_shadowCamera->ExtractFrustumCorner();
    m_boundingBoxCache       = BoundingBox();
    for (int i = 0; i < 8; i++)
    {
      m_boundingBoxCache.UpdateBoundary(frustumCorners[i]);
    }
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
    m_shadowMapMaterial->m_vertexShader                  = vert;
    m_shadowMapMaterial->m_fragmentShader                = frag;
    m_shadowMapMaterial->GetRenderState()->blendFunction = BlendFunction::NONE;
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
          if (BVHPtr bvh = m_bvh.lock())
          {
            EntityPtr self = Self<PointLight>();
            bvh->UpdateEntity(self);
          }
          m_invalidatedForLightCache = true;
        });

    ParamInnerAngle().m_onValueChangedFn.clear();
    ParamInnerAngle().m_onValueChangedFn.push_back([this](Value& oldVal, Value& newVal) -> void
                                                   { m_invalidatedForLightCache = true; });

    ParamOuterAngle().m_onValueChangedFn.clear();
    ParamOuterAngle().m_onValueChangedFn.push_back(
        [this](Value& oldVal, Value& newVal) -> void
        {
          const float outerAngle = std::get<float>(newVal);
          MeshGenerator::GenerateConeMesh(m_volumeMesh, GetRadiusVal(), 32, outerAngle);
          if (BVHPtr bvh = m_bvh.lock())
          {
            EntityPtr self = Self<PointLight>();
            bvh->UpdateEntity(self);
          }
          m_invalidatedForLightCache = true;
        });
  }
} // namespace ToolKit
