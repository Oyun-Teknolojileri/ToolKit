/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "ShadowPass.h"

#include "BVH.h"
#include "Camera.h"
#include "DirectionComponent.h"
#include "Logger.h"
#include "Material.h"
#include "MathUtil.h"
#include "Mesh.h"
#include "RHIConstants.h"
#include "RenderSystem.h"
#include "Scene.h"
#include "TKProfiler.h"
#include "TKStats.h"
#include "ToolKit.h"

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
    for (LightPtr& light : m_lights)
    {
      light->InitShadowMapDepthMaterial();
      if (light->GetLightType() == Light::LightType::Directional)
      {
        DirectionalLightPtr dLight = Cast<DirectionalLight>(light);
        dLight->UpdateShadowFrustum(m_params.viewCamera, m_params.scene);
      }

      // Do not update spot or point light shadow cameras since they should be updated on RenderPath that runs this pass
      RenderShadowMaps(light);
    }

    // The first set attachment did not call hw render pass while rendering shadow map
    if (m_lights.size() > 0)
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

    Renderer* renderer = GetRenderer();

    // Dropout non shadow casting lights.
    m_lights           = m_params.scene->GetLights();
    erase_if(m_lights, [](LightPtr light) -> bool { return !light->GetCastShadowVal(); });

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

  void ShadowPass::RenderShadowMaps(LightPtr light)
  {
    CPU_FUNC_RANGE();

    Renderer* renderer        = GetRenderer();

    auto renderForShadowMapFn = [this, &renderer](LightPtr light, CameraPtr shadowCamera) -> void
    {
      PUSH_CPU_MARKER("Render Call");

      MaterialPtr shadowMaterial                 = light->GetShadowMaterial();
      shadowMaterial->GetRenderState()->cullMode = CullingType::TwoSided;
      GpuProgramManager* gpuProgramManager       = GetGpuProgramManager();
      m_program = gpuProgramManager->CreateProgram(shadowMaterial->m_vertexShader, shadowMaterial->m_fragmentShader);
      renderer->BindProgram(m_program);

      renderer->SetCamera(shadowCamera, false);

      Frustum frustum;
      if (light->GetLightType() == Light::LightType::Directional)
      {
        // Set near-far of the directional light frustum huge since we do not want near-far planes to clip objects that
        // should contribute to the directional light shadow.
        const float f               = shadowCamera->Far();
        float value                 = 10000.0f;
        const Vec3 dir              = shadowCamera->Direction();
        const Vec3 pos              = shadowCamera->Position();
        const BoundingBox& sceneBox = m_params.scene->GetSceneBoundary();

        Vec3 nearPos                = pos + (-dir * value);
        while (PointInsideBBox(nearPos, sceneBox.max, sceneBox.min))
        {
          nearPos  = pos + (-dir * value);
          value   *= 10.0f;
        }

        shadowCamera->SetFarClipVal(value + f);
        shadowCamera->m_node->SetTranslation(nearPos);

        frustum = ExtractFrustum(shadowCamera->GetProjectViewMatrix(), false);

        shadowCamera->SetFarClipVal(f);
        shadowCamera->m_node->SetTranslation(pos);
      }
      else
      {
        frustum = ExtractFrustum(shadowCamera->GetProjectViewMatrix(), false);
      }

      EntityRawPtrArray ntties;
      m_params.scene->m_bvh->FrustumTest(frustum, ntties);

      RenderJob job;
      job.Material = shadowMaterial.get();

      for (Entity* ntt : ntties)
      {
        job.WorldTransform = ntt->m_node->GetTransform();
        if (MeshComponentPtr meshCmp = ntt->GetMeshComponent())
        {
          if (meshCmp->GetCastShadowVal())
          {
            const MeshRawPtrArray& meshes = meshCmp->GetMeshVal()->GetAllMeshes();
            for (Mesh* mesh : meshes)
            {
              job.Mesh = mesh;
              renderer->Render(job);
            }
          }
        }
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

        renderForShadowMapFn(light, light->m_shadowCamera);
      }
    }
    else if (light->IsA<SpotLight>())
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

      renderForShadowMapFn(light, light->m_shadowCamera);
    }
    else // if (light->IsA<DirectionalLight>())
    {
      DirectionalLightPtr dLight = Cast<DirectionalLight>(light);
      for (int i = 0; i < GetEngineSettings().Graphics.cascadeCount; ++i)
      {
        m_shadowFramebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0,
                                                m_shadowAtlas,
                                                0,
                                                light->m_shadowAtlasLayer + i);
        AddHWRenderPass();

        renderer->ClearBuffer(GraphicBitFields::DepthBits);
        AddHWRenderPass();

        renderer->SetViewportSize((uint) light->m_shadowAtlasCoord.x,
                                  (uint) light->m_shadowAtlasCoord.y,
                                  (uint) light->GetShadowResVal(),
                                  (uint) light->GetShadowResVal());

        renderForShadowMapFn(light, dLight->m_cascadeShadowCameras[i]);
      }
    }
  }

  int ShadowPass::PlaceShadowMapsToShadowAtlas(const LightPtrArray& lights)
  {
    CPU_FUNC_RANGE();

    int layerCount          = -1;
    int lastLayerInUse      = -1;

    LightPtrArray dirLights = lights;
    LightPtrArray spotLights;
    LightPtrArray pointLights;
    LightPtrArray::iterator it = dirLights.begin();
    while (it != dirLights.end())
    {
      if ((*it)->GetLightType() == Light::LightType::Point)
      {
        pointLights.push_back(*it);
        it = dirLights.erase(it);
      }
      else if ((*it)->GetLightType() == Light::LightType::Spot)
      {
        spotLights.push_back(*it);
        it = dirLights.erase(it);
      }
      else
      {
        ++it;
      }
    }

    // Sort lights based on resolutions (greater to smaller)
    auto sortByResFn = [](const LightPtr l1, const LightPtr l2) -> bool
    { return l1->GetShadowResVal() > l2->GetShadowResVal(); };

    std::sort(dirLights.begin(), dirLights.end(), sortByResFn);
    std::sort(spotLights.begin(), spotLights.end(), sortByResFn);
    std::sort(pointLights.begin(), pointLights.end(), sortByResFn);

    // Get dir and spot lights into the pack
    IntArray resolutions;
    resolutions.reserve(dirLights.size());
    for (LightPtr light : dirLights)
    {
      resolutions.push_back((int) light->GetShadowResVal());
    }

    int cascades                             = GetEngineSettings().Graphics.cascadeCount;

    std::vector<BinPack2D::PackedRect> rects = m_packer.Pack(resolutions, RHIConstants::ShadowAtlasTextureSize);

    int dirLightIndex                        = 0;
    for (int i = 0; i < rects.size(); ++i)
    {
      dirLights[i]->m_shadowAtlasCoord = rects[i].Coord;
      dirLights[i]->m_shadowAtlasLayer = (dirLightIndex * cascades) + rects[i].ArrayIndex;

      lastLayerInUse                   = dirLights[i]->m_shadowAtlasLayer + cascades - 1;
      layerCount                       = std::max(lastLayerInUse, layerCount);

      dirLightIndex++;
    }

    /////////////////////

    resolutions.clear();
    resolutions.reserve(spotLights.size());
    for (LightPtr light : spotLights)
    {
      resolutions.push_back((int) light->GetShadowResVal());
    }

    rects                           = m_packer.Pack(resolutions, RHIConstants::ShadowAtlasTextureSize);

    int spotLightStartingLayerIndex = lastLayerInUse + 1;
    for (int i = 0; i < rects.size(); ++i)
    {
      spotLights[i]->m_shadowAtlasCoord = rects[i].Coord;
      spotLights[i]->m_shadowAtlasLayer = spotLightStartingLayerIndex + rects[i].ArrayIndex;

      lastLayerInUse                    = spotLights[i]->m_shadowAtlasLayer;
      layerCount                        = std::max(lastLayerInUse, layerCount);
    }

    /////////////////////

    // Get point light into another pack
    resolutions.clear();
    resolutions.reserve(pointLights.size());
    for (LightPtr light : pointLights)
    {
      resolutions.push_back((int) light->GetShadowResVal());
    }

    rects = m_packer.Pack(resolutions, RHIConstants::ShadowAtlasTextureSize);

    for (int i = 0; i < rects.size(); ++i)
    {
      pointLights[i]->m_shadowAtlasCoord = rects[i].Coord;
      pointLights[i]->m_shadowAtlasLayer = rects[i].ArrayIndex;
    }

    int pointLightShadowLayerStartIndex = lastLayerInUse + 1;

    // Adjust point light parameters
    int pointLightIndex                 = 0;
    for (LightPtr light : pointLights)
    {
      light->m_shadowAtlasLayer = pointLightShadowLayerStartIndex + pointLightIndex * 6;
      layerCount                = std::max(light->m_shadowAtlasLayer + 5, layerCount);
      pointLightIndex++;
    }

    layerCount++;

    return layerCount;
  }

  void ShadowPass::InitShadowAtlas()
  {
    CPU_FUNC_RANGE();

    // Check if the shadow atlas needs to be updated
    bool needChange                      = false;
    EngineSettings::GraphicSettings& gfx = GetEngineSettings().Graphics;
    if (m_activeCascadeCount != gfx.cascadeCount)
    {
      m_activeCascadeCount = gfx.cascadeCount;
      needChange           = true;
    }

    // After this loop m_previousShadowCasters is set with lights with shadows
    int nextId = 0;
    for (int i = 0; i < m_lights.size(); ++i)
    {
      Light* light = m_lights[i].get();
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

    if (needChange && !m_lights.empty())
    {
      m_previousShadowCasters.resize(nextId);

      // Place shadow textures to atlas
      m_layerCount        = PlaceShadowMapsToShadowAtlas(m_lights);

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
                                   GraphicTypes::SampleLinear,
                                   GraphicTypes::SampleLinear,
                                   GraphicTypes::FormatRG32F,
                                   GraphicTypes::FormatRG,
                                   GraphicTypes::TypeFloat,
                                   m_layerCount,
                                   false};

      m_shadowFramebuffer->DetachColorAttachment(Framebuffer::Attachment::ColorAttachment0);
      m_shadowAtlas->Reconstruct(RHIConstants::ShadowAtlasTextureSize, RHIConstants::ShadowAtlasTextureSize, set);

      if (!m_shadowFramebuffer->Initialized())
      {
        m_shadowFramebuffer->Init(
            {RHIConstants::ShadowAtlasTextureSize, RHIConstants::ShadowAtlasTextureSize, false, true});
      }

      m_shadowFramebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, m_shadowAtlas, 0, 0);
    }
  }

} // namespace ToolKit