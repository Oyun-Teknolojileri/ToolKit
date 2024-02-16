/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "AdditiveLightingPass.h"

#include "DirectionComponent.h"
#include "Material.h"
#include "MathUtil.h"
#include "Mesh.h"
#include "Shader.h"
#include "TKProfiler.h"

namespace ToolKit
{
  using FAttachment = Framebuffer::Attachment;

  AdditiveLightingPass::AdditiveLightingPass()
  {
    // Render target for light calc.
    TextureSettings oneChannelSet = {};
    oneChannelSet.WarpS           = GraphicTypes::UVClampToEdge;
    oneChannelSet.WarpT           = GraphicTypes::UVClampToEdge;
    oneChannelSet.InternalFormat  = GraphicTypes::FormatRGBA16F;
    oneChannelSet.Format          = GraphicTypes::FormatRGBA;
    oneChannelSet.Type            = GraphicTypes::TypeFloat;
    oneChannelSet.GenerateMipMap  = false;

    int size                      = 128;
    m_lightingRt                  = MakeNewPtr<RenderTarget>(size, size, oneChannelSet);
    m_lightingRt->Init();

    // Frame buffer for light calc.
    m_lightingFrameBuffer = MakeNewPtr<Framebuffer>();
    m_lightingFrameBuffer->Init({size, size, false, false});

    m_fullQuadPass                   = MakeNewPtr<FullQuadPass>();

    ShaderManager* shaderMan         = GetShaderManager();
    m_mergeShader                    = shaderMan->Create<Shader>(ShaderPath("lightMerge.shader", true));
    m_lightingShader                 = shaderMan->Create<Shader>(ShaderPath("additiveLighting.shader", true));

    m_meshMaterial                   = MakeNewPtr<Material>();
    m_meshMaterial->m_fragmentShader = m_lightingShader;
    m_meshMaterial->m_vertexShader   = shaderMan->Create<Shader>(ShaderPath("additiveVertex.shader", true));
    m_meshMaterial->Init();

    RenderState* renderState   = m_meshMaterial->GetRenderState();
    renderState->blendFunction = BlendFunction::ONE_TO_ONE;

    m_sphereEntity             = MakeNewPtr<Sphere>();
    m_sphereEntity->SetNumRingVal(8);
    m_sphereEntity->SetNumSegVal(8);
    m_sphereEntity->SetRadiusVal(1.05f);
  }

  AdditiveLightingPass::~AdditiveLightingPass() {}

  void AdditiveLightingPass::PreRender()
  {
    PUSH_GPU_MARKER("AdditiveLightingPass::PreRender");
    PUSH_CPU_MARKER("AdditiveLightingPass::PreRender");

    Pass::PreRender();

    const FramebufferSettings& fbSet = m_params.MainFramebuffer->GetSettings();
    uint width                       = fbSet.width;
    uint height                      = fbSet.height;

    m_lightingFrameBuffer->ReconstructIfNeeded(width, height);
    m_lightingRt->ReconstructIfNeeded(width, height);
    m_lightingFrameBuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, m_lightingRt);

    Renderer* renderer          = GetRenderer();

    // Set gbuffer
    // 9: Position, 10: Normal, 11: Color, 14: metallic-roughness,
    FramebufferPtr gFrameBuffer = m_params.GBufferFramebuffer;

    renderer->SetTexture(9, gFrameBuffer->GetAttachment(FAttachment::ColorAttachment0)->m_textureId);
    renderer->SetTexture(10, gFrameBuffer->GetAttachment(FAttachment::ColorAttachment1)->m_textureId);
    renderer->SetTexture(11, gFrameBuffer->GetAttachment(FAttachment::ColorAttachment2)->m_textureId);
    renderer->SetTexture(14, gFrameBuffer->GetAttachment(FAttachment::ColorAttachment5)->m_textureId);

    renderer->SetTexture(5, m_params.AOTexture ? m_params.AOTexture->m_textureId : 0);

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void AdditiveLightingPass::SetLightUniforms(LightPtr light, int lightType)
  {
    CPU_FUNC_RANGE();

    Vec3 pos = light->m_node->GetTranslation();
    m_fullQuadPass->UpdateUniform(ShaderUniform("lightType", lightType));
    m_fullQuadPass->UpdateUniform(ShaderUniform("lightPos", pos));
    m_fullQuadPass->UpdateUniform(ShaderUniform("lightIntensity", light->GetIntensityVal()));
    m_fullQuadPass->UpdateUniform(ShaderUniform("lightColor", light->GetColorVal()));

    Vec3 dir;
    switch (lightType)
    {
    case 0: // is directional
    case 3:
      dir = static_cast<DirectionalLight*>(light.get())->GetComponentFast<DirectionComponent>()->GetDirection();
      break;
    case 1: // is point
    case 4:
    {
      PointLight* pointLight = static_cast<PointLight*>(light.get());
      m_fullQuadPass->UpdateUniform(ShaderUniform("lightRadius", pointLight->GetRadiusVal()));
    }
    break;
    case 2: // is spot
    case 5:
    {
      SpotLight* spotLight = static_cast<SpotLight*>(light.get());
      dir                  = spotLight->GetComponentFast<DirectionComponent>()->GetDirection();
      float outAngle       = glm::cos(glm::radians(spotLight->GetOuterAngleVal() * 0.5f));
      float innAngle       = glm::cos(glm::radians(spotLight->GetInnerAngleVal() * 0.5f));
      m_fullQuadPass->UpdateUniform(ShaderUniform("lightRadius", spotLight->GetRadiusVal()));
      m_fullQuadPass->UpdateUniform(ShaderUniform("lightOutAngle", outAngle));
      m_fullQuadPass->UpdateUniform(ShaderUniform("lightInnAngle", innAngle));
    }
    break;
    default:
      assert(0 && "unknown type");
      break;
    }

    m_fullQuadPass->UpdateUniform(ShaderUniform("lightDir", dir));

    if (lightType < 3) // does not have shadow
    {
      return;
    }

    float atlasTextureSize = (float) Renderer::RHIConstants::ShadowAtlasTextureSize;
    float bias             = light->GetShadowBiasVal() * Renderer::RHIConstants::ShadowBiasMultiplier;
    const Mat4& projView   = light->m_shadowMapCameraProjectionViewMatrix;
    float atlasResRatio    = light->GetShadowResVal() / atlasTextureSize;

    m_fullQuadPass->UpdateUniform(ShaderUniform("lightShadowMapCameraFar", light->m_shadowMapCameraFar));
    m_fullQuadPass->UpdateUniform(ShaderUniform("lightProjectionViewMatrix", projView));
    m_fullQuadPass->UpdateUniform(ShaderUniform("lightShadowAtlasCoord", light->m_shadowAtlasCoord / atlasTextureSize));
    m_fullQuadPass->UpdateUniform(ShaderUniform("lightShadowAtlasResRatio", atlasResRatio));
    m_fullQuadPass->UpdateUniform(ShaderUniform("lightShadowAtlasLayer", (float) light->m_shadowAtlasLayer));
    m_fullQuadPass->UpdateUniform(ShaderUniform("lightPCFSamples", light->GetPCFSamplesVal()));
    m_fullQuadPass->UpdateUniform(ShaderUniform("lightPCFRadius", light->GetPCFRadiusVal()));
    m_fullQuadPass->UpdateUniform(ShaderUniform("lightBleedReduction", light->GetBleedingReductionVal()));
    m_fullQuadPass->UpdateUniform(ShaderUniform("lightShadowBias", bias));
  }

  void AdditiveLightingPass::Render()
  {
    PUSH_GPU_MARKER("AdditiveLightingPass::Render");
    PUSH_CPU_MARKER("AdditiveLightingPass::Render");

    Renderer* renderer = GetRenderer();
    renderer->SetFramebuffer(m_lightingFrameBuffer, GraphicBitFields::AllBits);

    // Deferred render always uses PBR material
    m_fullQuadPass->m_params.BlendFunc        = BlendFunction::ONE_TO_ONE; // additive blending
    m_fullQuadPass->m_params.FrameBuffer      = m_lightingFrameBuffer;
    m_fullQuadPass->m_params.ClearFrameBuffer = false;

    m_fullQuadPass->SetFragmentShader(m_lightingShader, renderer);

    m_fullQuadPass->UpdateUniform(ShaderUniform("aoEnabled", m_params.AOTexture != nullptr));
    m_fullQuadPass->UpdateUniform(ShaderUniform("camPos", m_params.Cam->Position()));

    renderer->EnableDepthTest(true);
    CameraPtr camera = m_params.Cam;
    Frustum frustum  = ExtractFrustum(camera->GetProjectionMatrix() * camera->GetViewMatrix(), true);

    struct LightAndType
    {
      LightPtr light = nullptr;
      int type       = 0;

      LightAndType(LightPtr l, int t) : light(l), type(t) {}
    };

    std::vector<LightAndType> screenSpaceLights; // light and type pair
    std::vector<std::pair<LightAndType, RenderJob>> meshLights;

    for (size_t i = 0; i < m_params.lights.size(); i++)
    {
      LightPtr light = m_params.lights[i];
      int hasShadow  = light->GetCastShadowVal() * 3; // if light has shadow index will start from 3
      int lightType  = light->GetLightType() + hasShadow;
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
        job.Material       = m_meshMaterial.get();
        job.BoundingBox    = BoundingBox(Vec3(-radius), Vec3(radius));
        job.ShadowCaster   = false;
        job.WorldTransform = glm::translate(Mat4(), pos) * glm::scale(Mat4(), Vec3(radius * 1.05f));
        meshLights.push_back(std::make_pair(LightAndType(light, lightType), job));
      }
      else if (lightType == 2 || lightType == 5) // is spot light
      {
        SpotLight* spotLight = static_cast<SpotLight*>(light.get());
        Vec3 dir             = spotLight->GetComponentFast<DirectionComponent>()->GetDirection();
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
        job.Material       = m_meshMaterial.get();
        job.ShadowCaster   = false;
        job.WorldTransform = light->m_node->GetTransform();
        meshLights.push_back(std::make_pair(LightAndType(light, lightType), job));
      }
      else // directional light
      {
        screenSpaceLights.push_back({light, lightType});
      }
    }

    m_fullQuadPass->UpdateUniform(ShaderUniform("isScreenSpace", true));
    for (auto& [light, lightType] : screenSpaceLights)
    {
      SetLightUniforms(light, lightType);
      RenderSubPass(m_fullQuadPass);
    }

    renderer->CopyFrameBuffer(m_params.GBufferFramebuffer, m_params.MainFramebuffer, GraphicBitFields::DepthBits);

    // we need to use gbuffers depth in this pass in order to make proper depth test
    renderer->CopyFrameBuffer(m_params.GBufferFramebuffer, m_lightingFrameBuffer, GraphicBitFields::DepthBits);

    renderer->SetCamera(camera, true);

    renderer->EnableDepthWrite(false);

    m_program = GetGpuProgramManager()->CreateProgram(m_meshMaterial->m_vertexShader, m_meshMaterial->m_fragmentShader);
    renderer->BindProgram(m_program);

    for (auto& [lightAndType, job] : meshLights)
    {
      renderer->Render(job);
    }

    m_fullQuadPass->SetFragmentShader(m_mergeShader, renderer);

    // prepare to write main frame buffer
    m_fullQuadPass->m_params.FrameBuffer      = m_params.MainFramebuffer;
    m_fullQuadPass->m_params.ClearFrameBuffer = false;
    m_fullQuadPass->m_params.BlendFunc        = BlendFunction::NONE;

    RenderTargetPtr emmisiveRt = m_params.GBufferFramebuffer->GetAttachment(FAttachment::ColorAttachment3);
    RenderTargetPtr iblRt      = m_params.GBufferFramebuffer->GetAttachment(FAttachment::ColorAttachment6);

    renderer->SetTexture(0, m_lightingRt->m_textureId);
    renderer->SetTexture(1, emmisiveRt->m_textureId);
    renderer->SetTexture(2, iblRt->m_textureId);

    m_fullQuadPass->UpdateUniform(ShaderUniform("aoEnabled", m_params.AOTexture != nullptr));
    renderer->SetTexture(5, m_params.AOTexture != nullptr ? m_params.AOTexture->m_textureId : 0);

    // merge lighting, ibl, ao, and emmisive
    RenderSubPass(m_fullQuadPass);
    renderer->EnableDepthWrite(true);

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void AdditiveLightingPass::PostRender()
  {
    PUSH_GPU_MARKER("AdditiveLightingPass::PostRender");
    PUSH_CPU_MARKER("AdditiveLightingPass::PostRender");

    Pass::PostRender();

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }
} // namespace ToolKit