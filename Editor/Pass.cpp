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

    RenderOpaque(m_drawList, m_camera, m_contributingLights);
    RenderTranslucent(translucentDrawList, m_camera, m_contributingLights);

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
    m_drawList = m_params.Scene->GetEntities();
    m_camera   = m_params.Cam;

    renderer->SetFramebuffer(m_params.FrameBuffer, m_params.ClearFrameBuffer);
    renderer->SetCameraLens(m_camera);

    // Set contributing lights.
    LightRawPtrArray& lights = m_params.LightOverride;
    if (lights.empty())
    {
      m_contributingLights = m_params.Scene->GetLights();
    }
    else
    {
      m_contributingLights = lights;
    }

    // Gather volumes.
    renderer->CollectEnvironmentVolumes(m_drawList);

    CullDrawList(m_drawList, m_camera);
  }

  void RenderPass::PostRender()
  {
    Pass::PostRender();
  }

  void RenderPass::CullDrawList(EntityRawPtrArray& entities, Camera* camera)
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

  void RenderPass::SeperateTranslucentEntities(
      EntityRawPtrArray& entities, EntityRawPtrArray& translucentEntities)
  {
    auto delTrFn = [&translucentEntities](Entity* ntt) -> bool {
      // Check too see if there are any material with blend state.
      MaterialComponentPtrArray materials;
      ntt->GetComponent<MaterialComponent>(materials);

      if (!materials.empty())
      {
        for (MaterialComponentPtr& mt : materials)
        {
          if (mt->GetMaterialVal() &&
              mt->GetMaterialVal()->GetRenderState()->blendFunction !=
                  BlendFunction::NONE)
          {
            translucentEntities.push_back(ntt);
            return true;
          }
        }
      }
      else
      {
        MeshComponentPtrArray meshes;
        ntt->GetComponent<MeshComponent>(meshes);

        if (meshes.empty())
        {
          return false;
        }

        for (MeshComponentPtr& ms : meshes)
        {
          MeshRawCPtrArray all;
          ms->GetMeshVal()->GetAllMeshes(all);
          for (const Mesh* m : all)
          {
            if (m->m_material->GetRenderState()->blendFunction !=
                BlendFunction::NONE)
            {
              translucentEntities.push_back(ntt);
              return true;
            }
          }
        }
      }

      return false;
    };

    entities.erase(std::remove_if(entities.begin(), entities.end(), delTrFn),
                   entities.end());
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

    // Update shadow maps.
    for (Light* light : m_params.Lights)
    {
      if (light->GetCastShadowVal() == false)
      {
        continue;
      }

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

    PostRender();
  }

  void ShadowPass::PreRender()
  {
    Pass::PreRender();

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

    // Check if the shadow atlas needs to be updated
    bool needChange = false;

    // After this loop lastShadowLights is set with lights with shadows
    int nextId = 0;
    for (int i = 0; i < m_params.Lights.size(); ++i)
    {
      Light* light = m_params.Lights[i];
      if (light->GetCastShadowVal())
      {
        if (light->m_shadowResolutionUpdated)
        {
          light->m_shadowResolutionUpdated = false;
          needChange                       = true;
        }

        if (nextId >= m_lastShadowLights.size())
        {
          needChange = true;
          m_lastShadowLights.push_back(light);
          nextId++;
          continue;
        }

        if (m_lastShadowLights[nextId] != light)
        {
          needChange = true;
        }

        m_lastShadowLights[nextId] = light;
        nextId++;
      }
    }

    if (needChange)
    {

      // Place shadow textures to atlas
      m_layerCount = PlaceShadowMapsToShadowAtlas(m_lastShadowLights);

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
          g_shadowAtlasTextureSize, g_shadowAtlasTextureSize, set);

      if (!m_shadowFramebuffer->Initialized())
      {
        // TODO: Msaa is good for variance shadow mapping.
        m_shadowFramebuffer->Init({g_shadowAtlasTextureSize,
                                   g_shadowAtlasTextureSize,
                                   0,
                                   false,
                                   true});
      }
    }

    // Set all layers uncleared
    if (m_layerCount != m_clearedLayers.size())
    {
      m_clearedLayers.resize(m_layerCount);
    }
    for (int i = 0; i < m_layerCount; ++i)
      m_clearedLayers[i] = false;
  }

  void ShadowPass::PostRender()
  {
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
      FramebufferPtr shadowMapBuffer = light->GetShadowMapFramebuffer();
      renderer->SetFramebuffer(shadowMapBuffer, true, Vec4(1.0f));

      for (int i = 0; i < 6; ++i)
      {
        shadowMapBuffer->SetAttachment(
            Framebuffer::Attachment::ColorAttachment0,
            light->GetShadowMapRenderTarget(),
            -1,
            (Framebuffer::CubemapFace) i);

        light->m_node->SetOrientation(m_cubeMapRotations[i]);

        // TODO: Scales are not needed. Remove.
        light->m_node->SetScale(m_cubeMapScales[i]);

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

      if (!m_clearedLayers[light->m_shadowAtlasLayer])
      {
        renderer->ClearBuffer(GraphicBitFields::DepthBits);
        m_clearedLayers[light->m_shadowAtlasLayer] = true;
      }

      renderer->SetViewportSize((uint) light->m_shadowAtlasCoord.x,
                                (uint) light->m_shadowAtlasCoord.y,
                                (uint) light->GetShadowResolutionVal().x,
                                (uint) light->GetShadowResolutionVal().x);

      renderForShadowMapFn(light, entities);
    }
    break;
    default:
      break;
    }
  }

  void ShadowPass::FilterShadowMap(Light* light)
  {
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
  }

  int ShadowPass::PlaceShadowMapsToShadowAtlas(const LightRawPtrArray& lights)
  {
    // TODO: use better algorithm

    const int size = g_shadowAtlasTextureSize;

    int layer = 0;
    int rem   = size;
    for (Light* light : lights)
    {
      const float res = light->GetShadowResolutionVal().x;
      assert(res <= g_shadowAtlasTextureSize + 1.0f &&
             "Shadow resolution can not exceed 4096.");
      if (res > rem)
      {
        layer++;
        rem = size;
      }

      light->m_shadowAtlasCoord = Vec2((float) size - rem);
      light->m_shadowAtlasLayer = layer;
      rem -= (int) res;
    }

    return layer + 1;
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

    renderer->Render(m_quad.get(), m_camera.get());

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

    // Copy back buffer.
    renderer->CopyFrameBuffer(
        nullptr, m_copyBuffer, GraphicBitFields::ColorBits);

    // Set back buffer as a texture to be read in gamma pass.
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
  }

  SceneRenderPass::SceneRenderPass()
  {
    m_shadowPass = std::make_shared<ShadowPass>();
    m_renderPass = std::make_shared<RenderPass>();
  }

  SceneRenderPass::SceneRenderPass(const SceneRenderPassParams& params)
      : SceneRenderPass()
  {
    m_params               = params;
    m_shadowPass->m_params = params.shadowPassParams;
    m_renderPass->m_params = params.renderPassParams;
  }

  void SceneRenderPass::Render()
  {
    Renderer* renderer = GetRenderer();
    PreRender();

    // Shadow pass
    m_shadowPass->Render();

    renderer->SetShadowAtlas(
        std::static_pointer_cast<Texture>(m_shadowPass->GetShadowAtlas()));

    // Render pass
    m_renderPass->Render();

    renderer->SetShadowAtlas(nullptr);

    PostRender();
  }

  void SceneRenderPass::PreRender()
  {
    Pass::PreRender();

    m_shadowPass->m_params = m_params.shadowPassParams;
    m_renderPass->m_params = m_params.renderPassParams;
  }

  void SceneRenderPass::PostRender()
  {
  }

} // namespace ToolKit
