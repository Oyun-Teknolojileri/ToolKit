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

    renderer->SetFramebuffer(m_params.FrameBuffer, true);
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

    // Update billboards.
    for (Entity* ntt : m_drawList)
    {
      // Update billboards.
      if (ntt->GetType() == EntityType::Entity_Billboard)
      {
        Billboard* billboard = static_cast<Billboard*>(ntt);
        billboard->LookAt(m_camera, m_params.BillboardScale);
      }
    }
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

        if (FrustumBoxIntersection(frustum, aabb) == IntersectResult::Inside)
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
  }

  ShadowPass::ShadowPass(const ShadowPassParams& params) : m_params(params)
  {
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

      EntityRawPtrArray entities = m_drawList;
      if (light->GetType() == EntityType::Entity_DirectionalLight)
      {
        static_cast<DirectionalLight*>(light)->UpdateShadowFrustum(entities);
      }
      else
      {
        light->UpdateShadowCamera();
        FrustumCull(entities, light->m_shadowCamera);
      }

      UpdateShadowMap(light, entities);
      FilterShadowMap(light);
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

    // Clear old rts.
    Renderer* renderer = GetRenderer();
    for (Light* light : m_params.Lights)
    {
      if (light->GetShadowMapFramebuffer())
      {
        renderer->ClearFrameBuffer(light->GetShadowMapFramebuffer(),
                                   Vec4(1.0f));
      }
    }
  }

  void ShadowPass::PostRender()
  {
    Pass::PostRender();
  }

  void ShadowPass::UpdateShadowMap(Light* light,
                                   const EntityRawPtrArray& entities)
  {
    Renderer* renderer = GetRenderer();

    // Update shadow map ProjView matrix every frame for all lights.
    light->InitShadowMap();

    auto renderForShadowMapFn =
        [this, &renderer](Light* light, EntityRawPtrArray entities) -> void {
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
            (Framebuffer::CubemapFace) i);

        light->m_node->SetOrientation(m_cubeMapRotations[i]);

        // TODO: Scales are not needed. Remove.
        light->m_node->SetScale(m_cubeMapScales[i]);

        renderForShadowMapFn(light, entities);
      }
    }
    case EntityType::Entity_DirectionalLight:
    case EntityType::Entity_SpotLight:
      renderer->SetFramebuffer(
          light->GetShadowMapFramebuffer(), true, Vec4(1.0f));
      renderForShadowMapFn(light, entities);
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
    renderer->SetFramebuffer(m_prevFrameBuffer);
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

} // namespace ToolKit
