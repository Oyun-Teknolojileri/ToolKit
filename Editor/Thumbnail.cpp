#include "Thumbnail.h"

#include "Entity.h"
#include "stdafx.h"

namespace ToolKit
{

  namespace Editor
  {
    ThumbnailRenderer::ThumbnailRenderer()
    {
      m_maxThumbSize    = 300u;

      m_thumbnailBuffer = std::make_shared<Framebuffer>();
      m_thumbnailBuffer->Init({m_maxThumbSize, m_maxThumbSize, false, true});

      m_thumbnailScene = std::make_shared<Scene>();

      m_entity = std::make_shared<Entity>();
      m_entity->AddComponent(new MeshComponent());

      m_sphere = std::make_shared<Sphere>();
      m_sphere->AddComponent(new MaterialComponent());

      m_lights = std::make_shared<ThreePointLightSystem>();
      m_cam    = std::make_shared<Camera>();
    }

    ThumbnailRenderer::~ThumbnailRenderer()
    {
      m_thumbnailBuffer = nullptr;
      m_thumbnailRT     = nullptr;
      m_thumbnailScene  = nullptr;
      m_sphere          = nullptr;
      m_cam             = nullptr;
      m_entity          = nullptr;
    }

    RenderTargetPtr ThumbnailRenderer::RenderThumbnail(
        const DirectoryEntry& dirEnt)
    {
      String fullpath = dirEnt.GetFullPath();

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
          SkeletonComponentPtr skelComp = std::make_shared<SkeletonComponent>();
          skelComp->SetSkeletonResourceVal(skinMesh->m_skeleton);
          skelComp->Init();
          m_entity->AddComponent(skelComp);
        }

        m_thumbnailScene->AddEntity(m_entity.get());

        m_cam->SetLens(glm::half_pi<float>(), 1.0f);
        m_cam->FocusToBoundingBox(meshComp->GetAABB(), 1.1f);
      }
      else if (dirEnt.m_ext == MATERIAL)
      {
        MaterialPtr mat = GetMaterialManager()->Create<Material>(fullpath);

        // Disable ao
        bool aoActive   = mat->GetRenderState()->AOInUse;
        mat->GetRenderState()->AOInUse  = false;
        mat->GetRenderState()->IBLInUse = false;

        if (MaterialComponentPtr mc = m_sphere->GetMaterialComponent())
        {
          mc->SetMaterialVal(mat);
        }

        m_thumbnailScene->AddEntity(m_sphere.get());

        m_cam->SetLens(glm::half_pi<float>(), 1.0f);
        m_cam->m_node->SetTranslation(Vec3(0.0f, 0.0f, 1.5f));
      }
      else if (SupportedImageFormat(dirEnt.m_ext) || dirEnt.m_ext == HDR)
      {
        TexturePtr texture = nullptr;
        if (dirEnt.m_ext == HDR)
        {
          texture = std::make_shared<Texture>(fullpath, true);
          texture->Load();
          texture->Init(true);
        }
        else
        {
          texture = GetTextureManager()->Create<Texture>(fullpath);
        }

        float maxDim = float(glm::max(texture->m_width, texture->m_height));
        float w      = (texture->m_width / maxDim) * m_maxThumbSize;
        float h      = (texture->m_height / maxDim) * m_maxThumbSize;

        m_surface    = std::make_shared<Surface>(Vec2(w, h));
        MaterialComponentPtr matCom = m_surface->GetMaterialComponent();
        matCom->GetMaterialVal()->m_diffuseTexture = texture;
        matCom->Init(false);

        m_thumbnailScene->AddEntity(m_surface.get());

        m_cam
            ->SetLens(w * -0.5f, w * 0.5f, h * -0.5f, h * 0.5f, 0.01f, 1000.0f);

        m_cam->m_node->SetTranslation(Vec3(0.0f, 0.0f, 10.0f),
                                      TransformationSpace::TS_LOCAL);
      }

      m_thumbnailRT =
          std::make_shared<RenderTarget>(m_maxThumbSize, m_maxThumbSize);

      m_thumbnailBuffer->SetAttachment(
          Framebuffer::Attachment::ColorAttachment0,
          m_thumbnailRT);

      m_params.Scene           = m_thumbnailScene;
      m_params.Cam             = m_cam.get();
      m_params.MainFramebuffer = m_thumbnailBuffer;

      RenderSystem* rsys       = GetRenderSystem();
      rsys->Render(this);

      return m_thumbnailRT;
    }

    RenderTargetPtr ThumbnailManager::GetThumbnail(const DirectoryEntry& dirEnt)
    {
      String fullPath = dirEnt.GetFullPath();
      if (!Exist(fullPath))
      {
        RenderTargetPtr rt         = m_renderer.RenderThumbnail(dirEnt);
        m_thumbnailCache[fullPath] = rt;
      }

      return m_thumbnailCache[fullPath];
    }

    bool ThumbnailManager::Exist(const String& fullPath)
    {
      return m_thumbnailCache.find(fullPath) != m_thumbnailCache.end();
    }

    void ThumbnailManager::UpdateThumbnail(const DirectoryEntry& dirEnt)
    {
      String fullPath            = dirEnt.GetFullPath();
      m_thumbnailCache[fullPath] = m_renderer.RenderThumbnail(dirEnt);
    }

  } // namespace Editor

} // namespace ToolKit
