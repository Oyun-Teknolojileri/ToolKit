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

#include "AdditiveLightingPass.h"

#include "DataTexture.h"
#include "DirectionComponent.h"
#include "Material.h"
#include "MathUtil.h"
#include "Mesh.h"
#include "Shader.h"

namespace ToolKit
{
  using FAttachment = Framebuffer::Attachment;

  AdditiveLightingPass::AdditiveLightingPass()
  {
    m_fullQuadPass                   = std::make_shared<FullQuadPass>();
    m_lightingFrameBuffer            = std::make_shared<Framebuffer>();
    m_lightingRt                     = std::make_shared<RenderTarget>();
    m_lightingShader                 = GetShaderManager()->Create<Shader>(ShaderPath("additiveLighting.shader", true));
    m_mergeShader                    = GetShaderManager()->Create<Shader>(ShaderPath("lightMerge.shader", true));
    m_meshMaterial                   = std::make_shared<Material>();

    m_meshMaterial->m_fragmentShader = m_lightingShader;
    m_meshMaterial->m_vertexShader   = GetShaderManager()->Create<Shader>(ShaderPath("additiveVertex.shader", true));
    m_meshMaterial->Init();
    RenderState* renderState   = m_meshMaterial->GetRenderState();
    renderState->blendFunction = BlendFunction::ONE_TO_ONE;
    m_sphereEntity             = MakeNewPtr<Sphere>();
    m_sphereEntity->SetNumRingVal(16);
    m_sphereEntity->SetNumSegVal(16);
  }

  AdditiveLightingPass::~AdditiveLightingPass() {}

  void AdditiveLightingPass::Init(const LightingPassParams& params) {}

  void AdditiveLightingPass::PreRender()
  {
    Pass::PreRender();

    int width  = m_params.MainFramebuffer->GetAttachment(FAttachment::ColorAttachment0)->m_width;
    int height = m_params.MainFramebuffer->GetAttachment(FAttachment::ColorAttachment0)->m_height;

    m_lightingFrameBuffer->Init({(uint) width, (uint) height, false, false});
    m_lightingFrameBuffer->ReconstructIfNeeded((uint) width, (uint) height);

    RenderTargetSettigs oneChannelSet = {};
    oneChannelSet.WarpS               = GraphicTypes::UVClampToEdge;
    oneChannelSet.WarpT               = GraphicTypes::UVClampToEdge;
    oneChannelSet.InternalFormat      = GraphicTypes::FormatRGB16F;
    oneChannelSet.Format              = GraphicTypes::FormatRGB;
    oneChannelSet.Type                = GraphicTypes::TypeFloat;

    m_lightingRt->m_settings          = oneChannelSet;
    m_lightingRt->ReconstructIfNeeded((uint) width, (uint) height);
    m_lightingFrameBuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0, m_lightingRt);

    Renderer* renderer          = GetRenderer();
    // Set gbuffer
    // 9: Position, 10: Normal, 11: Color, 14: metallic-roughness,
    FramebufferPtr gFrameBuffer = m_params.GBufferFramebuffer;

    renderer->SetTexture(9, gFrameBuffer->GetAttachment(FAttachment::ColorAttachment0)->m_textureId);
    renderer->SetTexture(10, gFrameBuffer->GetAttachment(FAttachment::ColorAttachment1)->m_textureId);
    renderer->SetTexture(11, gFrameBuffer->GetAttachment(FAttachment::ColorAttachment2)->m_textureId);
    renderer->SetTexture(14, gFrameBuffer->GetAttachment(FAttachment::ColorAttachment5)->m_textureId);

    m_lightingShader->SetShaderParameter("aoEnabled", ParameterVariant(m_params.AOTexture != nullptr));
    renderer->SetTexture(5, m_params.AOTexture ? m_params.AOTexture->m_textureId : 0);
  }

  void AdditiveLightingPass::SetLightUniforms(LightPtr light, int lightType)
  {
    Vec3 pos = light->m_node->GetTranslation();
    m_lightingShader->SetShaderParameter("lightType", ParameterVariant(lightType));
    m_lightingShader->SetShaderParameter("lightPos", ParameterVariant(pos));
    m_lightingShader->SetShaderParameter("lightIntensity", ParameterVariant(light->GetIntensityVal()));
    m_lightingShader->SetShaderParameter("lightColor", ParameterVariant(light->GetColorVal()));

    Vec3 dir;
    switch (lightType)
    {
    case 0: // is directional
    case 3:
      dir = static_cast<DirectionalLight*>(light.get())->GetComponent<DirectionComponent>()->GetDirection();
      break;
    case 1: // is point
    case 4:
    {
      PointLight* pointLight = static_cast<PointLight*>(light.get());
      m_lightingShader->SetShaderParameter("lightRadius", ParameterVariant(pointLight->GetRadiusVal()));
    }
    break;
    case 2: // is spot
    case 5:
    {
      SpotLight* spotLight = static_cast<SpotLight*>(light.get());
      dir                  = spotLight->GetComponent<DirectionComponent>()->GetDirection();
      float outAngle       = glm::cos(glm::radians(spotLight->GetOuterAngleVal() * 0.5f));
      float innAngle       = glm::cos(glm::radians(spotLight->GetInnerAngleVal() * 0.5f));
      m_lightingShader->SetShaderParameter("lightRadius", ParameterVariant(spotLight->GetRadiusVal()));
      m_lightingShader->SetShaderParameter("lightOutAngle", ParameterVariant(outAngle));
      m_lightingShader->SetShaderParameter("lightInnAngle", ParameterVariant(innAngle));
    }
    break;
    default:
      assert(0 && "unknown type");
      break;
    }

    m_lightingShader->SetShaderParameter("lightDir", ParameterVariant(dir));

    if (lightType < 3) // does not have shadow
    {
      return;
    }

    float atlasTextureSize = (float) Renderer::m_rhiSettings::g_shadowAtlasTextureSize;
    float bias             = light->GetShadowBiasVal() * Renderer::g_shadowBiasMultiplier;
    const Mat4& projView   = light->m_shadowMapCameraProjectionViewMatrix;
    float atlasResRatio    = light->GetShadowResVal() / atlasTextureSize;

    m_lightingShader->SetShaderParameter("lightShadowMapCameraFar", ParameterVariant(light->m_shadowMapCameraFar));
    m_lightingShader->SetShaderParameter("lightProjectionViewMatrix", ParameterVariant(projView));
    m_lightingShader->SetShaderParameter("lightShadowAtlasCoord",
                                         ParameterVariant(light->m_shadowAtlasCoord / atlasTextureSize));
    m_lightingShader->SetShaderParameter("lightShadowAtlasResRatio", ParameterVariant(atlasResRatio));
    m_lightingShader->SetShaderParameter("lightShadowAtlasLayer", ParameterVariant((float) light->m_shadowAtlasLayer));
    m_lightingShader->SetShaderParameter("lightPCFSamples", ParameterVariant(light->GetPCFSamplesVal()));
    m_lightingShader->SetShaderParameter("lightPCFRadius", ParameterVariant(light->GetPCFRadiusVal()));
    m_lightingShader->SetShaderParameter("lightBleedReduction", ParameterVariant(light->GetBleedingReductionVal()));
    m_lightingShader->SetShaderParameter("lightShadowBias", ParameterVariant(bias));
  }

  uint64 AdditiveLightingPass::ConeMeshHash(float radius, float outerAngle)
  {
    return (uint64) (radius * 100.0f) | (uint64(outerAngle * 100.0f) << 32ull);
  }

  void AdditiveLightingPass::Render()
  {
    Renderer* renderer = GetRenderer();
    renderer->SetFramebuffer(m_lightingFrameBuffer, true, Vec4(0.0f));
    // Deferred render always uses PBR material
    m_fullQuadPass->m_material->m_materialType = MaterialType::PBR;
    m_fullQuadPass->m_params.BlendFunc         = BlendFunction::ONE_TO_ONE; // additive blending
    m_fullQuadPass->m_params.FrameBuffer       = m_lightingFrameBuffer;
    m_fullQuadPass->m_params.FragmentShader    = m_lightingShader;
    m_fullQuadPass->m_params.ClearFrameBuffer  = false;

    m_lightingShader->SetShaderParameter("camPos", ParameterVariant(m_params.Cam->m_node->GetTranslation()));

    renderer->EnableDepthTest(true);
    CameraPtr camera = m_params.Cam;
    Frustum frustum  = ExtractFrustum(camera->GetProjectionMatrix() * camera->GetViewMatrix(), true);

    struct LightAndType
    {
      LightPtr light;
      int type;

      LightAndType() {}

      LightAndType(LightPtr l, int t) : light(l), type(t) {}
    };

    std::vector<LightAndType> screenSpaceLights; // light and type pair
    std::vector<std::pair<LightAndType, RenderJob>> meshLights;

    for (size_t i = 0; i < m_params.lights.size(); i++)
    {
      LightPtr light = m_params.lights[i];
      int hasShadow  = light->GetCastShadowVal() * 3; // if light has shadow index will start from 3
      int lightType  = ((int) light->GetType() - (int) EntityType::Entity_DirectionalLight) + hasShadow;
      assert(lightType < 6 && lightType >= 0 && "light type invalid");

      if (lightType == 1 || lightType == 4) // point light
      {
        PointLight* pointLight = static_cast<PointLight*>(light.get());
        RenderJob job {};
        Vec3 pos     = light->m_node->GetTranslation();
        float radius = pointLight->GetRadiusVal();

        // check if we are inside of the sphere
        if (glm::distance(pos, camera->m_node->GetTranslation()) < radius + camera->Near() + 0.06f)
        {
          // if we are inside of the sphere we should draw on quad
          screenSpaceLights.push_back({light, lightType});
          continue;
        }

        if (!FrustumSphereIntersection(frustum, pos, radius))
        {
          // frustum and sphere does not intersects don't render this point light
          continue;
        }

        job.Mesh           = m_sphereEntity->GetMeshComponent()->GetMeshVal().get();
        job.Material       = m_meshMaterial;
        job.BoundingBox    = BoundingBox(Vec3(-radius), Vec3(radius));
        job.ShadowCaster   = false;
        job.WorldTransform = glm::translate(Mat4(), pos) * glm::scale(Mat4(), Vec3(radius * 1.05f));
        meshLights.push_back(std::make_pair(LightAndType(light, lightType), job));
      }
      else if (lightType == 2 || lightType == 5) // is spot light
      {
        SpotLight* spotLight = static_cast<SpotLight*>(light.get());
        Vec3 dir             = spotLight->GetComponent<DirectionComponent>()->GetDirection();
        Vec3 pos             = spotLight->m_node->GetTranslation();
        float height         = spotLight->GetRadiusVal();
        float outerAngle     = spotLight->GetOuterAngleVal();

        if (ConePointIntersection(pos, dir, height, outerAngle, camera->m_node->GetTranslation()))
        {
          // if we are inside of the cone we should draw on quad
          screenSpaceLights.push_back({light, lightType});
          continue;
        }

        if (!FrustumConeIntersect(frustum, pos, dir, height, outerAngle))
        {
          continue; // we don't see the cone, no need to render
        }

        RenderJob job {};
        job.Mesh           = spotLight->m_volumeMesh.get();
        job.Material       = m_meshMaterial;
        job.ShadowCaster   = false;
        job.WorldTransform = light->m_node->GetTransform();
        meshLights.push_back(std::make_pair(LightAndType(light, lightType), job));
      }
      else // directional light
      {
        screenSpaceLights.push_back({light, lightType});
      }
    }

    m_lightingShader->SetShaderParameter("isScreenSpace", ParameterVariant(1));
    for (auto& [light, lightType] : screenSpaceLights)
    {
      SetLightUniforms(light, lightType);
      RenderSubPass(m_fullQuadPass);
    }

    // swap depth texture of gbuffer and this. because main depth buffer is empty now
    // and we need depth buffer for mesh lights
    DepthTexturePtr mainDepth = m_params.MainFramebuffer->GetDepthTexture();
    m_params.MainFramebuffer->AttachDepthTexture(m_params.GBufferFramebuffer->GetDepthTexture());
    // we need to use gbuffers depth in this pass in order to make proper depth test
    m_lightingFrameBuffer->AttachDepthTexture(m_params.GBufferFramebuffer->GetDepthTexture());
    m_params.GBufferFramebuffer->AttachDepthTexture(mainDepth);

    m_lightingShader->SetShaderParameter("isScreenSpace", ParameterVariant(0));
    renderer->EnableDepthWrite(false);
    for (auto& [lightAndType, job] : meshLights)
    {
      SetLightUniforms(lightAndType.light, lightAndType.type);
      renderer->Render(job, m_params.Cam, {});
    }

    // prepare to write main frame buffer
    m_fullQuadPass->m_params.FragmentShader   = m_mergeShader;
    m_fullQuadPass->m_params.FrameBuffer      = m_params.MainFramebuffer;
    m_fullQuadPass->m_params.ClearFrameBuffer = false;
    m_fullQuadPass->m_params.BlendFunc        = BlendFunction::NONE;

    RenderTargetPtr emmisiveRt = m_params.GBufferFramebuffer->GetAttachment(FAttachment::ColorAttachment3);
    RenderTargetPtr iblRt      = m_params.GBufferFramebuffer->GetAttachment(FAttachment::ColorAttachment6);

    renderer->SetTexture(0, m_lightingRt->m_textureId);
    renderer->SetTexture(1, emmisiveRt->m_textureId);
    renderer->SetTexture(2, iblRt->m_textureId);

    m_mergeShader->SetShaderParameter("aoEnabled", ParameterVariant(m_params.AOTexture != nullptr));
    renderer->SetTexture(5, m_params.AOTexture != nullptr ? m_params.AOTexture->m_textureId : 0);
    // merge lighting, ibl, ao, and emmisive
    RenderSubPass(m_fullQuadPass);
    renderer->EnableDepthWrite(true);
  }

  void AdditiveLightingPass::PostRender() { Pass::PostRender(); }
} // namespace ToolKit