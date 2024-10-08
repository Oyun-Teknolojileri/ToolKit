/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Thumbnail.h"

#include "App.h"

#include <Camera.h>
#include <GradientSky.h>
#include <Material.h>
#include <Mesh.h>
#include <Surface.h>

#include <DebugNew.h>

namespace ToolKit
{

  namespace Editor
  {
    ThumbnailRenderer::ThumbnailRenderer()
    {
      m_maxThumbSize                 = 300;

      FramebufferSettings fbSettings = {m_maxThumbSize, m_maxThumbSize, false, true, 4};
      m_thumbnailBuffer              = MakeNewPtr<Framebuffer>(fbSettings, "ThumbnailRendererFB");
      m_thumbnailBuffer->Init();

      m_thumbnailScene = MakeNewPtr<Scene>();
      m_sky            = MakeNewPtr<GradientSky>();
      m_sky->Init();

      m_entity = MakeNewPtr<Entity>();
      m_entity->AddComponent<MeshComponent>();

      m_sphere      = MakeNewPtr<Sphere>();
      m_surface     = MakeNewPtr<Surface>();

      m_lightSystem = MakeNewPtr<ThreePointLightSystem>();
      m_lightSystem->m_lights[0]->SetIntensityVal(2.0f);

      m_cam                          = MakeNewPtr<Camera>();

      m_params.applyGammaTonemapFxaa = true;
    }

    ThumbnailRenderer::~ThumbnailRenderer()
    {
      m_thumbnailBuffer = nullptr;
      m_thumbnailRT     = nullptr;

      m_thumbnailScene->ClearEntities();
      m_thumbnailScene = nullptr;

      m_sphere         = nullptr;
      m_cam            = nullptr;
      m_entity         = nullptr;

      m_lightSystem    = nullptr;
    }

    RenderTargetPtr ThumbnailRenderer::RenderThumbnail(Renderer* renderer, const DirectoryEntry& dirEnt)
    {
      String fullpath = dirEnt.GetFullPath();
      m_thumbnailScene->ClearEntities();
      m_renderData.jobs.clear();

      m_thumbnailScene->AddEntity(m_sky);

      m_params.Gfx = EngineSettings::PostProcessingSettings();

      // TODO: This function should not load meshes.
      // Instead another task queue may be used to load meshes async.
      // Because we are in the rendering phase at this stage.

      if (dirEnt.m_ext == MESH || dirEnt.m_ext == SKINMESH)
      {
        MeshPtr mesh = nullptr;
        if (dirEnt.m_ext == MESH)
        {
          mesh = GetMeshManager()->Create<Mesh>(fullpath);
        }
        else
        {
          mesh = GetMeshManager()->Create<SkinMesh>(fullpath);
        }

        mesh->Init();
        MeshComponentPtr meshComp = m_entity->GetMeshComponent();
        meshComp->SetMeshVal(mesh);

        if (dirEnt.m_ext == SKINMESH)
        {
          SkinMesh* skinMesh            = (SkinMesh*) mesh.get();
          SkeletonComponentPtr skelComp = m_entity->AddComponent<SkeletonComponent>();
          skelComp->SetSkeletonResourceVal(skinMesh->m_skeleton);
          skelComp->Init();
        }

        m_entity->InvalidateSpatialCaches();
        m_thumbnailScene->AddEntity(m_entity);

        m_cam->SetLens(glm::half_pi<float>(), 1.0f);
        m_cam->FocusToBoundingBox(m_entity->GetBoundingBox(true), 1.5f);
      }
      else if (dirEnt.m_ext == MATERIAL)
      {
        m_params.Gfx.SSAOEnabled = false;

        MaterialPtr mat          = GetMaterialManager()->Create<Material>(fullpath);
        if (MaterialComponentPtr mc = m_sphere->GetMaterialComponent())
        {
          mc->SetFirstMaterial(mat);
        }

        m_sphere->InvalidateSpatialCaches();
        m_thumbnailScene->AddEntity(m_sphere);

        m_cam->SetLens(glm::half_pi<float>(), 1.0f);
        m_cam->m_node->SetOrientation(Quaternion());
        m_cam->m_node->SetTranslation(Vec3(0.0f, 0.0f, 1.5f));
      }
      else if (SupportedImageFormat(dirEnt.m_ext))
      {
        m_params.Gfx.BloomEnabled = false;
        m_params.Gfx.SSAOEnabled  = false;

        TexturePtr texture        = nullptr;
        if (dirEnt.m_ext == HDR)
        {
          texture = GetTextureManager()->Create<Hdri>(fullpath);
        }
        else
        {
          texture = GetTextureManager()->Create<Texture>(fullpath);
        }
        texture->Init(true);

        float maxDim = float(glm::max(texture->m_width, texture->m_height));
        float w      = (texture->m_width / maxDim) * m_maxThumbSize;
        float h      = (texture->m_height / maxDim) * m_maxThumbSize;

        m_surface->InvalidateSpatialCaches();
        m_surface->Update(Vec2(w, h));
        m_surface->UpdateGeometry(false);

        MaterialPtr unlitMaterial       = GetMaterialManager()->GetCopyOfUIMaterial();
        unlitMaterial->m_diffuseTexture = texture;

        m_surface->GetMaterialComponent()->SetFirstMaterial(unlitMaterial);

        m_thumbnailScene->AddEntity(m_surface);
        m_cam->m_orthographicScale = 1.0f;
        m_cam->SetLens(w * -0.5f, w * 0.5f, h * -0.5f, h * 0.5f, 0.01f, 1000.0f);

        m_cam->m_node->SetOrientation(Quaternion());
        m_cam->m_node->SetTranslation(Vec3(0.0f, 0.0f, 10.0f), TransformationSpace::TS_LOCAL);
      }
      else // extension is not recognized, this is probably shader file.
      {
        return g_app->m_thumbnailManager.GetDefaultThumbnail();
      }

      m_thumbnailScene->Update(0.0f);

      m_thumbnailRT = MakeNewPtr<RenderTarget>(m_maxThumbSize, m_maxThumbSize, TextureSettings());
      m_thumbnailRT->Init();

      m_thumbnailBuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, m_thumbnailRT);

      Mat4 camTs = m_cam->m_node->GetTransform();
      m_lightSystem->m_parentNode->SetTransform(camTs);

      m_params.overrideLights  = m_lightSystem->m_lights;
      m_params.Scene           = m_thumbnailScene;
      m_params.Cam             = m_cam;
      m_params.MainFramebuffer = m_thumbnailBuffer;

      Render(renderer);

      return m_thumbnailRT;
    }

    ThumbnailManager::ThumbnailManager() { m_defaultThumbnail = MakeNewPtr<RenderTarget>(10u, 10u, TextureSettings()); }

    ThumbnailManager::~ThumbnailManager()
    {
      m_defaultThumbnail = nullptr;
      m_thumbnailCache.clear();
    }

    bool ThumbnailManager::IsDefaultThumbnail(RenderTargetPtr thumb) { return thumb == m_defaultThumbnail; }

    RenderTargetPtr ThumbnailManager::GetDefaultThumbnail() { return m_defaultThumbnail; }

    RenderTargetPtr ThumbnailManager::GetThumbnail(const DirectoryEntry& dirEnt)
    {
      String fullPath = dirEnt.GetFullPath();

      if (!Exist(fullPath))
      {
        CreateRenderTask(dirEnt);
        m_thumbnailCache[fullPath] = m_defaultThumbnail;
      }

      return m_thumbnailCache[fullPath];
    }

    bool ThumbnailManager::TryGetThumbnail(uint& iconId, const DirectoryEntry& dirEnt)
    {
      RenderTargetPtr thumb = GetThumbnail(dirEnt);
      bool valid            = thumb->m_textureId != 0 && !IsDefaultThumbnail(thumb);
      if (valid)
      {
        iconId = thumb->m_textureId;
      }
      return valid;
    }

    bool ThumbnailManager::Exist(const String& fullPath)
    {
      return m_thumbnailCache.find(fullPath) != m_thumbnailCache.end();
    }

    void ThumbnailManager::UpdateThumbnail(const DirectoryEntry& dirEnt) { CreateRenderTask(dirEnt); }

    void ToolKit::Editor::ThumbnailManager::CreateRenderTask(const DirectoryEntry& dirEnt)
    {
      String fullPath = dirEnt.GetFullPath();
      GetRenderSystem()->AddRenderTask({[this, fullPath, dirEnt](Renderer* renderer) -> void
                                        {
                                          RenderTargetPtr rt         = m_renderer.RenderThumbnail(renderer, dirEnt);
                                          m_thumbnailCache[fullPath] = rt;
                                        },
                                        nullptr,
                                        RenderTaskPriority::Low});
    }

  } // namespace Editor

} // namespace ToolKit
