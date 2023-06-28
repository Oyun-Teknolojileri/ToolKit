/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Light.h"

#include "Component.h"
#include "DirectionComponent.h"
#include "ToolKit.h"

#include <memory>
#include <string>

#include "DebugNew.h"

namespace ToolKit
{

  // Light
  //////////////////////////////////////////

  TKDefineClass(Light, Entity);

  Light::Light()
  {
    m_shadowCamera = new Camera();
    m_shadowCamera->SetOrthographicScaleVal(1.0f);

    Color_Define(Vec3(1.0f), "Light", 0, true, true, {true});
    Intensity_Define(1.0f, "Light", 90, true, true, {false, true, 0.0f, 100000.0f, 0.1f});
    CastShadow_Define(false, "Light", 90, true, true);
    ShadowRes_Define(1024.0f, "Light", 90, true, true, {false, true, 32.0f, 4096.0f, 2.0f});
    PCFSamples_Define(32, "Light", 90, true, true, {false, true, 0, 128, 1});
    PCFRadius_Define(0.01f, "Light", 90, true, true, {false, true, 0.0f, 5.0f, 0.0001f});
    ShadowBias_Define(0.1f, "Light", 90, true, true, {false, true, 0.0f, 20000.0f, 0.01f});
    LightBleedingReduction_Define(0.1f, "Light", 90, true, true, {false, true, 0.0f, 1.0f, 0.001f});

    ParameterEventConstructor();
  }

  Light::~Light() { SafeDel(m_shadowCamera); }

  void Light::ParameterEventConstructor()
  {
    ParamShadowRes().m_onValueChangedFn.clear();
    ParamShadowRes().m_onValueChangedFn.push_back(
        [this](Value& oldVal, Value& newVal) -> void
        {
          const float val = std::get<float>(newVal);

          if (val > -0.5f && val < Renderer::m_rhiSettings::g_shadowAtlasTextureSize + 0.1f)
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

  EntityType Light::GetType() const { return EntityType::Entity_Light; }

  void Light::DeSerializeImp(XmlDocument* doc, XmlNode* parent)
  {
    ClearComponents(); // Read from file.
    Entity::DeSerializeImp(doc, parent);
    ParameterEventConstructor();
  }

  MaterialPtr Light::GetShadowMaterial() { return m_shadowMapMaterial; }

  void Light::UpdateShadowCamera()
  {
    Mat4 proj                             = m_shadowCamera->GetProjectionMatrix();
    Mat4 view                             = m_shadowCamera->GetViewMatrix();

    m_shadowMapCameraProjectionViewMatrix = proj * view;
    m_shadowMapCameraFar                  = m_shadowCamera->Far();
  }

  float Light::AffectDistance() { return 1000.0f; }

  void Light::InitShadowMapDepthMaterial()
  {
    // Create shadow material
    ShaderPtr vert      = GetShaderManager()->Create<Shader>(ShaderPath("orthogonalDepthVert.shader", true));
    ShaderPtr frag      = GetShaderManager()->Create<Shader>(ShaderPath("orthogonalDepthFrag.shader", true));

    m_shadowMapMaterial = std::make_shared<Material>();
    m_shadowMapMaterial->m_vertexShader   = vert;
    m_shadowMapMaterial->m_fragmentShader = frag;
    m_shadowMapMaterial->Init();
  }

  void Light::UpdateShadowCameraTransform() { m_shadowCamera->m_node->SetTransform(m_node->GetTransform()); }

  XmlNode* Light::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

  // DirectionalLight
  //////////////////////////////////////////

  TKDefineClass(DirectionalLight, Light);

  DirectionalLight::DirectionalLight() { AddComponent(new DirectionComponent(this)); }

  DirectionalLight::~DirectionalLight() {}

  EntityType DirectionalLight::GetType() const { return EntityType::Entity_DirectionalLight; }

  void DirectionalLight::UpdateShadowFrustum(const RenderJobArray& jobs)
  {
    FitEntitiesBBoxIntoShadowFrustum(m_shadowCamera, jobs);
    UpdateShadowCamera();
  }

  // Returns 8 sized array
  Vec3Array DirectionalLight::GetShadowFrustumCorners()
  {
    Vec3Array frustum             = {Vec3(-1.0f, -1.0f, -1.0f),
                                     Vec3(1.0f, -1.0f, -1.0f),
                                     Vec3(1.0f, -1.0f, 1.0f),
                                     Vec3(-1.0f, -1.0f, 1.0f),
                                     Vec3(-1.0f, 1.0f, -1.0f),
                                     Vec3(1.0f, 1.0f, -1.0f),
                                     Vec3(1.0f, 1.0f, 1.0f),
                                     Vec3(-1.0f, 1.0f, 1.0f)};

    const Mat4 inverseSpaceMatrix = glm::inverse(m_shadowMapCameraProjectionViewMatrix);
    for (int i = 0; i < 8; ++i)
    {
      const Vec4 t = inverseSpaceMatrix * Vec4(frustum[i], 1.0f);
      frustum[i]   = Vec3(t.x / t.w, t.y / t.w, t.z / t.w);
    }
    return frustum;
  }

  XmlNode* DirectionalLight::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

  void DirectionalLight::FitEntitiesBBoxIntoShadowFrustum(Camera* lightCamera, const RenderJobArray& jobs)
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

  void DirectionalLight::FitViewFrustumIntoLightFrustum(Camera* lightCamera, Camera* viewCamera)
  {
    assert(false && "Experimental.");
    // Fit view frustum into light frustum
    Vec3 frustum[8]            = {Vec3(-1.0f, -1.0f, -1.0f),
                                  Vec3(1.0f, -1.0f, -1.0f),
                                  Vec3(1.0f, -1.0f, 1.0f),
                                  Vec3(-1.0f, -1.0f, 1.0f),
                                  Vec3(-1.0f, 1.0f, -1.0f),
                                  Vec3(1.0f, 1.0f, -1.0f),
                                  Vec3(1.0f, 1.0f, 1.0f),
                                  Vec3(-1.0f, 1.0f, 1.0f)};

    const Mat4 inverseViewProj = glm::inverse(viewCamera->GetProjectionMatrix() * viewCamera->GetViewMatrix());

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
    center                 /= 8.0f;

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

  // PointLight
  //////////////////////////////////////////

  TKDefineClass(PointLight, Light);

  PointLight::PointLight()
  {
    Radius_Define(3.0f, "Light", 90, true, true, {false, true, 0.1f, 100000.0f, 0.4f});
    ParamPCFRadius().m_hint.increment = 0.02f;
  }

  PointLight::~PointLight() {}

  EntityType PointLight::GetType() const { return EntityType::Entity_PointLight; }

  void PointLight::UpdateShadowCamera()
  {
    m_shadowCamera->SetLens(glm::half_pi<float>(), 1.0f, 0.01f, AffectDistance());

    Light::UpdateShadowCamera();
    UpdateShadowCameraTransform();
  }

  float PointLight::AffectDistance() { return GetRadiusVal(); }

  void PointLight::InitShadowMapDepthMaterial()
  {
    // Create shadow material
    ShaderPtr vert      = GetShaderManager()->Create<Shader>(ShaderPath("perspectiveDepthVert.shader", true));
    ShaderPtr frag      = GetShaderManager()->Create<Shader>(ShaderPath("perspectiveDepthFrag.shader", true));

    m_shadowMapMaterial = std::make_shared<Material>();
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

  // SpotLight
  //////////////////////////////////////////

  TKDefineClass(SpotLight, Light);

  SpotLight::SpotLight()
  {
    Radius_Define(10.0f, "Light", 90, true, true, {false, true, 0.1f, 100000.0f, 0.4f});
    OuterAngle_Define(35.0f, "Light", 90, true, true, {false, true, 0.5f, 179.8f, 1.0f});
    InnerAngle_Define(30.0f, "Light", 90, true, true, {false, true, 0.5f, 179.8f, 1.0f});

    AddComponent(new DirectionComponent(this));
  }

  SpotLight::~SpotLight() {}

  EntityType SpotLight::GetType() const { return EntityType::Entity_SpotLight; }

  void SpotLight::UpdateShadowCamera()
  {
    m_shadowCamera->SetLens(glm::radians(GetOuterAngleVal()), 1.0f, 0.01f, AffectDistance());

    Light::UpdateShadowCamera();
    UpdateShadowCameraTransform();

    n_frustumCache = ExtractFrustum(m_shadowMapCameraProjectionViewMatrix, false);
  }

  float SpotLight::AffectDistance() { return GetRadiusVal(); }

  void SpotLight::InitShadowMapDepthMaterial()
  {
    // Create shadow material
    ShaderPtr vert      = GetShaderManager()->Create<Shader>(ShaderPath("perspectiveDepthVert.shader", true));
    ShaderPtr frag      = GetShaderManager()->Create<Shader>(ShaderPath("perspectiveDepthFrag.shader", true));

    m_shadowMapMaterial = std::make_shared<Material>();
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
} // namespace ToolKit
