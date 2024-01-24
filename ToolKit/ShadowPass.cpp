/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "ShadowPass.h"

#include "Camera.h"
#include "Logger.h"
#include "Material.h"
#include "MathUtil.h"
#include "Mesh.h"
#include "TKProfiler.h"
#include "TKStats.h"
#include "ToolKit.h"

#include <chrono>

namespace ToolKit
{

  ShadowPass::ShadowPass()
  {
    Mat4 views[6] = {glm::lookAt(ZERO, Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
                     glm::lookAt(ZERO, Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
                     glm::lookAt(ZERO, Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f)),
                     glm::lookAt(ZERO, Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)),
                     glm::lookAt(ZERO, Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, -1.0f, 0.0f)),
                     glm::lookAt(ZERO, Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, -1.0f, 0.0f))};

    for (int i = 0; i < 6; ++i)
    {
      DecomposeMatrix(views[i], nullptr, &m_cubeMapRotations[i], &m_cubeMapScales[i]);
    }

    m_shadowAtlas       = MakeNewPtr<RenderTarget>();
    m_shadowFramebuffer = MakeNewPtr<Framebuffer>();
  }

  ShadowPass::ShadowPass(const ShadowPassParams& params) : ShadowPass() { m_params = params; }

  ShadowPass::~ShadowPass() {}

  void ShadowPass::Render()
  {
    PUSH_GPU_MARKER("ShadowPass::Render");
    PUSH_CPU_MARKER("ShadowPass::Render");

    Renderer* renderer        = GetRenderer();
    const Vec4 lastClearColor = renderer->m_clearColor;

    // Clear shadow atlas before any draw call
    renderer->SetFramebuffer(m_shadowFramebuffer, GraphicBitFields::None);
    for (int i = 0; i < m_layerCount; ++i)
    {
      m_shadowFramebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, m_shadowAtlas, 0, i);
      renderer->ClearBuffer(GraphicBitFields::ColorBits, m_shadowClearColor);
    }

    // Update shadow maps.
    for (LightPtr light : m_params.Lights)
    {
      light->InitShadowMapDepthMaterial();
      if (DirectionalLight* dLight = light->As<DirectionalLight>())
      {
        dLight->UpdateShadowFrustum(m_params.RendeJobs, m_params.ViewCamera, m_params.shadowVolume);
      }
      // Do not update spot or point light shadow cameras since they should be updated on RenderPath that runs this pass

      RenderShadowMaps(light, m_params.RendeJobs);
    }

    // The first set attachment did not call hw render pass while rendering shadow map
    if (m_params.Lights.size() > 0)
    {
      RemoveHWRenderPass();
    }

    renderer->m_clearColor = lastClearColor;

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void ShadowPass::PreRender()
  {
    PUSH_GPU_MARKER("ShadowPass::PreRender");
    PUSH_CPU_MARKER("ShadowPass::PreRender");

    Pass::PreRender();

    // Dropout non shadow casters.
    erase_if(m_params.RendeJobs, [](RenderJob& job) -> bool { return !job.ShadowCaster; });

    // Dropout non shadow casting lights.
    erase_if(m_params.Lights, [](LightPtr light) -> bool { return !light->GetCastShadowVal(); });

    InitShadowAtlas();

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void ShadowPass::PostRender()
  {
    PUSH_GPU_MARKER("ShadowPas::PostRender");
    PUSH_CPU_MARKER("ShadowPas::PostRender");

    Pass::PostRender();

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  RenderTargetPtr ShadowPass::GetShadowAtlas() { return m_shadowAtlas; }

  void ShadowPass::RenderShadowMaps(LightPtr light, const RenderJobArray& jobs)
  {
    CPU_FUNC_RANGE();

    auto t_start              = std::chrono::high_resolution_clock::now();

    Renderer* renderer        = GetRenderer();

    auto renderForShadowMapFn = [this, &renderer](LightPtr light, const RenderJobArray& jobs) -> void
    {
      PUSH_CPU_MARKER("Render Call");

      const Mat4& pr             = light->m_shadowCamera->GetProjectionMatrix();
      const Mat4 v               = light->m_shadowCamera->GetViewMatrix();
      const Frustum frustum      = ExtractFrustum(pr * v, false);

      MaterialPtr shadowMaterial = light->GetShadowMaterial();
      renderer->SetCamera(light->m_shadowCamera, false);

      static RenderJobArray culled;
      culled.clear();
      culled.insert(culled.end(), jobs.begin(), jobs.end());
      FrustumCull(culled, light->m_shadowCamera);

      for (const RenderJob& job : culled)
      {
        renderer->m_overrideMat = shadowMaterial;
        MaterialPtr material    = job.Material;
        renderer->m_overrideMat->SetRenderState(material->GetRenderState());
        renderer->m_overrideMat->UnInit();
        renderer->m_overrideMat->SetAlpha(material->GetAlpha());
        renderer->m_overrideMat->m_diffuseTexture                = material->m_diffuseTexture;
        renderer->m_overrideMat->GetRenderState()->blendFunction = BlendFunction::NONE;
        renderer->m_overrideMat->GetRenderState()->cullMode      = CullingType::TwoSided;
        renderer->m_overrideMat->Init();
        renderer->Render(job, light->m_shadowCamera);
      }

      POP_CPU_MARKER();
    };

    if (light->IsA<PointLight>())
    {
      for (int i = 0; i < 6; ++i)
      {
        m_shadowFramebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0,
                                                m_shadowAtlas,
                                                0,
                                                light->m_shadowAtlasLayer + i);

        AddHWRenderPass();

        light->m_shadowCamera->m_node->SetTranslation(light->m_node->GetTranslation());
        light->m_shadowCamera->m_node->SetOrientation(m_cubeMapRotations[i]);

        // TODO: Scales are not needed. Remove.
        light->m_shadowCamera->m_node->SetScale(m_cubeMapScales[i]);

        renderer->ClearBuffer(GraphicBitFields::DepthBits);
        AddHWRenderPass();

        renderer->SetViewportSize((uint) light->m_shadowAtlasCoord.x,
                                  (uint) light->m_shadowAtlasCoord.y,
                                  (uint) light->GetShadowResVal(),
                                  (uint) light->GetShadowResVal());

        renderForShadowMapFn(light, jobs);
      }
    }
    else if (light->IsA<DirectionalLight>() || light->IsA<SpotLight>())
    {
      m_shadowFramebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0,
                                              m_shadowAtlas,
                                              0,
                                              light->m_shadowAtlasLayer);

      AddHWRenderPass();

      renderer->ClearBuffer(GraphicBitFields::DepthBits);
      AddHWRenderPass();

      renderer->SetViewportSize((uint) light->m_shadowAtlasCoord.x,
                                (uint) light->m_shadowAtlasCoord.y,
                                (uint) light->GetShadowResVal(),
                                (uint) light->GetShadowResVal());

      renderForShadowMapFn(light, jobs);
    }
  }

  int ShadowPass::PlaceShadowMapsToShadowAtlas(const LightPtrArray& lights)
  {
    CPU_FUNC_RANGE();

    int layerCount                           = -1;
    int lastLayerOfDirAndSpotLightShadowsUse = -1;

    // Create 2 arrays: dirandspotlights, point lights
    LightPtrArray dirAndSpotLights           = lights;
    LightPtrArray pointLights;
    LightPtrArray::iterator it = dirAndSpotLights.begin();
    while (it != dirAndSpotLights.end())
    {
      if ((*it)->IsA<PointLight>())
      {
        pointLights.push_back(*it);
        it = dirAndSpotLights.erase(it);
      }
      else
      {
        ++it;
      }
    }

    // Sort lights based on resolutions (greater to smaller)
    auto sortByResFn = [](const LightPtr l1, const LightPtr l2) -> bool
    { return l1->GetShadowResVal() > l2->GetShadowResVal(); };

    std::sort(dirAndSpotLights.begin(), dirAndSpotLights.end(), sortByResFn);
    std::sort(pointLights.begin(), pointLights.end(), sortByResFn);

    // Get dir and spot lights into the pack
    IntArray resolutions;
    resolutions.reserve(dirAndSpotLights.size());
    for (LightPtr light : dirAndSpotLights)
    {
      resolutions.push_back((int) light->GetShadowResVal());
    }

    std::vector<BinPack2D::PackedRect> rects =
        m_packer.Pack(resolutions, Renderer::RHIConstants::ShadowAtlasTextureSize);

    for (int i = 0; i < rects.size(); ++i)
    {
      dirAndSpotLights[i]->m_shadowAtlasCoord = rects[i].Coord;
      dirAndSpotLights[i]->m_shadowAtlasLayer = rects[i].ArrayIndex;

      lastLayerOfDirAndSpotLightShadowsUse    = rects[i].ArrayIndex;
      layerCount                              = std::max(rects[i].ArrayIndex, layerCount);
    }

    // Get point light into another pack
    resolutions.clear();
    resolutions.reserve(pointLights.size());
    for (LightPtr light : pointLights)
    {
      resolutions.push_back((int) light->GetShadowResVal());
    }

    rects = m_packer.Pack(resolutions, Renderer::RHIConstants::ShadowAtlasTextureSize);

    for (int i = 0; i < rects.size(); ++i)
    {
      pointLights[i]->m_shadowAtlasCoord = rects[i].Coord;
      pointLights[i]->m_shadowAtlasLayer = rects[i].ArrayIndex;
    }

    int pointLightShadowLayerStartIndex = lastLayerOfDirAndSpotLightShadowsUse + 1;

    // Adjust point light parameters
    int pointLightIndex                 = 0;
    for (LightPtr light : pointLights)
    {
      light->m_shadowAtlasLayer = pointLightShadowLayerStartIndex + pointLightIndex * 6;
      layerCount                = light->m_shadowAtlasLayer + 6;
      pointLightIndex++;
    }

    if (pointLights.empty())
    {
      layerCount += 1;
    }

    return layerCount;
  }

  void ShadowPass::InitShadowAtlas()
  {
    CPU_FUNC_RANGE();

    // Check if the shadow atlas needs to be updated
    bool needChange = false;

    // After this loop m_previousShadowCasters is set with lights with shadows
    int nextId      = 0;
    for (int i = 0; i < m_params.Lights.size(); ++i)
    {
      LightPtr light = m_params.Lights[i];
      if (light->m_shadowResolutionUpdated)
      {
        light->m_shadowResolutionUpdated = false;
        needChange                       = true;
      }

      if (nextId >= m_previousShadowCasters.size())
      {
        needChange = true;
        m_previousShadowCasters.push_back(light->GetIdVal());
        nextId++;
        continue;
      }

      if (m_previousShadowCasters[nextId] != light->GetIdVal())
      {
        needChange = true;
      }

      m_previousShadowCasters[nextId] = light->GetIdVal();
      nextId++;
    }

    if (needChange)
    {
      m_previousShadowCasters.resize(nextId);

      // Place shadow textures to atlas
      m_layerCount        = PlaceShadowMapsToShadowAtlas(m_params.Lights);

      const int maxLayers = GetRenderer()->GetMaxArrayTextureLayers();
      if (maxLayers < m_layerCount)
      {
        m_layerCount = maxLayers;
        GetLogger()->Log("ERROR: Max array texture layer size is reached: " + std::to_string(maxLayers) + " !");
      }

      const TextureSettings set = {GraphicTypes::Target2DArray,
                                   GraphicTypes::UVClampToEdge,
                                   GraphicTypes::UVClampToEdge,
                                   GraphicTypes::UVClampToEdge,
                                   GraphicTypes::SampleNearest,
                                   GraphicTypes::SampleNearest,
                                   GraphicTypes::FormatRG32F,
                                   GraphicTypes::FormatRG,
                                   GraphicTypes::TypeFloat,
                                   m_layerCount,
                                   false};

      m_shadowFramebuffer->DetachColorAttachment(Framebuffer::Attachment::ColorAttachment0);
      m_shadowAtlas->Reconstruct(Renderer::RHIConstants::ShadowAtlasTextureSize,
                                 Renderer::RHIConstants::ShadowAtlasTextureSize,
                                 set);

      if (!m_shadowFramebuffer->Initialized())
      {
        m_shadowFramebuffer->Init({Renderer::RHIConstants::ShadowAtlasTextureSize,
                                   Renderer::RHIConstants::ShadowAtlasTextureSize,
                                   false,
                                   true});
      }

      m_shadowFramebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, m_shadowAtlas, 0, 0);
    }
  }

} // namespace ToolKit