#include "Thumbnail.h"

#include "Entity.h"

#include "DebugNew.h"

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

      m_entity         = std::make_shared<Entity>();
      m_entity->AddComponent(new MeshComponent());

      m_sphere = std::make_shared<Sphere>();
      m_sphere->AddComponent(new MaterialComponent());

      m_lightSystem = std::make_shared<ThreePointLightSystem>();
      m_cam         = std::make_shared<Camera>();
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

    RenderTargetPtr ThumbnailRenderer::RenderThumbnail(
        Renderer* renderer,
        const DirectoryEntry& dirEnt)
    {
      String fullpath = dirEnt.GetFullPath();
      m_thumbnailScene->ClearEntities();

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
          SkeletonComponentPtr skelComp = std::make_shared<SkeletonComponent>();
          skelComp->SetSkeletonResourceVal(skinMesh->m_skeleton);
          skelComp->Init();
          m_entity->AddComponent(skelComp);
        }

        m_thumbnailScene->AddEntity(m_entity.get());

        m_cam->SetLens(glm::half_pi<float>(), 1.0f);
        m_cam->FocusToBoundingBox(m_entity->GetAABB(true), 1.5f);
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
        m_cam->m_node->SetOrientation(Quaternion());
        m_cam->m_node->SetTranslation(Vec3(0.0f, 0.0f, 1.5f));
      }
      else if (SupportedImageFormat(dirEnt.m_ext) || dirEnt.m_ext == HDR)
      {
        TexturePtr texture = nullptr;
        if (dirEnt.m_ext == HDR)
        {
          TextureSettings ts;
          ts.InternalFormat = GraphicTypes::FormatRGB16F;
          ts.GenerateMipMap = false;
          texture           = std::make_shared<Texture>(fullpath, ts);
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
        m_surface->UpdateGeometry(false);
        MaterialComponentPtr matCom = m_surface->GetMaterialComponent();
        matCom->GetMaterialVal()->m_diffuseTexture = texture;
        matCom->Init(false);

        m_thumbnailScene->AddEntity(m_surface.get());
        m_cam->m_orthographicScale = 1.0f;
        m_cam
            ->SetLens(w * -0.5f, w * 0.5f, h * -0.5f, h * 0.5f, 0.01f, 1000.0f);

        m_cam->m_node->SetOrientation(Quaternion());
        m_cam->m_node->SetTranslation(Vec3(0.0f, 0.0f, 10.0f),
                                      TransformationSpace::TS_LOCAL);
      }

      m_thumbnailRT =
          std::make_shared<RenderTarget>(m_maxThumbSize, m_maxThumbSize);
      m_thumbnailRT->Init();

      m_thumbnailBuffer->SetAttachment(
          Framebuffer::Attachment::ColorAttachment0,
          m_thumbnailRT);

      Mat4 camTs = m_cam->m_node->GetTransform();
      m_lightSystem->m_parentNode->SetTransform(camTs);

      m_params.Lights          = m_lightSystem->m_lights;
      m_params.Scene           = m_thumbnailScene;
      m_params.Cam             = m_cam.get();
      m_params.MainFramebuffer = m_thumbnailBuffer;

      Render(renderer);

      return m_thumbnailRT;
    }

    ThumbnailManager::ThumbnailManager()
    {
      m_defaultThumbnail = std::make_shared<RenderTarget>(10u, 10u);
    }

    ThumbnailManager::~ThumbnailManager() { m_defaultThumbnail = nullptr; }

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

    bool ThumbnailManager::Exist(const String& fullPath)
    {
      return m_thumbnailCache.find(fullPath) != m_thumbnailCache.end();
    }

    void ThumbnailManager::UpdateThumbnail(const DirectoryEntry& dirEnt)
    {
      CreateRenderTask(dirEnt);
    }

    void ToolKit::Editor::ThumbnailManager::CreateRenderTask(
        const DirectoryEntry& dirEnt)
    {
      String fullPath = dirEnt.GetFullPath();
      GetRenderSystem()->AddRenderTask(
          {[this, fullPath, dirEnt](Renderer* renderer) -> void
           {
             RenderTargetPtr rt = m_renderer.RenderThumbnail(renderer, dirEnt);
             m_thumbnailCache[fullPath] = rt;
           },
           nullptr,
           RenderTaskPriority::Low});
    }

  } // namespace Editor

} // namespace ToolKit
