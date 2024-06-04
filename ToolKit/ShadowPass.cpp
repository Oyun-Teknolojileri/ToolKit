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

    EngineSettings& settings = GetEngineSettings();
    if (settings.Graphics.useParallelSplitPartitioning)
    {
      float minDistance = settings.Graphics.shadowMinDistance;
      float maxDistance = settings.Graphics.GetShadowMaxDistance();
      float lambda      = settings.Graphics.parallelSplitLambda;

      float nearClip    = m_params.viewCamera->Near();
      float farClip     = m_params.viewCamera->Far();
      float clipRange   = farClip - nearClip;

      float minZ        = nearClip + minDistance * clipRange;
      float maxZ        = nearClip + maxDistance * clipRange;

      float range       = maxZ - minZ;
      float ratio       = maxZ / minZ;

      int cascadeCount  = settings.Graphics.cascadeCount;
      for (int i = 0; i < cascadeCount; i++)
      {
        float p                               = (i + 1) / (float) (cascadeCount);
        float log                             = minZ * std::pow(ratio, p);
        float uniform                         = minZ + range * p;
        float d                               = lambda * (log - uniform) + uniform;
        settings.Graphics.cascadeDistances[i] = (d - nearClip) / clipRange;
      }
    }

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
      shadowMaterial->GetRenderState()->cullMode = CullingType::Back;
      GpuProgramManager* gpuProgramManager       = GetGpuProgramManager();
      m_program = gpuProgramManager->CreateProgram(shadowMaterial->m_vertexShader, shadowMaterial->m_fragmentShader);
      renderer->BindProgram(m_program);

      renderer->SetCamera(shadowCamera, false);

      Frustum frustum;
      if (light->GetLightType() == Light::LightType::Directional)
      {
        // Here we will try to find a distance that covers all shadow casters.
        // Shadow camera placed at the outer bounds of the scene to find all shadow casters.
        // The frustum is only used to find potential shadow casters.
        // The tight bounds of the shadow camera which is used to create the shadow map is preserved.
        // The casters that will fall behind the camera will still cast shadows, this is why all the fuss for.
        // In the shader, the objects that fall behind the camera is "pancaked" to shadow camera's front plane.

        float n                     = shadowCamera->Near(); // Backup near.
        float f                     = shadowCamera->Far();  // Backup the far.

        Vec3 dir                    = shadowCamera->Direction();
        Vec3 pos                    = shadowCamera->Position();
        const BoundingBox& sceneBox = m_params.scene->GetSceneBoundary();

        // Find the intersection where the ray hits to scene.
        // This position will be used to not miss any caster.
        Ray r                       = {pos, dir};
        float t                     = 0.0f;
        RayBoxIntersection(r, sceneBox, t);
        Vec3 outerPoint = PointOnRay(r, t);

        shadowCamera->m_node->SetTranslation(outerPoint); // Set the camera position.

        shadowCamera->SetNearClipVal(0.5f);

        // New far clip is calculated. Its the distance newly calculated outer poi
        shadowCamera->SetFarClipVal(glm::distance(outerPoint, pos) + f);

        // Frustum for culling is calculated from the current camera settings.
        frustum = ExtractFrustum(shadowCamera->GetProjectViewMatrix(), false);

        // Camera is set back to its original values for rendering the shadow.
        shadowCamera->SetNearClipVal(n);
        shadowCamera->SetFarClipVal(f);
        shadowCamera->m_node->SetTranslation(pos);
      }
      else
      {
        frustum = ExtractFrustum(shadowCamera->GetProjectViewMatrix(), false);
      }

      // Find shadow casters from the light's view.
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
      for (int i = 0; i < GetEngineSettings().Graphics.cascadeCount; i++)
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

    // Collect all the maps to create coordinate / layer pairs. ( Packing )
    LightPtrArray lightArray = lights;
    auto dirLightEndItr      = std::partition(lightArray.begin(),
                                         lightArray.end(),
                                         [](const LightPtr& light) -> bool
                                         { return light->GetLightType() == Light::LightType::Directional; });

    // Shadow map accumulator.
    IntArray resolutions;

    // Directional lights requires cascade x number of shadow maps. So we are handling them differently.
    for (auto lightItr = lightArray.begin(); lightItr != dirLightEndItr; lightItr++)
    {
      // Allocate shadow map space for each cascade.
      int shadowRes = (int) glm::round((*lightItr)->GetShadowResVal());
      for (int i = 0; i < RHIConstants::MaxCascadeCount; i++)
      {
        resolutions.push_back(shadowRes);
      }
    }

    // Accumulate spot lights.
    auto spotLightEndItr =
        std::partition(dirLightEndItr,
                       lightArray.end(),
                       [](const LightPtr& light) -> bool { return light->GetLightType() == Light::LightType::Spot; });

    for (auto lightItr = dirLightEndItr; lightItr != spotLightEndItr; lightItr++)
    {
      int shadowRes = (int) glm::round((*lightItr)->GetShadowResVal());
      resolutions.push_back(shadowRes);
    }

    // Accumulate point lights. Each point lights requires 6 shadow maps.
    // TODO: 6 shadow maps for point lights. Point lights won't work at the moment.
    for (auto lightItr = dirLightEndItr; lightItr != spotLightEndItr; lightItr++)
    {
      int shadowRes = (int) glm::round((*lightItr)->GetShadowResVal());
      resolutions.push_back(shadowRes);
    }

    BinPack2D::PackedRectArray rects = m_packer.Pack(resolutions, RHIConstants::ShadowAtlasTextureSize);

    int layerCount                   = -1;
    int lastLayerInUse               = -1;

    LightPtrArray spotLights;
    LightPtrArray pointLights;
    LightPtrArray dirLights    = lights;
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
    int cascades = GetEngineSettings().Graphics.cascadeCount;

    // IntArray resolutions;
    resolutions.reserve(dirLights.size());
    for (LightPtr light : dirLights)
    {
      for (int i = 0; i < cascades; i++)
      {
        resolutions.push_back((int) light->GetShadowResVal());
      }
    }

    for (int i = 0; i < rects.size(); ++i)
    {
      DirectionalLightPtr dirLight            = Cast<DirectionalLight>(dirLights[i]);
      dirLight->m_shadowCascadeAtlasCoords[i] = rects[i].Coord;

      int layerIndex                          = rects[i].ArrayIndex;
      dirLight->m_shadowCascadeAtlasLayers[i] = layerIndex;

      layerCount                              = glm::max(layerIndex, layerCount);
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
    for (int i = 0; i < rects.size(); i++)
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