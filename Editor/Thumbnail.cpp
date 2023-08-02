/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Thumbnail.h"

#include "App.h"

#include <Camera.h>
#include <Material.h>
#include <Surface.h>

#include <DebugNew.h>

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

      m_entity         = MakeNewPtr<Entity>();
      m_entity->AddComponent<MeshComponent>();

      m_sphere = MakeNewPtr<Sphere>();
      m_sphere->AddComponent<MaterialComponent>();

      m_lightSystem = std::make_shared<ThreePointLightSystem>();
      m_cam         = MakeNewPtr<Camera>();
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

        m_thumbnailScene->AddEntity(m_entity.get());

        m_cam->SetLens(glm::half_pi<float>(), 1.0f);
        m_cam->FocusToBoundingBox(m_entity->GetAABB(true), 1.5f);
      }
      else if (dirEnt.m_ext == MATERIAL)
      {
        MaterialPtr mat = GetMaterialManager()->Create<Material>(fullpath);
        if (MaterialComponentPtr mc = m_sphere->GetMaterialComponent())
        {
          mc->SetFirstMaterial(mat);
        }

        m_thumbnailScene->AddEntity(m_sphere.get());

        m_cam->SetLens(glm::half_pi<float>(), 1.0f);
        m_cam->m_node->SetOrientation(Quaternion());
        m_cam->m_node->SetTranslation(Vec3(0.0f, 0.0f, 1.5f));
      }
      else if (SupportedImageFormat(dirEnt.m_ext))
      {
        TexturePtr texture = nullptr;
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

        m_surface    = MakeNewPtr<Surface>();
        m_surface->Update(Vec2(w, h));
        m_surface->UpdateGeometry(false);
        MaterialComponentPtr matCom                  = m_surface->GetMaterialComponent();
        matCom->GetFirstMaterial()->m_diffuseTexture = texture;
        matCom->Init(false);

        m_thumbnailScene->AddEntity(m_surface.get());
        m_cam->m_orthographicScale = 1.0f;
        m_cam->SetLens(w * -0.5f, w * 0.5f, h * -0.5f, h * 0.5f, 0.01f, 1000.0f);

        m_cam->m_node->SetOrientation(Quaternion());
        m_cam->m_node->SetTranslation(Vec3(0.0f, 0.0f, 10.0f), TransformationSpace::TS_LOCAL);
      }
      else // extension is not recognized, this is probably shader file.
      {
        return g_app->m_thumbnailManager.GetDefaultThumbnail();
      }

      m_thumbnailRT = std::make_shared<RenderTarget>(m_maxThumbSize, m_maxThumbSize);
      m_thumbnailRT->Init();

      m_thumbnailBuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0, m_thumbnailRT);

      Mat4 camTs = m_cam->m_node->GetTransform();
      m_lightSystem->m_parentNode->SetTransform(camTs);

      m_params.Lights          = m_lightSystem->m_lights;
      m_params.Scene           = m_thumbnailScene;
      m_params.Cam             = m_cam.get();
      m_params.MainFramebuffer = m_thumbnailBuffer;

      Render(renderer);

      return m_thumbnailRT;
    }

    ThumbnailManager::ThumbnailManager() { m_defaultThumbnail = std::make_shared<RenderTarget>(10u, 10u); }

    ThumbnailManager::~ThumbnailManager() { m_defaultThumbnail = nullptr; }

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
