#include "Pass.h"

#include "DataTexture.h"
#include "DirectionComponent.h"
#include "Renderer.h"
#include "ShaderReflectionCache.h"
#include "Toolkit.h"
#include "Viewport.h"

#include <random>

namespace ToolKit
{
  void RenderPass::StableSortByDistanceToCamera(EntityRawPtrArray& entities,
                                                const Camera* cam)
  {
    std::function<bool(Entity*, Entity*)> sortFn = [cam](Entity* ntt1,
                                                         Entity* ntt2) -> bool
    {
      Vec3 camLoc = cam->m_node->GetTranslation(TransformationSpace::TS_WORLD);

      BoundingBox bb1 = ntt1->GetAABB(true);
      float first     = glm::length2(bb1.GetCenter() - camLoc);

      BoundingBox bb2 = ntt2->GetAABB(true);
      float second    = glm::length2(bb2.GetCenter() - camLoc);

      return second < first;
    };

    if (cam->IsOrtographic())
    {
      sortFn = [cam](Entity* ntt1, Entity* ntt2) -> bool
      {
        float first =
            ntt1->m_node->GetTranslation(TransformationSpace::TS_WORLD).z;

        float second =
            ntt2->m_node->GetTranslation(TransformationSpace::TS_WORLD).z;

        return first < second;
      };
    }

    std::stable_sort(entities.begin(), entities.end(), sortFn);
  }

  void RenderPass::StableSortByMaterialPriority(EntityRawPtrArray& entities)
  {
    std::stable_sort(entities.begin(),
                     entities.end(),
                     [](Entity* a, Entity* b) -> bool
                     {
                       MaterialComponentPtr matA = a->GetMaterialComponent();
                       MaterialComponentPtr matB = b->GetMaterialComponent();
                       if (matA && matB)
                       {
                         int pA =
                             matA->GetMaterialVal()->GetRenderState()->priority;
                         int pB =
                             matB->GetMaterialVal()->GetRenderState()->priority;
                         return pA > pB;
                       }

                       return false;
                     });
  }

  void RenderPass::SeperateTranslucentEntities(
      EntityRawPtrArray& entities,
      EntityRawPtrArray& translucentEntities)
  {
    auto delTrFn = [&translucentEntities](Entity* ntt) -> bool
    {
      // Check too see if there are any material with blend state.
      MaterialComponentPtrArray materials;
      ntt->GetComponent<MaterialComponent>(materials);

      if (!materials.empty())
      {
        for (MaterialComponentPtr& mt : materials)
        {
          if (mt->GetMaterialVal() &&
              mt->GetMaterialVal()->GetRenderState()->blendFunction ==
                  BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA)
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
            if (m->m_material->GetRenderState()->blendFunction ==
                BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA)
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

  void RenderPass::SeperateTranslucentAndUnlitEntities(
      EntityRawPtrArray& entities,
      EntityRawPtrArray& translucentAndUnlitEntities)
  {
    auto delTrFn = [&translucentAndUnlitEntities](Entity* ntt) -> bool
    {
      // Check too see if there are any material with blend state.
      MaterialComponentPtrArray materials;
      ntt->GetComponent<MaterialComponent>(materials);

      if (!materials.empty())
      {
        for (MaterialComponentPtr& mtc : materials)
        {
          if (MaterialPtr mat = mtc->GetMaterialVal())
          {
            RenderState* rs = mat->GetRenderState();
            if (rs->blendFunction ==
                    BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA ||
                rs->useForwardPath)
            {
              translucentAndUnlitEntities.push_back(ntt);
              return true;
            }
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
            if (m->m_material->GetRenderState()->blendFunction ==
                    BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA ||
                m->m_material->GetRenderState()->useForwardPath)
            {
              translucentAndUnlitEntities.push_back(ntt);
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

  ForwardRenderPass::ForwardRenderPass() {}

  ForwardRenderPass::ForwardRenderPass(const ForwardRenderPassParams& params)
      : m_params(params)
  {
    // Create a default frame buffer.
    if (m_params.FrameBuffer == nullptr)
    {
      m_params.FrameBuffer = std::make_shared<Framebuffer>();
      m_params.FrameBuffer->Init({1024u, 768u, false, true});
    }
  }

  void ForwardRenderPass::Render()
  {
    PreRender();

    EntityRawPtrArray translucentDrawList;
    SeperateTranslucentEntities(m_drawList, translucentDrawList);

    RenderOpaque(m_drawList, m_camera, m_params.Lights);
    RenderTranslucent(translucentDrawList, m_camera, m_params.Lights);

    PostRender();
  }

  ForwardRenderPass::~ForwardRenderPass() {}

  void ForwardRenderPass::PreRender()
  {
    Pass::PreRender();
    Renderer* renderer = GetRenderer();

    // Set self data.
    m_drawList         = m_params.Entities;
    m_camera           = m_params.Cam;

    renderer->SetFramebuffer(m_params.FrameBuffer, m_params.ClearFrameBuffer);
    renderer->SetCameraLens(m_camera);

    // Gather volumes.
    renderer->CollectEnvironmentVolumes(m_drawList);
  }

  void ForwardRenderPass::PostRender() { Pass::PostRender(); }

  void ForwardRenderPass::CullLightList(const Entity* entity,
                                        LightRawPtrArray& lights)
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
              [](const LightSortStruct& i1, const LightSortStruct& i2) -> bool
              { return (i1.intersectCount > i2.intersectCount); });

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

  void ForwardRenderPass::RenderOpaque(EntityRawPtrArray entities,
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

  void ForwardRenderPass::RenderTranslucent(EntityRawPtrArray entities,
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
      DecomposeMatrix(views[i],
                      nullptr,
                      &m_cubeMapRotations[i],
                      &m_cubeMapScales[i]);
    }

    m_shadowAtlas       = std::make_shared<RenderTarget>();
    m_shadowFramebuffer = std::make_shared<Framebuffer>();
  }

  ShadowPass::ShadowPass(const ShadowPassParams& params) : ShadowPass()
  {
    m_params = params;
  }

  ShadowPass::~ShadowPass() {}

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
    }

    GetRenderer()->m_clearColor = lastClearColor;

    PostRender();
  }

  void ShadowPass::PreRender()
  {
    Pass::PreRender();

    m_lastOverrideMat = GetRenderer()->m_overrideMat;

    // Dropout non shadow casters.
    m_drawList        = m_params.Entities;
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

  RenderTargetPtr ShadowPass::GetShadowAtlas() { return m_shadowAtlas; }

  void ShadowPass::RenderShadowMaps(Light* light,
                                    const EntityRawPtrArray& entities)
  {
    Renderer* renderer = GetRenderer();

    auto renderForShadowMapFn =
        [this, &renderer](Light* light, EntityRawPtrArray entities) -> void
    {
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
    case EntityType::Entity_PointLight:
    {
      renderer->SetFramebuffer(m_shadowFramebuffer, false);

      for (int i = 0; i < 6; ++i)
      {
        m_shadowFramebuffer->SetAttachment(
            Framebuffer::Attachment::ColorAttachment0,
            m_shadowAtlas,
            0,
            light->m_shadowAtlasLayer + i);

        // Clear the layer if needed
        if (!m_clearedLayers[light->m_shadowAtlasLayer + i])
        {
          renderer->m_clearColor = m_shadowClearColor;
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
    case EntityType::Entity_SpotLight:
    {

      renderer->SetFramebuffer(m_shadowFramebuffer, false);
      m_shadowFramebuffer->SetAttachment(
          Framebuffer::Attachment::ColorAttachment0,
          m_shadowAtlas,
          0,
          light->m_shadowAtlasLayer);

      // Clear the layer if needed
      if (!m_clearedLayers[light->m_shadowAtlasLayer])
      {
        renderer->m_clearColor = m_shadowClearColor;
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

  int ShadowPass::PlaceShadowMapsToShadowAtlas(const LightRawPtrArray& lights)
  {
    int layerCount                           = -1;
    int lastLayerOfDirAndSpotLightShadowsUse = -1;

    // Create 2 arrays: dirandspotlights, point lights
    LightRawPtrArray dirAndSpotLights        = lights;
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
    auto sortByResFn = [](const Light* l1, const Light* l2) -> bool
    { return l1->GetShadowResVal() > l2->GetShadowResVal(); };

    std::sort(dirAndSpotLights.begin(), dirAndSpotLights.end(), sortByResFn);
    std::sort(pointLights.begin(), pointLights.end(), sortByResFn);

    // Get dir and spot lights into the pack
    std::vector<int> resolutions;
    resolutions.reserve(dirAndSpotLights.size());
    for (Light* light : dirAndSpotLights)
    {
      resolutions.push_back((int) light->GetShadowResVal());
    }

    std::vector<BinPack2D::PackedRect> rects =
        m_packer.Pack(resolutions,
                      Renderer::m_rhiSettings::g_shadowAtlasTextureSize);

    for (int i = 0; i < rects.size(); ++i)
    {
      dirAndSpotLights[i]->m_shadowAtlasCoord = rects[i].Coord;
      dirAndSpotLights[i]->m_shadowAtlasLayer = rects[i].ArrayIndex;

      lastLayerOfDirAndSpotLightShadowsUse    = rects[i].ArrayIndex;
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
    int nextId      = 0;
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
      m_layerCount        = PlaceShadowMapsToShadowAtlas(m_params.Lights);

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
             false,
             true});
      }
    }
  }

  Pass::Pass() {}

  Pass::~Pass() {}

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
    m_camera                   = std::make_shared<Camera>(); // Unused.
    m_quad                     = std::make_shared<Quad>();

    m_material                 = std::make_shared<Material>();
    m_material->m_vertexShader = GetShaderManager()->Create<Shader>(
        ShaderPath("fullQuadVert.shader", true));
  }

  FullQuadPass::FullQuadPass(const FullQuadPassParams& params) : FullQuadPass()
  {
    m_params = params;
  }

  FullQuadPass::~FullQuadPass() {}

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
    m_material->GetRenderState()->blendFunction    = m_params.BlendFunc;

    MeshComponentPtr mc                            = m_quad->GetMeshComponent();
    MeshPtr mesh                                   = mc->GetMeshVal();
    mesh->m_material                               = m_material;
    mesh->Init();
  }

  void FullQuadPass::PostRender() { Pass::PostRender(); }

  CubeMapPass::CubeMapPass()
  {
    m_cube = std::make_shared<Cube>();
    m_cube->AddComponent(new MaterialComponent());
  }

  CubeMapPass::CubeMapPass(const CubeMapPassParams& params) : CubeMapPass()
  {
    m_params = params;
  }

  CubeMapPass::~CubeMapPass() {}

  void CubeMapPass::Render()
  {
    PreRender();

    Renderer* renderer = GetRenderer();
    renderer->SetFramebuffer(m_params.FrameBuffer, false);

    m_cube->m_node->SetTransform(m_params.Transform);
    renderer->Render(m_cube.get(), m_params.Cam);

    PostRender();
  }

  void CubeMapPass::PreRender()
  {
    Pass::PreRender();
    MaterialComponentPtr matCom = m_cube->GetMaterialComponent();
    matCom->SetMaterialVal(m_params.Material);
  }

  void CubeMapPass::PostRender() { Pass::PostRender(); }

  StencilRenderPass::StencilRenderPass()
  {
    // Init sub pass.
    m_copyStencilSubPass = std::make_shared<FullQuadPass>();
    m_copyStencilSubPass->m_params.FragmentShader =
        GetShaderManager()->Create<Shader>(
            ShaderPath("unlitFrag.shader", true));

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

    Renderer* renderer                         = GetRenderer();
    renderer->SetFramebuffer(m_frameBuffer, true, Vec4(0.0f));
    renderer->SetCameraLens(m_params.Camera);
  }

  void StencilRenderPass::PostRender() { Pass::PostRender(); }

  OutlinePass::OutlinePass()
  {
    m_stencilPass  = std::make_shared<StencilRenderPass>();
    m_stencilAsRt  = std::make_shared<RenderTarget>();

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
    FramebufferSettings fbs          = m_params.FrameBuffer->GetSettings();
    m_stencilAsRt->ReconstructIfNeeded(fbs.width, fbs.height);
    m_stencilPass->m_params.OutputTarget = m_stencilAsRt;
  }

  void OutlinePass::PostRender() { Pass::PostRender(); }

  SceneRenderPass::SceneRenderPass()
  {
    m_shadowPass        = std::make_shared<ShadowPass>();
    m_forwardRenderPass = std::make_shared<ForwardRenderPass>();
    m_skyPass           = std::make_shared<CubeMapPass>();
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
    CullDrawList(m_forwardRenderPass->m_params.Entities, m_params.Cam);

    // Gbuffer for deferred render
    m_gBufferPass.Render();

    // Shadow pass
    m_shadowPass->Render();

    // SSAO pass
    m_ssaoPass.Render();

    renderer->SetShadowAtlas(
        std::static_pointer_cast<Texture>(m_shadowPass->GetShadowAtlas()));

    // Render non-blended entities with deferred renderer
    m_deferredRenderPass.Render();

    if (m_drawSky)
    {
      m_skyPass->Render();
    }

    // Forward render blended entities
    m_forwardRenderPass->Render();

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

  void SceneRenderPass::PostRender() { Pass::PostRender(); }

  void SceneRenderPass::SetPassParams()
  {
    m_shadowPass->m_params.Entities  = m_params.Scene->GetEntities();
    m_shadowPass->m_params.Lights    = m_params.Lights;

    // Give blended entities to forward render, non-blendeds to deferred
    // render

    EntityRawPtrArray opaqueDrawList = m_params.Scene->GetEntities();
    EntityRawPtrArray translucentAndUnlitDrawList;
    m_forwardRenderPass->SeperateTranslucentAndUnlitEntities(
        opaqueDrawList,
        translucentAndUnlitDrawList);

    m_gBufferPass.m_params.entities                = opaqueDrawList;
    m_gBufferPass.m_params.camera                  = m_params.Cam;

    m_deferredRenderPass.m_params.ClearFramebuffer = true;
    m_deferredRenderPass.m_params.GBufferFramebuffer =
        m_gBufferPass.m_framebuffer;

    m_deferredRenderPass.m_params.lights           = m_params.Lights;
    m_deferredRenderPass.m_params.MainFramebuffer  = m_params.MainFramebuffer;
    m_deferredRenderPass.m_params.Cam              = m_params.Cam;
    m_deferredRenderPass.m_params.AOTexture        = m_ssaoPass.m_ssaoTexture;

    m_forwardRenderPass->m_params.Lights           = m_params.Lights;
    m_forwardRenderPass->m_params.Cam              = m_params.Cam;
    m_forwardRenderPass->m_params.FrameBuffer      = m_params.MainFramebuffer;
    m_forwardRenderPass->m_params.ClearFrameBuffer = false;

    m_forwardRenderPass->m_params.Entities = translucentAndUnlitDrawList;

    m_ssaoPass.m_params.GPositionBuffer    = m_gBufferPass.m_gPosRt;
    m_ssaoPass.m_params.GNormalBuffer      = m_gBufferPass.m_gNormalRt;
    m_ssaoPass.m_params.GLinearDepthBuffer = m_gBufferPass.m_gLinearDepthRt;
    m_ssaoPass.m_params.Cam                = m_params.Cam;

    // Set CubeMapPass for sky.
    m_drawSky                              = false;
    if (SkyBase* sky = m_params.Scene->GetSky())
    {
      if (m_drawSky = sky->GetDrawSkyVal())
      {
        m_skyPass->m_params.FrameBuffer = m_params.MainFramebuffer;
        m_skyPass->m_params.Cam         = m_params.Cam;
        m_skyPass->m_params.Transform   = sky->m_node->GetTransform();
        m_skyPass->m_params.Material    = sky->GetSkyboxMaterial();
      }
    }
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
    m_gEmissiveRt =
        std::make_shared<RenderTarget>(1024, 1024, gBufferRenderTargetSettings);
    gBufferRenderTargetSettings.InternalFormat = GraphicTypes::FormatR32F;
    gBufferRenderTargetSettings.Format         = GraphicTypes::FormatRed;
    m_gLinearDepthRt =
        std::make_shared<RenderTarget>(1024, 1024, gBufferRenderTargetSettings);
    gBufferRenderTargetSettings.InternalFormat = GraphicTypes::FormatRG32F;
    gBufferRenderTargetSettings.Format         = GraphicTypes::FormatRG;
    m_gMetallicRoughnessRt =
        std::make_shared<RenderTarget>(1024, 1024, gBufferRenderTargetSettings);
    m_framebuffer     = std::make_shared<Framebuffer>();

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
    m_framebuffer->Init({(uint) width, (uint) height, false, true});
    m_gPosRt->m_width                = width;
    m_gPosRt->m_height               = height;
    m_gNormalRt->m_width             = width;
    m_gNormalRt->m_height            = height;
    m_gColorRt->m_width              = width;
    m_gColorRt->m_height             = height;
    m_gEmissiveRt->m_width           = width;
    m_gEmissiveRt->m_height          = height;
    m_gLinearDepthRt->m_width        = width;
    m_gLinearDepthRt->m_height       = height;
    m_gMetallicRoughnessRt->m_width  = width;
    m_gMetallicRoughnessRt->m_height = height;
    m_gPosRt->Init();
    m_gNormalRt->Init();
    m_gColorRt->Init();
    m_gEmissiveRt->Init();
    m_gLinearDepthRt->Init();
    m_gMetallicRoughnessRt->Init();

    if (!m_attachmentsSet)
    {
      m_framebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0,
                                   m_gPosRt);
      m_framebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment1,
                                   m_gNormalRt);
      m_framebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment2,
                                   m_gColorRt);
      m_framebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment3,
                                   m_gEmissiveRt);
      m_framebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment4,
                                   m_gLinearDepthRt);
      m_framebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment5,
                                   m_gMetallicRoughnessRt);
      m_attachmentsSet = true;
    }

    // Gbuffer material
    ShaderPtr vertexShader = GetShaderManager()->Create<Shader>(
        ShaderPath("defaultVertex.shader", true));
    ShaderPtr fragmentShader = GetShaderManager()->Create<Shader>(
        ShaderPath("gBufferFrag.shader", true));
    m_gBufferMaterial->m_vertexShader   = vertexShader;
    m_gBufferMaterial->m_fragmentShader = fragmentShader;

    m_initialized                       = true;
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
    m_gEmissiveRt->UnInit();
    m_gLinearDepthRt->UnInit();
    m_gMetallicRoughnessRt->UnInit();

    m_initialized = false;
  }

  void GBufferPass::PreRender()
  {
    Pass::PreRender();

    GetRenderer()->SetFramebuffer(m_framebuffer, true, Vec4(0.0f));
    GetRenderer()->SetCameraLens(m_params.camera);

    m_params.entities.erase(std::remove_if(m_params.entities.begin(),
                                           m_params.entities.end(),
                                           [](Entity* ntt) -> bool {
                                             return !ntt->GetVisibleVal() ||
                                                    !ntt->IsDrawable();
                                           }),
                            m_params.entities.end());
  }

  void GBufferPass::PostRender() { Pass::PostRender(); }

  void GBufferPass::Render()
  {
    PreRender();

    Renderer* renderer = GetRenderer();

    for (Entity* ntt : m_params.entities)
    {
      MaterialPtr mat = ntt->GetRenderMaterial();

      m_gBufferMaterial->SetRenderState(mat->GetRenderState());
      m_gBufferMaterial->UnInit();
      m_gBufferMaterial->m_diffuseTexture  = mat->m_diffuseTexture;
      m_gBufferMaterial->m_emissiveTexture = mat->m_emissiveTexture;
      m_gBufferMaterial->m_emissiveColor   = mat->m_emissiveColor;
      m_gBufferMaterial->m_metallicRoughnessTexture =
          mat->m_metallicRoughnessTexture;
      m_gBufferMaterial->m_normalMap    = mat->m_normalMap;
      m_gBufferMaterial->m_cubeMap      = mat->m_cubeMap;
      m_gBufferMaterial->m_color        = mat->m_color;
      m_gBufferMaterial->m_alpha        = mat->m_alpha;
      m_gBufferMaterial->m_metallic     = mat->m_metallic;
      m_gBufferMaterial->m_roughness    = mat->m_roughness;
      m_gBufferMaterial->m_materialType = mat->m_materialType;
      m_gBufferMaterial->Init();
      renderer->m_overrideMat = m_gBufferMaterial;

      renderer->Render(ntt, m_params.camera);
    }

    PostRender();
  }

  SSAOPass::SSAOPass()
  {
    m_ssaoFramebuffer = std::make_shared<Framebuffer>();
    m_ssaoTexture     = std::make_shared<RenderTarget>();
    m_tempBlurRt      = std::make_shared<RenderTarget>();
    m_noiseTexture    = std::make_shared<SSAONoiseTexture>(4, 4);
  }

  SSAOPass::SSAOPass(const SSAOPassParams& params) : SSAOPass()
  {
    m_params = params;
  }

  void SSAOPass::Render()
  {
    PreRender();

    Renderer* renderer = GetRenderer();

    // Generate SSAO texture
    renderer->SetTexture(0, m_params.GPositionBuffer->m_textureId);
    renderer->SetTexture(1, m_params.GNormalBuffer->m_textureId);
    renderer->SetTexture(2, m_noiseTexture->m_textureId);
    renderer->SetTexture(3, m_params.GLinearDepthBuffer->m_textureId);
    m_quadPass.m_params.FragmentShader = m_ssaoShader;
    m_quadPass.Render();

    // Horizontal blur
    renderer->ApplyAverageBlur(m_ssaoTexture,
                               m_tempBlurRt,
                               X_AXIS,
                               1.0f / m_ssaoTexture->m_width);

    // Vertical blur
    renderer->ApplyAverageBlur(m_tempBlurRt,
                               m_ssaoTexture,
                               Y_AXIS,
                               1.0f / m_ssaoTexture->m_height);

    PostRender();
  }

  void SSAOPass::PreRender()
  {
    Pass::PreRender();

    int width  = m_params.GPositionBuffer->m_width;
    int height = m_params.GPositionBuffer->m_height;

    GenerateSSAONoise();

    // No need destroy and re init framebuffer when size is changed, because
    // the only render target is already being resized.
    m_ssaoFramebuffer->Init({(uint) width, (uint) height, false, false});

    RenderTargetSettigs oneChannelSet = {};
    oneChannelSet.WarpS               = GraphicTypes::UVClampToEdge;
    oneChannelSet.WarpT               = GraphicTypes::UVClampToEdge;
    oneChannelSet.InternalFormat      = GraphicTypes::FormatR32F;
    oneChannelSet.Format              = GraphicTypes::FormatRed;
    oneChannelSet.Type                = GraphicTypes::TypeFloat;

    // Init ssao texture
    m_ssaoTexture->m_settings         = oneChannelSet;
    m_ssaoTexture->ReconstructIfNeeded((uint) width, (uint) height);

    m_ssaoFramebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0,
                                     m_ssaoTexture);

    // Init temporary blur render target
    m_tempBlurRt->m_settings = oneChannelSet;
    m_tempBlurRt->ReconstructIfNeeded((uint) width, (uint) height);

    // Init noise texture
    m_noiseTexture->Init(&m_ssaoNoise[0]);

    GetRenderer()->SetFramebuffer(m_ssaoFramebuffer, true, Vec4(0.0f));

    m_quadPass.m_params.FrameBuffer      = m_ssaoFramebuffer;
    m_quadPass.m_params.ClearFrameBuffer = true;

    // SSAO fragment shader
    if (!m_ssaoShader)
    {
      m_ssaoShader = GetShaderManager()->Create<Shader>(
          ShaderPath("ssaoCalcFrag.shader", true));
      for (uint i = 0; i < 64; ++i)
      {
        m_ssaoShader->SetShaderParameter(g_ssaoSamplesStrCache[i],
                                         ParameterVariant(m_ssaoKernel[i]));
      }
    }
    m_ssaoShader->SetShaderParameter("screen_size",
                                     ParameterVariant(Vec2(width, height)));
    m_ssaoShader->SetShaderParameter(
        "projection",
        ParameterVariant(m_params.Cam->GetProjectionMatrix()));
    m_ssaoShader->SetShaderParameter(
        "viewMatrix",
        ParameterVariant(m_params.Cam->GetViewMatrix()));
  }

  void SSAOPass::PostRender() { Pass::PostRender(); }

  void SSAOPass::GenerateSSAONoise()
  {
    // generate sample kernel
    // ----------------------
    auto lerp = [](float a, float b, float f) { return a + f * (b - a); };

    // generates random floats between 0.0 and 1.0
    std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
    std::default_random_engine generator;
    if (m_ssaoKernel.size() == 0)
    {
      for (uint i = 0; i < 64; ++i)
      {
        Vec3 sample(randomFloats(generator) * 2.0f - 1.0f,
                    randomFloats(generator) * 2.0f - 1.0f,
                    randomFloats(generator));
        sample      = glm::normalize(sample);
        sample      *= randomFloats(generator);
        float scale = float(i) / 64.0f;

        // scale samples s.t. they're more aligned to center of kernel
        scale       = lerp(0.1f, 1.0f, scale * scale);
        sample      *= scale;
        m_ssaoKernel.push_back(sample);
      }
    }

    // generate noise texture
    // ----------------------
    if (m_ssaoNoise.size() == 0)
    {
      for (unsigned int i = 0; i < 16; i++)
      {
        glm::vec2 noise(randomFloats(generator) * 2.0f - 1.0f,
                        randomFloats(generator) * 2.0f - 1.0f);
        m_ssaoNoise.push_back(noise);
      }
    }
  }

  DeferredRenderPass::DeferredRenderPass() {}

  DeferredRenderPass::DeferredRenderPass(const DeferredRenderPassParams& params)
  {
    m_params = params;
  }

  void DeferredRenderPass::PreRender()
  {
    Pass::PreRender();

    if (m_lightDataTexture == nullptr)
    {
      InitLightDataTexture();
    }

    if (m_deferredRenderShader == nullptr)
    {
      m_deferredRenderShader = GetShaderManager()->Create<Shader>(
          ShaderPath("deferredRenderFrag.shader", true));
    }
    m_deferredRenderShader->SetShaderParameter(
        "camPos",
        ParameterVariant(m_params.Cam->m_node->GetTranslation(
            TransformationSpace::TS_WORLD)));

    m_fullQuadPass.m_params.ClearFrameBuffer = m_params.ClearFramebuffer;
    m_fullQuadPass.m_params.FragmentShader   = m_deferredRenderShader;
    m_fullQuadPass.m_params.FrameBuffer      = m_params.MainFramebuffer;

    Renderer* renderer                       = GetRenderer();

    Vec2 sd, nsd, sp, nsp, ss, nss;
    float sizeD, sizeP, sizeS, sizeND, sizeNP, sizeNS;

    // Update light data texture
    m_lightDataTexture->UpdateTextureData(m_params.lights,
                                          sd,
                                          sp,
                                          ss,
                                          nsd,
                                          nsp,
                                          nss,
                                          sizeD,
                                          sizeP,
                                          sizeS,
                                          sizeND,
                                          sizeNP,
                                          sizeNS);

    // Update light uniforms
    m_deferredRenderShader->SetShaderParameter(
        "lightDataTextureWidth",
        ParameterVariant((float) m_lightDataTextureSize.x));
    m_deferredRenderShader->SetShaderParameter("shadowDirLightsInterval",
                                               ParameterVariant(sd));
    m_deferredRenderShader->SetShaderParameter("shadowPointLightsInterval",
                                               ParameterVariant(sp));
    m_deferredRenderShader->SetShaderParameter("shadowSpotLightsInterval",
                                               ParameterVariant(ss));
    m_deferredRenderShader->SetShaderParameter("nonShadowDirLightsInterval",
                                               ParameterVariant(nsd));
    m_deferredRenderShader->SetShaderParameter("nonShadowPointLightsInterval",
                                               ParameterVariant(nsp));
    m_deferredRenderShader->SetShaderParameter("nonShadowSpotLightsInterval",
                                               ParameterVariant(nss));
    m_deferredRenderShader->SetShaderParameter("dirShadowLightDataSize",
                                               ParameterVariant(sizeD));
    m_deferredRenderShader->SetShaderParameter("pointShadowLightDataSize",
                                               ParameterVariant(sizeP));
    m_deferredRenderShader->SetShaderParameter("spotShadowLightDataSize",
                                               ParameterVariant(sizeS));
    m_deferredRenderShader->SetShaderParameter("dirNonShadowLightDataSize",
                                               ParameterVariant(sizeND));
    m_deferredRenderShader->SetShaderParameter("pointNonShadowLightDataSize",
                                               ParameterVariant(sizeNP));
    m_deferredRenderShader->SetShaderParameter("spotNonShadowLightDataSize",
                                               ParameterVariant(sizeNS));

    // Set gbuffer
    // 9: Position, 10: Normal, 11: Color, 12: emissive, 14: metallic-roughness
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
    renderer->SetTexture(
        12,
        m_params.GBufferFramebuffer
            ->GetAttachment(Framebuffer::Attachment::ColorAttachment3)
            ->m_textureId);
    renderer->SetTexture(
        14,
        m_params.GBufferFramebuffer
            ->GetAttachment(Framebuffer::Attachment::ColorAttachment5)
            ->m_textureId);

    renderer->SetTexture(13, m_lightDataTexture->m_textureId);

    if (m_params.AOTexture)
    {
      m_deferredRenderShader->SetShaderParameter("aoEnabled",
                                                 ParameterVariant(1));
      renderer->SetTexture(5, m_params.AOTexture->m_textureId);
    }
    else
    {
      m_deferredRenderShader->SetShaderParameter("aoEnabled",
                                                 ParameterVariant(0));
    }
  }

  void DeferredRenderPass::PostRender()
  {
    // Copy real depth buffer to main framebuffer depth
    GetRenderer()->CopyFrameBuffer(m_params.GBufferFramebuffer,
                                   m_params.MainFramebuffer,
                                   GraphicBitFields::DepthBits);

    Pass::PostRender();
  }

  void DeferredRenderPass::Render()
  {
    PreRender();

    // Deferred render always uses PBR material
    m_fullQuadPass.m_material->m_materialType = MaterialType::PBR;
    m_fullQuadPass.Render();

    PostRender();
  }

  void DeferredRenderPass::InitLightDataTexture()
  {
    m_lightDataTexture =
        std::make_shared<LightDataTexture>(m_lightDataTextureSize.x,
                                           m_lightDataTextureSize.y);
    m_lightDataTexture->Init();
  }

} // namespace ToolKit
