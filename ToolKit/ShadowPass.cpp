/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "ShadowPass.h"

#include "Camera.h"
#include "DirectionComponent.h"
#include "Light.h"
#include "Logger.h"
#include "Material.h"
#include "MathUtil.h"
#include "Mesh.h"
#include "RHI.h"
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
    // Order must match with TextureUtil.shader::UVWToUVLayer
    Mat4 views[6] = {glm::lookAt(ZERO, Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
                     glm::lookAt(ZERO, Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
                     glm::lookAt(ZERO, Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f)),
                     glm::lookAt(ZERO, Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)),
                     glm::lookAt(ZERO, Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, -1.0f, 0.0f)),
                     glm::lookAt(ZERO, Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, -1.0f, 0.0f))};

    for (int i = 0; i < 6; i++)
    {
      DecomposeMatrix(views[i], nullptr, &m_cubeMapRotations[i], nullptr);
    }

    m_shadowAtlas               = MakeNewPtr<RenderTarget>();
    m_shadowFramebuffer         = MakeNewPtr<Framebuffer>();

    // Create shadow material
    auto createShadowMaterialFn = [](StringView vertexShader, StringView fragmentShader) -> MaterialPtr
    {
      ShaderPtr vert             = GetShaderManager()->Create<Shader>(ShaderPath(vertexShader.data(), true));
      ShaderPtr frag             = GetShaderManager()->Create<Shader>(ShaderPath(fragmentShader.data(), true));

      MaterialPtr material       = MakeNewPtr<Material>();
      material->m_vertexShader   = vert;
      material->m_fragmentShader = frag;
      material->GetRenderState()->blendFunction = BlendFunction::NONE;
      material->Init();

      return material;
    };

    m_shadowMatOrtho = createShadowMaterialFn("orthogonalDepthVert.shader", "orthogonalDepthFrag.shader");
    m_shadowMatPersp = createShadowMaterialFn("perspectiveDepthVert.shader", "perspectiveDepthFrag.shader");
  }

  ShadowPass::ShadowPass(const ShadowPassParams& params) : ShadowPass() { m_params = params; }

  ShadowPass::~ShadowPass() {}

  void ShadowPass::Render()
  {
    if (m_lights.empty())
    {
      return;
    }

    Renderer* renderer        = GetRenderer();
    const Vec4 lastClearColor = renderer->m_clearColor;

    // Clear shadow atlas before any draw call
    renderer->SetFramebuffer(m_shadowFramebuffer, GraphicBitFields::AllBits);
    for (int i = 0; i < m_layerCount; i++)
    {
      m_shadowFramebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, m_shadowAtlas, 0, i);
      renderer->ClearBuffer(GraphicBitFields::ColorBits, m_shadowClearColor);
    }

    // Update shadow maps.
    for (LightPtr& light : m_lights)
    {
      light->UpdateShadowCamera();

      if (light->GetLightType() == Light::LightType::Directional)
      {
        DirectionalLightPtr dLight = Cast<DirectionalLight>(light);
        dLight->UpdateShadowFrustum(m_params.viewCamera, m_params.scene);
      }

      RenderShadowMaps(light);
    }

    // The first set attachment did not call hw render pass while rendering shadow map
    if (m_lights.size() > 0)
    {
      RemoveHWRenderPass();
    }

    renderer->m_clearColor = lastClearColor;
  }

  void ShadowPass::PreRender()
  {
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
    m_lights           = m_params.lights;
    erase_if(m_lights, [](LightPtr light) -> bool { return !light->GetCastShadowVal(); });

    InitShadowAtlas();
  }

  void ShadowPass::PostRender() { Pass::PostRender(); }

  RenderTargetPtr ShadowPass::GetShadowAtlas() { return m_shadowAtlas; }

  void ShadowPass::RenderShadowMaps(LightPtr light)
  {
    Renderer* renderer                                = GetRenderer();
    EngineSettings::GraphicSettings& graphicsSettings = GetEngineSettings().Graphics;

    if (light->GetLightType() == Light::LightType::Directional)
    {
      int cascadeCount           = graphicsSettings.cascadeCount;
      DirectionalLightPtr dLight = Cast<DirectionalLight>(light);
      for (int i = 0; i < cascadeCount; i++)
      {
        int layer = dLight->m_shadowAtlasLayers[i];
        m_shadowFramebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, m_shadowAtlas, 0, layer);
        renderer->ClearBuffer(GraphicBitFields::DepthBits, m_shadowClearColor);
        AddHWRenderPass();

        Vec2 coord       = dLight->m_shadowAtlasCoords[i];
        float resolution = light->GetShadowResVal().GetValue<float>();
        renderer->SetViewportSize((uint) coord.x, (uint) coord.y, (uint) resolution, (uint) resolution);

        RenderShadowMap(light, dLight->m_cascadeShadowCameras[i], dLight->m_cascadeCullCameras[i]);

        // Depth is invalidated because, atlas has the shadow map.
        renderer->InvalidateFramebufferDepth(m_shadowFramebuffer);
      }
    }
    else if (light->GetLightType() == Light::LightType::Point)
    {
      for (int i = 0; i < 6; i++)
      {
        int layer = light->m_shadowAtlasLayers[i];
        m_shadowFramebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, m_shadowAtlas, 0, layer);

        AddHWRenderPass();

        light->m_shadowCamera->m_node->SetTranslation(light->m_node->GetTranslation());
        light->m_shadowCamera->m_node->SetOrientation(m_cubeMapRotations[i]);

        renderer->ClearBuffer(GraphicBitFields::DepthBits, m_shadowClearColor);
        AddHWRenderPass();

        Vec2 coord       = light->m_shadowAtlasCoords[i];
        float resolution = light->GetShadowResVal().GetValue<float>();
        renderer->SetViewportSize((uint) coord.x, (uint) coord.y, (uint) resolution, (uint) resolution);

        RenderShadowMap(light, light->m_shadowCamera, light->m_shadowCamera);

        // Depth is invalidated because, atlas has the shadow map.
        renderer->InvalidateFramebufferDepth(m_shadowFramebuffer);
      }
    }
    else
    {
      assert(light->GetLightType() == Light::LightType::Spot);

      m_shadowFramebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0,
                                              m_shadowAtlas,
                                              0,
                                              light->m_shadowAtlasLayers[0]);
      AddHWRenderPass();

      renderer->ClearBuffer(GraphicBitFields::DepthBits, m_shadowClearColor);
      AddHWRenderPass();

      Vec2 coord       = light->m_shadowAtlasCoords[0];
      float resolution = light->GetShadowResVal().GetValue<float>();

      renderer->SetViewportSize((uint) coord.x, (uint) coord.y, (uint) resolution, (uint) resolution);
      RenderShadowMap(light, light->m_shadowCamera, light->m_shadowCamera);

      // Depth is invalidated because, atlas has the shadow map.
      renderer->InvalidateFramebufferDepth(m_shadowFramebuffer);
    }
  }

  void ShadowPass::RenderShadowMap(LightPtr light, CameraPtr shadowCamera, CameraPtr cullCamera)
  {
    Renderer* renderer                                = GetRenderer();
    EngineSettings::GraphicSettings& graphicsSettings = GetEngineSettings().Graphics;

    // Adjust light's camera.
    renderer->SetCamera(shadowCamera, false);

    Light::LightType lightType = light->GetLightType();
    if (lightType == Light::LightType::Directional)
    {
      // Here we will try to find a distance that covers all shadow casters.
      // Shadow camera placed at the outer bounds of the scene to find all shadow casters.
      // The frustum is only used to find potential shadow casters.
      // The tight bounds of the shadow camera which is used to create the shadow map is preserved.
      // The casters that will fall behind the camera will still cast shadows, this is why all the fuss for.
      // In the shader, the objects that fall behind the camera is "pancaked" to shadow camera's front plane.
      const BoundingBox& sceneBox = m_params.scene->GetSceneBoundary();
      Vec3 dir                    = cullCamera->Direction();
      Vec3 pos                    = cullCamera->Position(); // Backup pos.
      Vec3 outerPoint             = pos - glm::normalize(dir) * glm::distance(sceneBox.min, sceneBox.max) * 0.5f;

      cullCamera->m_node->SetTranslation(outerPoint); // Set the camera position.
      cullCamera->SetNearClipVal(0.0f);

      // New far clip is calculated. Its the distance newly calculated outer poi
      cullCamera->SetFarClipVal(glm::distance(outerPoint, pos) + cullCamera->Far());
    }

    // Create render jobs for shadow map generation.
    RenderJobArray jobs;
    RenderData renderData;

    Frustum frustum            = ExtractFrustum(cullCamera->GetProjectViewMatrix(), false);
    EntityRawPtrArray entities = m_params.scene->m_aabbTree.VolumeQuery(frustum);

    RenderJobProcessor::CreateRenderJobs(renderData.jobs, entities);
    RenderJobProcessor::SeperateRenderData(renderData, true);

    renderer->OverrideBlendState(true, BlendFunction::NONE); // Blending must be disabled for shadow map generation.

    // Set material and program.
    MaterialPtr shadowMaterial = lightType == Light::LightType::Directional ? m_shadowMatOrtho : m_shadowMatPersp;
    shadowMaterial->m_fragmentShader->SetDefine("EVSM4", m_useEVSM4 ? "1" : "0");
    shadowMaterial->m_fragmentShader->SetDefine("EnableDiscardPixel", "0");
    shadowMaterial->m_fragmentShader->SetDefine("SMFormat16Bit", std::to_string(!m_use32BitShadowMap));

    GpuProgramManager* gpuProgramManager = GetGpuProgramManager();
    m_program = gpuProgramManager->CreateProgram(shadowMaterial->m_vertexShader, shadowMaterial->m_fragmentShader);
    renderer->BindProgram(m_program);

    // Draw opaque.
    RenderJobItr forwardBegin       = renderData.GetForwardOpaqueBegin();
    RenderJobItr forwardMaskedBegin = renderData.GetForwardAlphaMaskedBegin();
    for (RenderJobItr jobItr = forwardBegin; jobItr < forwardMaskedBegin; jobItr++)
    {
      renderer->Render(*jobItr);
    }

    // Draw alpha masked.
    shadowMaterial->m_fragmentShader->SetDefine("EnableDiscardPixel", "1");
    shadowMaterial->m_fragmentShader->SetDefine("UseAlphaMask", "1");

    m_program = gpuProgramManager->CreateProgram(shadowMaterial->m_vertexShader, shadowMaterial->m_fragmentShader);
    renderer->BindProgram(m_program);

    RenderJobItr translucentBegin = renderData.GetForwardTranslucentBegin();
    for (RenderJobItr jobItr = forwardMaskedBegin; jobItr < translucentBegin; jobItr++)
    {
      renderer->Render(*jobItr);
    }

    // Draw translucent.
    shadowMaterial->m_fragmentShader->SetDefine("UseAlphaMask", "0");

    m_program = gpuProgramManager->CreateProgram(shadowMaterial->m_vertexShader, shadowMaterial->m_fragmentShader);
    renderer->BindProgram(m_program);

    RenderJobItr jobItrEnd = renderData.jobs.end();
    for (RenderJobItr jobItr = translucentBegin; jobItr < jobItrEnd; jobItr++)
    {
      renderer->Render(*jobItr);
    }

    renderer->OverrideBlendState(false, BlendFunction::NONE);
  }

  int ShadowPass::PlaceShadowMapsToShadowAtlas(const LightPtrArray& lights)
  {
    LightPtrArray lightArray = lights;

    // Sort all lights based on resolution.
    std::sort(lightArray.begin(),
              lightArray.end(),
              [](LightPtr l1, LightPtr l2) -> bool
              { return l1->GetShadowResVal().GetValue<float>() < l2->GetShadowResVal().GetValue<float>(); });

    EngineSettings& settings = GetEngineSettings();
    const int cascadeCount   = settings.Graphics.cascadeCount;

    IntArray resolutions;
    for (size_t i = 0; i < lightArray.size(); i++)
    {
      LightPtr light = lightArray[i];
      int resolution = (int) light->GetShadowResVal().GetValue<float>();

      if (light->GetLightType() == Light::Directional)
      {
        for (int ii = 0; ii < cascadeCount; ii++)
        {
          resolutions.push_back(resolution);
        }
      }
      else if (light->GetLightType() == Light::Point)
      {
        for (int ii = 0; ii < 6; ii++)
        {
          resolutions.push_back(resolution);
        }
      }
      else
      {
        assert(light->GetLightType() == Light::LightType::Spot);
        resolutions.push_back(resolution);
      }
    }

    int layerCount                   = 0;
    BinPack2D::PackedRectArray rects = m_packer.Pack(resolutions, RHIConstants::ShadowAtlasTextureSize, &layerCount);

    int rectIndex                    = 0;
    for (int i = 0; i < lightArray.size(); i++)
    {
      LightPtr light = lightArray[i];
      if (light->GetLightType() == Light::LightType::Directional)
      {
        for (int ii = 0; ii < cascadeCount; ii++)
        {
          light->m_shadowAtlasCoords[ii] = rects[rectIndex].coordinate;
          light->m_shadowAtlasLayers[ii] = rects[rectIndex].layer;
          rectIndex++;
        }
      }
      else if (light->GetLightType() == Light::LightType::Point)
      {
        for (int ii = 0; ii < 6; ii++)
        {
          light->m_shadowAtlasCoords[ii] = rects[rectIndex].coordinate;
          light->m_shadowAtlasLayers[ii] = rects[rectIndex].layer;
          rectIndex++;
        }
      }
      else
      {
        assert(light->GetLightType() == Light::LightType::Spot);

        light->m_shadowAtlasCoords[0] = rects[rectIndex].coordinate;
        light->m_shadowAtlasLayers[0] = rects[rectIndex].layer;
        rectIndex++;
      }
    }

    return layerCount;
  }

  void ShadowPass::InitShadowAtlas()
  {
    // Check if the shadow atlas needs to be updated
    bool needChange                                  = false;
    EngineSettings::GraphicSettings& graphicSettings = GetEngineSettings().Graphics;
    if (m_activeCascadeCount != graphicSettings.cascadeCount)
    {
      m_activeCascadeCount = graphicSettings.cascadeCount;
      needChange           = true;
    }

    if (m_useEVSM4 != graphicSettings.useEVSM4)
    {
      m_useEVSM4 = graphicSettings.useEVSM4;
      needChange = true;
    }

    if (m_use32BitShadowMap != graphicSettings.use32BitShadowMap)
    {
      m_use32BitShadowMap = graphicSettings.use32BitShadowMap;
      needChange          = true;
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

      GraphicTypes bufferComponents = m_useEVSM4 ? GraphicTypes::FormatRGBA : GraphicTypes::FormatRG;
      GraphicTypes bufferFormat     = m_useEVSM4 ? GraphicTypes::FormatRGBA32F : GraphicTypes::FormatRG32F;

      if (!m_use32BitShadowMap)
      {
        bufferFormat = m_useEVSM4 ? GraphicTypes::FormatRGBA16F : GraphicTypes::FormatRG16F;
      }

      GraphicTypes sampler = GraphicTypes::SampleLinear;
      if (!TK_GL_OES_texture_float_linear)
      {
        // Fall back to nearest sampling. 32 bit filterable textures are not available.
        sampler = GraphicTypes::SampleNearest;
      }

      const TextureSettings set = {GraphicTypes::Target2DArray,
                                   GraphicTypes::UVClampToEdge,
                                   GraphicTypes::UVClampToEdge,
                                   GraphicTypes::UVClampToEdge,
                                   sampler,
                                   sampler,
                                   bufferFormat,
                                   bufferComponents,
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