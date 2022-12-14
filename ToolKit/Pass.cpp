#include "Pass.h"

#include "Renderer.h"
#include "Toolkit.h"
#include "Viewport.h"

namespace ToolKit
{

  RenderPass::RenderPass()
  {
  }

  RenderPass::RenderPass(const RenderPassParams& params) : m_params(params)
  {
    // Create a default frame buffer.
    if (m_params.FrameBuffer == nullptr)
    {
      m_params.FrameBuffer = std::make_shared<Framebuffer>();
      m_params.FrameBuffer->Init({1024u, 768u, 0, false, true});
    }
  }

  void RenderPass::Render()
  {
    PreRender();

    EntityRawPtrArray translucentDrawList;
    SeperateTranslucentEntities(m_drawList, translucentDrawList);

    RenderOpaque(m_drawList, m_camera, m_params.Lights);
    RenderTranslucent(translucentDrawList, m_camera, m_params.Lights);

    PostRender();
  }

  RenderPass::~RenderPass()
  {
  }

  void RenderPass::PreRender()
  {
    Pass::PreRender();
    Renderer* renderer = GetRenderer();

    // Set self data.
    m_drawList = m_params.Entities;
    m_camera   = m_params.Cam;

    renderer->SetFramebuffer(m_params.FrameBuffer, m_params.ClearFrameBuffer);
    renderer->SetCameraLens(m_camera);

    // Gather volumes.
    renderer->CollectEnvironmentVolumes(m_drawList);
  }

  void RenderPass::PostRender()
  {
    Pass::PostRender();
  }

  void RenderPass::CullLightList(Entity const* entity, LightRawPtrArray& lights)
  {
    LightRawPtrArray bestLights;
    bestLights.reserve(lights.size());

    // Get all directional lights.
    for (int i = 0; i < lights.size(); i++)
    {
      if (lights[i]->GetType() == EntityType::Entity_DirectionalLight)
      {
        bestLights.push_back(lights[i]);
      }
    }

    struct LightSortStruct
    {
      Light* light        = nullptr;
      uint intersectCount = 0;
    };

    // Add the lights inside of the radius first
    std::vector<LightSortStruct> intersectCounts(lights.size());
    BoundingBox aabb = entity->GetAABB(true);
    for (size_t lightIndx = 0; lightIndx < lights.size(); lightIndx++)
    {
      Light* light = lights[lightIndx];
      switch (light->GetType())
      {
      case EntityType::Entity_PointLight:
      case EntityType::Entity_SpotLight:
        break;
      default:
        continue;
      }

      intersectCounts[lightIndx].light = light;
      uint& curIntersectCount = intersectCounts[lightIndx].intersectCount;

      if (light->GetType() == EntityType::Entity_SpotLight)
      {
        // Update in case its outdated.
        light->UpdateShadowCamera();
        Frustum frustum =
            ExtractFrustum(light->m_shadowMapCameraProjectionViewMatrix, false);

        if (FrustumBoxIntersection(frustum, aabb) != IntersectResult::Outside)
        {
          curIntersectCount++;
        }
      }

      if (light->GetType() == EntityType::Entity_PointLight)
      {
        BoundingSphere lightSphere = {light->m_node->GetTranslation(),
                                      light->AffectDistance()};

        if (SphereBoxIntersection(lightSphere, aabb))
        {
          curIntersectCount++;
        }
      }
    }

    std::sort(intersectCounts.begin(),
              intersectCounts.end(),
              [](const LightSortStruct& i1, const LightSortStruct& i2) -> bool {
                return (i1.intersectCount > i2.intersectCount);
              });

    for (uint i = 0; i < intersectCounts.size(); i++)
    {
      if (intersectCounts[i].intersectCount == 0)
      {
        break;
      }

      bestLights.push_back(intersectCounts[i].light);
    }

    lights = bestLights;
  }

  void RenderPass::RenderOpaque(EntityRawPtrArray entities,
                                Camera* cam,
                                const LightRawPtrArray& lights)
  {
    Renderer* renderer = GetRenderer();
    for (Entity* ntt : entities)
    {
      LightRawPtrArray lightList = lights;
      CullLightList(ntt, lightList);

      renderer->Render(ntt, cam, lightList);
    }
  }

  void RenderPass::RenderTranslucent(EntityRawPtrArray entities,
                                     Camera* cam,
                                     const LightRawPtrArray& lights)
  {
    StableSortByDistanceToCamera(entities, cam);
    StableSortByMaterialPriority(entities);

    Renderer* renderer = GetRenderer();
    for (Entity* ntt : entities)
    {
      LightRawPtrArray lightList = lights;
      CullLightList(ntt, lightList);

      // For two sided materials,
      // first render back of translucent objects then render front
      MaterialPtr renderMaterial = renderer->GetRenderMaterial(ntt);
      if (renderMaterial->GetRenderState()->cullMode == CullingType::TwoSided)
      {
        renderMaterial->GetRenderState()->cullMode = CullingType::Front;
        renderer->Render(ntt, cam, lights);

        renderMaterial->GetRenderState()->cullMode = CullingType::Back;
        renderer->Render(ntt, cam, lights);

        renderMaterial->GetRenderState()->cullMode = CullingType::TwoSided;
      }
      else
      {
        renderer->Render(ntt, cam, lightList);
      }
    }
  }

  ShadowPass::ShadowPass()
  {
    Mat4 views[6] = {
        glm::lookAt(ZERO, Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(ZERO, Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(ZERO, Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f)),
        glm::lookAt(ZERO, Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)),
        glm::lookAt(ZERO, Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(ZERO, Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, -1.0f, 0.0f))};

    for (int i = 0; i < 6; ++i)
    {
      DecomposeMatrix(
          views[i], nullptr, &m_cubeMapRotations[i], &m_cubeMapScales[i]);
    }

    m_shadowAtlas       = std::make_shared<RenderTarget>();
    m_shadowFramebuffer = std::make_shared<Framebuffer>();
  }

  ShadowPass::ShadowPass(const ShadowPassParams& params) : ShadowPass()
  {
    m_params = params;
  }

  ShadowPass::~ShadowPass()
  {
  }

  void ShadowPass::Render()
  {
    PreRender();

    const Vec4 lastClearColor = GetRenderer()->m_clearColor;

    // Update shadow maps.
    for (Light* light : m_params.Lights)
    {
      light->InitShadowMapDepthMaterial();

      EntityRawPtrArray entities = m_drawList;
      if (light->GetType() == EntityType::Entity_DirectionalLight)
      {
        static_cast<DirectionalLight*>(light)->UpdateShadowFrustum(entities);
      }
      else
      {
        light->UpdateShadowCamera();
      }

      RenderShadowMaps(light, entities);
      // TODO FilterShadowMap(light);
    }

    GetRenderer()->m_clearColor = lastClearColor;

    PostRender();
  }

  void ShadowPass::PreRender()
  {
    Pass::PreRender();

    m_lastOverrideMat = GetRenderer()->m_overrideMat;

    // Dropout non shadow casters.
    m_drawList = m_params.Entities;
    m_drawList.erase(
        std::remove_if(m_drawList.begin(),
                       m_drawList.end(),
                       [](Entity* ntt) -> bool {
                         return !ntt->IsDrawable() ||
                                !ntt->GetMeshComponent()->GetCastShadowVal();
                       }),
        m_drawList.end());

    // Dropout non shadow casting lights.
    m_params.Lights.erase(std::remove_if(m_params.Lights.begin(),
                                         m_params.Lights.end(),
                                         [](Light* light) -> bool {
                                           return !light->GetCastShadowVal();
                                         }),
                          m_params.Lights.end());

    InitShadowAtlas();

    // Set all shadow atlas layers uncleared
    if (m_layerCount != m_clearedLayers.size())
    {
      m_clearedLayers.resize(m_layerCount);
    }
    for (int i = 0; i < m_layerCount; ++i)
      m_clearedLayers[i] = false;
  }

  void ShadowPass::PostRender()
  {
    GetRenderer()->m_overrideMat = m_lastOverrideMat;
    Pass::PostRender();
  }

  RenderTargetPtr ShadowPass::GetShadowAtlas()
  {
    return m_shadowAtlas;
  }

  void ShadowPass::RenderShadowMaps(Light* light,
                                    const EntityRawPtrArray& entities)
  {
    Renderer* renderer = GetRenderer();

    auto renderForShadowMapFn =
        [this, &renderer](Light* light, EntityRawPtrArray entities) -> void {
      FrustumCull(entities, light->m_shadowCamera);

      renderer->m_overrideMat = light->GetShadowMaterial();
      for (Entity* ntt : entities)
      {
        MaterialPtr entityMat = ntt->GetRenderMaterial();
        renderer->m_overrideMat->SetRenderState(entityMat->GetRenderState());
        renderer->m_overrideMat->UnInit();
        renderer->m_overrideMat->m_alpha          = entityMat->m_alpha;
        renderer->m_overrideMat->m_diffuseTexture = entityMat->m_diffuseTexture;
        renderer->m_overrideMat->GetRenderState()->blendFunction =
            BlendFunction::NONE;
        renderer->m_overrideMat->Init();
        renderer->Render(ntt, light->m_shadowCamera);
      }
    };

    switch (light->GetType())
    {
    case EntityType::Entity_PointLight: {
      renderer->SetFramebuffer(m_shadowFramebuffer, false);

      for (int i = 0; i < 6; ++i)
      {
        m_shadowFramebuffer->SetAttachment(
            Framebuffer::Attachment::ColorAttachment0,
            m_shadowAtlas,
            light->m_shadowAtlasLayer + i);

        // Clear the layer if needed
        if (!m_clearedLayers[light->m_shadowAtlasLayer + i])
        {
          renderer->m_clearColor = Vec4(1.0f);
          renderer->ClearBuffer(GraphicBitFields::AllBits);
          m_clearedLayers[light->m_shadowAtlasLayer + i] = true;
        }
        else
        {
          renderer->ClearBuffer(GraphicBitFields::DepthBits);
        }

        light->m_shadowCamera->m_node->SetTranslation(
            light->m_node->GetTranslation());
        light->m_shadowCamera->m_node->SetOrientation(m_cubeMapRotations[i]);

        // TODO: Scales are not needed. Remove.
        light->m_shadowCamera->m_node->SetScale(m_cubeMapScales[i]);

        renderer->SetViewportSize((uint) light->m_shadowAtlasCoord.x,
                                  (uint) light->m_shadowAtlasCoord.y,
                                  (uint) light->GetShadowResVal(),
                                  (uint) light->GetShadowResVal());

        renderForShadowMapFn(light, entities);
      }
    }
    break;
    case EntityType::Entity_DirectionalLight:
    case EntityType::Entity_SpotLight: {

      renderer->SetFramebuffer(m_shadowFramebuffer, false);
      m_shadowFramebuffer->SetAttachment(
          Framebuffer::Attachment::ColorAttachment0,
          m_shadowAtlas,
          light->m_shadowAtlasLayer);

      // Clear the layer if needed
      if (!m_clearedLayers[light->m_shadowAtlasLayer])
      {
        renderer->m_clearColor = Vec4(1.0f);
        renderer->ClearBuffer(GraphicBitFields::AllBits);
        m_clearedLayers[light->m_shadowAtlasLayer] = true;
      }
      else
      {
        renderer->ClearBuffer(GraphicBitFields::DepthBits);
      }

      renderer->SetViewportSize((uint) light->m_shadowAtlasCoord.x,
                                (uint) light->m_shadowAtlasCoord.y,
                                (uint) light->GetShadowResVal(),
                                (uint) light->GetShadowResVal());

      renderForShadowMapFn(light, entities);
    }
    break;
    default:
      break;
    }
  }

  void ShadowPass::FilterShadowMap(Light* light)
  {
    /*
    if (light->GetType() == EntityType::Entity_PointLight ||
        light->GetShadowThicknessVal() < 0.001f)
    {
      return;
    }

    Renderer* renderer = GetRenderer();
    float softness     = light->GetShadowThicknessVal();
    Vec2 shadowRes     = light->GetShadowResolutionVal();

    renderer->Apply7x1GaussianBlur(light->GetShadowMapRenderTarget(),
                                   light->GetShadowMapTempBlurRt(),
                                   X_AXIS,
                                   softness / shadowRes.x);

    renderer->Apply7x1GaussianBlur(light->GetShadowMapTempBlurRt(),
                                   light->GetShadowMapRenderTarget(),
                                   Y_AXIS,
                                   softness / shadowRes.y);
                                   */
  }

  int ShadowPass::PlaceShadowMapsToShadowAtlas(const LightRawPtrArray& lights)
  {
    int layerCount                           = -1;
    int lastLayerOfDirAndSpotLightShadowsUse = -1;

    // Create 2 arrays: dirandspotlights, point lights
    LightRawPtrArray dirAndSpotLights = lights;
    LightRawPtrArray pointLights;
    LightRawPtrArray::iterator it = dirAndSpotLights.begin();
    while (it != dirAndSpotLights.end())
    {
      if ((*it)->GetType() == EntityType::Entity_PointLight)
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
    auto sortByResFn = [](const Light* l1, const Light* l2) -> bool {
      return l1->GetShadowResVal() > l2->GetShadowResVal();
    };

    std::sort(dirAndSpotLights.begin(), dirAndSpotLights.end(), sortByResFn);
    std::sort(pointLights.begin(), pointLights.end(), sortByResFn);

    // Get dir and spot lights into the pack
    std::vector<int> resolutions;
    resolutions.reserve(dirAndSpotLights.size());
    for (Light* light : dirAndSpotLights)
    {
      resolutions.push_back((int) light->GetShadowResVal());
    }

    std::vector<BinPack2D::PackedRect> rects = m_packer.Pack(
        resolutions, Renderer::m_rhiSettings::g_shadowAtlasTextureSize);

    for (int i = 0; i < rects.size(); ++i)
    {
      dirAndSpotLights[i]->m_shadowAtlasCoord = rects[i].Coord;
      dirAndSpotLights[i]->m_shadowAtlasLayer = rects[i].ArrayIndex;

      lastLayerOfDirAndSpotLightShadowsUse = rects[i].ArrayIndex;
      layerCount = std::max(rects[i].ArrayIndex, layerCount);
    }

    // Get point light into another pack
    resolutions.clear();
    resolutions.reserve(pointLights.size());
    for (Light* light : pointLights)
    {
      resolutions.push_back((int) light->GetShadowResVal());
    }

    rects = m_packer.Pack(resolutions,
                          Renderer::m_rhiSettings::g_shadowAtlasTextureSize);

    for (int i = 0; i < rects.size(); ++i)
    {
      pointLights[i]->m_shadowAtlasCoord = rects[i].Coord;
      pointLights[i]->m_shadowAtlasLayer = rects[i].ArrayIndex;
    }

    // Adjust point light parameters
    for (Light* light : pointLights)
    {
      light->m_shadowAtlasLayer += lastLayerOfDirAndSpotLightShadowsUse + 1;
      light->m_shadowAtlasLayer *= 6;
      layerCount = std::max(light->m_shadowAtlasLayer + 5, layerCount);
    }

    return layerCount + 1;
  }

  void ShadowPass::InitShadowAtlas()
  {
    // Check if the shadow atlas needs to be updated
    bool needChange = false;

    // After this loop lastShadowLights is set with lights with shadows
    int nextId = 0;
    for (int i = 0; i < m_params.Lights.size(); ++i)
    {
      Light* light = m_params.Lights[i];
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
      m_layerCount = PlaceShadowMapsToShadowAtlas(m_params.Lights);

      const int maxLayers = GetRenderer()->GetMaxArrayTextureLayers();
      if (maxLayers < m_layerCount)
      {
        m_layerCount = maxLayers;
        GetLogger()->Log("ERROR: Max array texture layer size is reached: " +
                         std::to_string(maxLayers) + " !");
      }

      const RenderTargetSettigs set = {0,
                                       GraphicTypes::Target2DArray,
                                       GraphicTypes::UVClampToEdge,
                                       GraphicTypes::UVClampToEdge,
                                       GraphicTypes::UVClampToEdge,
                                       GraphicTypes::SampleLinear,
                                       GraphicTypes::SampleLinear,
                                       GraphicTypes::FormatRG32F,
                                       GraphicTypes::FormatRG,
                                       GraphicTypes::TypeFloat,
                                       m_layerCount};

      m_shadowAtlas->Reconstruct(
          Renderer::m_rhiSettings::g_shadowAtlasTextureSize,
          Renderer::m_rhiSettings::g_shadowAtlasTextureSize,
          set);

      if (!m_shadowFramebuffer->Initialized())
      {
        // TODO: Msaa is good for variance shadow mapping.
        m_shadowFramebuffer->Init(
            {Renderer::m_rhiSettings::g_shadowAtlasTextureSize,
             Renderer::m_rhiSettings::g_shadowAtlasTextureSize,
             0,
             false,
             true});
      }
    }
  }

  Pass::Pass()
  {
  }

  Pass::~Pass()
  {
  }

  void Pass::PreRender()
  {
    Renderer* renderer     = GetRenderer();
    m_prevOverrideMaterial = renderer->m_overrideMat;
    m_prevFrameBuffer      = renderer->GetFrameBuffer();
  }

  void Pass::PostRender()
  {
    Renderer* renderer      = GetRenderer();
    renderer->m_overrideMat = m_prevOverrideMaterial;
    renderer->SetFramebuffer(m_prevFrameBuffer, false);
  }

  FullQuadPass::FullQuadPass()
  {
    m_camera = std::make_shared<Camera>(); // Unused.
    m_quad   = std::make_shared<Quad>();

    m_material                 = std::make_shared<Material>();
    m_material->m_vertexShader = GetShaderManager()->Create<Shader>(
        ShaderPath("fullQuadVert.shader", true));
  }

  FullQuadPass::FullQuadPass(const FullQuadPassParams& params) : FullQuadPass()
  {
    m_params = params;
  }

  FullQuadPass::~FullQuadPass()
  {
  }

  void FullQuadPass::Render()
  {
    PreRender();

    Renderer* renderer = GetRenderer();
    renderer->SetFramebuffer(m_params.FrameBuffer,
                             m_params.ClearFrameBuffer,
                             {0.0f, 0.0f, 0.0f, 1.0f});

    renderer->Render(m_quad.get(), m_camera.get(), m_params.lights);

    PostRender();
  }

  void FullQuadPass::PreRender()
  {
    Pass::PreRender();
    GetRenderer()->m_overrideMat = nullptr;
    m_material->m_fragmentShader = m_params.FragmentShader;
    m_material->UnInit(); // Reinit in case, shader change.
    m_material->Init();
    m_material->GetRenderState()->depthTestEnabled = false;

    MeshComponentPtr mc = m_quad->GetMeshComponent();
    MeshPtr mesh        = mc->GetMeshVal();
    mesh->m_material    = m_material;
    mesh->Init();
  }

  void FullQuadPass::PostRender()
  {
    Pass::PostRender();
  }

  StencilRenderPass::StencilRenderPass()
  {
    // Init sub pass.
    m_copyStencilSubPass = std::make_shared<FullQuadPass>();
    m_copyStencilSubPass->m_params.FragmentShader =
        GetShaderManager()->Create<Shader>(
            ShaderPath("unlitColorFrag.shader", true));

    m_solidOverrideMaterial =
        GetMaterialManager()->GetCopyOfUnlitColorMaterial();
  }

  StencilRenderPass::StencilRenderPass(const StencilRenderPassParams& params)
      : StencilRenderPass()
  {
    m_params = params;
  }

  void StencilRenderPass::Render()
  {
    PreRender();

    Renderer* renderer      = GetRenderer();
    renderer->m_overrideMat = m_solidOverrideMaterial;

    // Stencil pass.
    renderer->SetStencilOperation(StencilOperation::AllowAllPixels);
    renderer->ColorMask(false, false, false, false);

    for (Entity* ntt : m_params.DrawList)
    {
      renderer->Render(ntt, m_params.Camera);
    }

    // Copy pass.
    renderer->ColorMask(true, true, true, true);
    renderer->SetStencilOperation(StencilOperation::AllowPixelsFailingStencil);

    m_copyStencilSubPass->Render();

    renderer->SetStencilOperation(StencilOperation::None);

    PostRender();
  }

  void StencilRenderPass::PreRender()
  {
    Pass::PreRender();

    m_frameBuffer = std::make_shared<Framebuffer>();

    FramebufferSettings settings;
    settings.depthStencil    = true;
    settings.useDefaultDepth = true;
    settings.width           = m_params.OutputTarget->m_width;
    settings.height          = m_params.OutputTarget->m_height;

    m_frameBuffer->Init(settings);
    m_frameBuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0,
                                 m_params.OutputTarget);

    m_copyStencilSubPass->m_params.FrameBuffer = m_frameBuffer;

    Renderer* renderer = GetRenderer();
    renderer->SetFramebuffer(m_frameBuffer, true, Vec4(0.0f));
    renderer->SetCameraLens(m_params.Camera);
  }

  void StencilRenderPass::PostRender()
  {
    Pass::PostRender();
  }

  OutlinePass::OutlinePass()
  {
    m_stencilPass = std::make_shared<StencilRenderPass>();
    m_stencilAsRt = std::make_shared<RenderTarget>();

    m_outlinePass  = std::make_shared<FullQuadPass>();
    m_dilateShader = GetShaderManager()->Create<Shader>(
        ShaderPath("dilateFrag.shader", true));
  }

  OutlinePass::OutlinePass(const OutlinePassParams& params) : OutlinePass()
  {
    m_params = params;
  }

  void OutlinePass::Render()
  {
    PreRender();

    // Generate stencil binary image.
    m_stencilPass->Render();

    // Use stencil output as input to the dilation.
    GetRenderer()->SetTexture(0, m_stencilAsRt->m_textureId);
    m_dilateShader->SetShaderParameter("Color",
                                       ParameterVariant(m_params.OutlineColor));

    // Draw outline to the viewport.
    m_outlinePass->m_params.FragmentShader   = m_dilateShader;
    m_outlinePass->m_params.FrameBuffer      = m_params.FrameBuffer;
    m_outlinePass->m_params.ClearFrameBuffer = false;
    m_outlinePass->Render();

    PostRender();
  }

  void OutlinePass::PreRender()
  {
    Pass::PreRender();

    // Create stencil image.
    m_stencilPass->m_params.Camera   = m_params.Camera;
    m_stencilPass->m_params.DrawList = m_params.DrawList;

    // Construct output target.
    FramebufferSettings fbs = m_params.FrameBuffer->GetSettings();
    m_stencilAsRt->ReconstructIfNeeded(fbs.width, fbs.height);
    m_stencilPass->m_params.OutputTarget = m_stencilAsRt;
  }

  void OutlinePass::PostRender()
  {
    Pass::PostRender();
  }

  GammaPass::GammaPass()
  {
    m_copyTexture = std::make_shared<RenderTarget>();
    // m_copyTexture->m_settings.InternalFormat = GraphicTypes::FormatRGBA8;
    // m_copyTexture->m_settings.Type           =
    // GraphicTypes::TypeUnsignedByte;
    m_copyBuffer = std::make_shared<Framebuffer>();
    m_copyBuffer->Init({0, 0, 0, false, false});

    m_gammaPass   = std::make_shared<FullQuadPass>();
    m_gammaShader = GetShaderManager()->Create<Shader>(
        ShaderPath("gammaFrag.shader", true));
  }

  GammaPass::GammaPass(const GammaPassParams& params) : GammaPass()
  {
    m_params = params;
  }

  void GammaPass::PreRender()
  {
    Pass::PreRender();

    Renderer* renderer = GetRenderer();

    // Initiate copy buffer.
    FramebufferSettings fbs;
    fbs.depthStencil    = false;
    fbs.useDefaultDepth = false;
    if (m_params.FrameBuffer == nullptr)
    {
      fbs.width  = renderer->m_windowSize.x;
      fbs.height = renderer->m_windowSize.y;
    }
    else
    {
      FramebufferSettings targetFbs = m_params.FrameBuffer->GetSettings();
      fbs.width                     = targetFbs.width;
      fbs.height                    = targetFbs.height;
    }

    m_copyTexture->ReconstructIfNeeded(fbs.width, fbs.height);
    m_copyBuffer->ReconstructIfNeeded(fbs.width, fbs.height);
    m_copyBuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0,
                                m_copyTexture);

    // Copy given buffer.
    renderer->CopyFrameBuffer(
        m_params.FrameBuffer, m_copyBuffer, GraphicBitFields::ColorBits);

    // Set given buffer as a texture to be read in gamma pass.
    renderer->SetTexture(0, m_copyTexture->m_textureId);

    m_gammaPass->m_params.FragmentShader   = m_gammaShader;
    m_gammaPass->m_params.FrameBuffer      = m_params.FrameBuffer;
    m_gammaPass->m_params.ClearFrameBuffer = false;

    m_gammaShader->SetShaderParameter("Gamma",
                                      ParameterVariant(m_params.Gamma));
  }

  void GammaPass::Render()
  {
    PreRender();
    m_gammaPass->Render();
    PostRender();
  }

  void GammaPass::PostRender()
  {
    Pass::PostRender();
  }

  SceneRenderPass::SceneRenderPass()
  {
    m_shadowPass = std::make_shared<ShadowPass>();
    m_renderPass = std::make_shared<RenderPass>();
  }

  SceneRenderPass::SceneRenderPass(const SceneRenderPassParams& params)
      : SceneRenderPass()
  {
    m_params = params;
  }

  void SceneRenderPass::Render()
  {
    Renderer* renderer = GetRenderer();
    PreRender();

    CullDrawList(m_gBufferPass.m_params.entities, m_params.Cam);
    CullDrawList(m_renderPass->m_params.Entities, m_params.Cam);

    // Gbuffer for deferred render
    m_gBufferPass.Render();

    // Shadow pass
    m_shadowPass->Render();

    renderer->SetShadowAtlas(
        std::static_pointer_cast<Texture>(m_shadowPass->GetShadowAtlas()));

    // Render non-blended entities with deferred renderer
    m_deferredRenderPass.Render();

    // TODO SSAO pass

    // TODO render sky pass

    // Forward render blended entities
    m_renderPass->Render();

    renderer->SetShadowAtlas(nullptr);

    PostRender();
  }

  void SceneRenderPass::PreRender()
  {
    Pass::PreRender();

    SetPassParams();

    m_gBufferPass.InitGBuffers(m_params.MainFramebuffer->GetSettings().width,
                               m_params.MainFramebuffer->GetSettings().height);
  }

  void SceneRenderPass::PostRender()
  {
    Pass::PostRender();
  }

  void SceneRenderPass::SetPassParams()
  {
    m_shadowPass->m_params.Entities = m_params.Scene->GetEntities();
    m_shadowPass->m_params.Lights   = m_params.Lights;

    // Give blended entities to forward render, non-blendeds to deferred
    // render

    EntityRawPtrArray opaqueDrawList = m_params.Scene->GetEntities();
    EntityRawPtrArray translucentAndUnlitDrawList;
    SeperateTranslucentAndUnlitEntities(opaqueDrawList, translucentAndUnlitDrawList);

    m_gBufferPass.m_params.entities = opaqueDrawList;
    m_gBufferPass.m_params.camera   = m_params.Cam;

    m_deferredRenderPass.m_params.ClearFramebuffer = true;
    m_deferredRenderPass.m_params.GBufferFramebuffer =
        m_gBufferPass.m_framebuffer;
    m_deferredRenderPass.m_params.lights          = m_params.Lights;
    m_deferredRenderPass.m_params.MainFramebuffer = m_params.MainFramebuffer;
    m_deferredRenderPass.m_params.GBufferCamera   = m_params.Cam;

    m_renderPass->m_params.Lights           = m_params.Lights;
    m_renderPass->m_params.Cam              = m_params.Cam;
    m_renderPass->m_params.FrameBuffer      = m_params.MainFramebuffer;
    m_renderPass->m_params.ClearFrameBuffer = false;

    m_renderPass->m_params.Entities = translucentAndUnlitDrawList;
  }

  void SceneRenderPass::CullDrawList(EntityRawPtrArray& entities,
                                     Camera* camera)
  {
    // Dropout non visible & drawable entities.
    entities.erase(std::remove_if(entities.begin(),
                                  entities.end(),
                                  [](Entity* ntt) -> bool {
                                    return !ntt->GetVisibleVal() ||
                                           !ntt->IsDrawable();
                                  }),
                   entities.end());

    FrustumCull(entities, camera);
  }

  GBufferPass::GBufferPass()
  {
    RenderTargetSettigs gBufferRenderTargetSettings = {
        0,
        GraphicTypes::Target2D,
        GraphicTypes::UVClampToEdge,
        GraphicTypes::UVClampToEdge,
        GraphicTypes::UVClampToEdge,
        GraphicTypes::SampleNearest,
        GraphicTypes::SampleNearest,
        GraphicTypes::FormatRGB16F,
        GraphicTypes::FormatRGB,
        GraphicTypes::TypeFloat,
        1};

    m_gPosRt =
        std::make_shared<RenderTarget>(1024, 1024, gBufferRenderTargetSettings);
    m_gNormalRt =
        std::make_shared<RenderTarget>(1024, 1024, gBufferRenderTargetSettings);
    m_gColorRt =
        std::make_shared<RenderTarget>(1024, 1024, gBufferRenderTargetSettings);
    m_framebuffer = std::make_shared<Framebuffer>();

    m_gBufferMaterial = std::make_shared<Material>();
  }

  void GBufferPass::InitGBuffers(int width, int height)
  {
    if (m_initialized)
    {
      if (width != m_width || height != m_height)
      {
        UnInitGBuffers();
      }
      else
      {
        return;
      }
    }

    m_width  = width;
    m_height = height;

    // Gbuffers render targets
    m_framebuffer->Init({(uint) width, (uint) height, 0, false, true});
    m_gPosRt->m_width     = width;
    m_gPosRt->m_height    = height;
    m_gNormalRt->m_width  = width;
    m_gNormalRt->m_height = height;
    m_gColorRt->m_width   = width;
    m_gColorRt->m_height  = height;
    m_gPosRt->Init();
    m_gNormalRt->Init();
    m_gColorRt->Init();

    if (!m_attachmentsSet)
    {
      m_framebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0,
                                   m_gPosRt);
      m_framebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment1,
                                   m_gNormalRt);
      m_framebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment2,
                                   m_gColorRt);
      m_attachmentsSet = true;
    }

    // Gbuffer material
    ShaderPtr vertexShader = GetShaderManager()->Create<Shader>(
        ShaderPath("defaultVertex.shader", true));
    ShaderPtr fragmentShader = GetShaderManager()->Create<Shader>(
        ShaderPath("gBufferFrag.shader", true));
    m_gBufferMaterial->m_vertexShader   = vertexShader;
    m_gBufferMaterial->m_fragmentShader = fragmentShader;

    m_initialized = true;
  }

  void GBufferPass::UnInitGBuffers()
  {
    if (!m_initialized)
    {
      return;
    }

    m_framebuffer->UnInit();
    m_attachmentsSet = false;

    m_gPosRt->UnInit();
    m_gNormalRt->UnInit();
    m_gColorRt->UnInit();

    m_initialized = false;
  }

  void GBufferPass::PreRender()
  {
    Pass::PreRender();

    GetRenderer()->SetFramebuffer(m_framebuffer, true, Vec4(0.0f));
    GetRenderer()->SetCameraLens(m_params.camera);

    // TODO change it with CullDrawList and carry the function somewhere else
    // from RenderPass
    m_params.entities.erase(std::remove_if(m_params.entities.begin(),
                                           m_params.entities.end(),
                                           [](Entity* ntt) -> bool {
                                             return !ntt->GetVisibleVal() ||
                                                    !ntt->IsDrawable();
                                           }),
                            m_params.entities.end());
  }

  void GBufferPass::PostRender()
  {
    Pass::PostRender();
  }

  void GBufferPass::Render()
  {
    PreRender();

    Renderer* renderer = GetRenderer();

    for (Entity* ntt : m_params.entities)
    {
      MaterialPtr mat = ntt->GetRenderMaterial();

      m_gBufferMaterial->SetRenderState(mat->GetRenderState());
      m_gBufferMaterial->UnInit();
      m_gBufferMaterial->m_diffuseTexture = mat->m_diffuseTexture;
      m_gBufferMaterial->m_cubeMap        = mat->m_cubeMap;
      m_gBufferMaterial->m_color          = mat->m_color;
      m_gBufferMaterial->m_alpha          = mat->m_alpha;
      m_gBufferMaterial->Init();
      renderer->m_overrideMat = m_gBufferMaterial;

      renderer->Render(ntt, m_params.camera);
    }

    PostRender();
  }

  DeferredRenderPass::DeferredRenderPass()
  {
  }

  DeferredRenderPass::DeferredRenderPass(const DeferredRenderPassParams& params)
  {
    m_params = params;
  }

  void DeferredRenderPass::PreRender()
  {
    Pass::PreRender();

    if (m_deferredRenderShader == nullptr)
    {
      m_deferredRenderShader = GetShaderManager()->Create<Shader>(
          ShaderPath("deferredRenderFrag.shader", true));
    }
    m_deferredRenderShader->SetShaderParameter(
        "camPos",
        ParameterVariant(m_params.GBufferCamera->m_node->GetTranslation(
            TransformationSpace::TS_WORLD)));

    m_fullQuadPass.m_params.ClearFrameBuffer = true;
    m_fullQuadPass.m_params.FragmentShader   = m_deferredRenderShader;
    m_fullQuadPass.m_params.FrameBuffer      = m_params.MainFramebuffer;
    m_fullQuadPass.m_params.lights           = m_params.lights;
  }

  void DeferredRenderPass::PostRender()
  {
    Pass::PostRender();
  }

  void DeferredRenderPass::Render()
  {
    PreRender();

    Renderer* renderer = GetRenderer();

    renderer->SetFramebuffer(m_params.MainFramebuffer, true, Vec4(0.0f));

    // Set gbuffer
    // 9: Position, 10: Normal, 11: Color
    renderer->SetTexture(
        9,
        m_params.GBufferFramebuffer
            ->GetAttachment(Framebuffer::Attachment::ColorAttachment0)
            ->m_textureId);
    renderer->SetTexture(
        10,
        m_params.GBufferFramebuffer
            ->GetAttachment(Framebuffer::Attachment::ColorAttachment1)
            ->m_textureId);
    renderer->SetTexture(
        11,
        m_params.GBufferFramebuffer
            ->GetAttachment(Framebuffer::Attachment::ColorAttachment2)
            ->m_textureId);

    m_fullQuadPass.Render();

    // Copy real depth buffer to main framebuffer depth
    renderer->CopyFrameBuffer(m_params.GBufferFramebuffer,
                              m_params.MainFramebuffer,
                              GraphicBitFields::DepthBits);

    PostRender();
  }

} // namespace ToolKit
