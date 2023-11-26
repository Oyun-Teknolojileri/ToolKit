/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "MeshView.h"

#include "App.h"

#include <Camera.h>
#include <DirectionComponent.h>
#include <Material.h>
#include <Mesh.h>

namespace ToolKit
{
  namespace Editor
  {
    MeshView::MeshView() : View("Mesh View")
    {
      m_viewID   = 3;
      m_viewIcn  = UI::m_meshIcon;
      m_viewport = new PreviewViewport();
      m_viewport->Init({300.0f, 300.0f});

      ScenePtr scene  = GetSceneManager()->Create<Scene>(ScenePath("mesh-view.scene", true));
      m_previewEntity = scene->GetFirstByTag("target");

      m_viewport->SetScene(scene);
    }

    MeshView::~MeshView() { SafeDel(m_viewport); }

    void MeshView::ResetCamera()
    {
      m_viewport->GetCamera()->m_node->SetTranslation(Vec3(0, 2.0, 5));
      m_viewport->GetCamera()->GetComponent<DirectionComponent>()->LookAt(Vec3(0));
    }

    void MeshView::Show()
    {
      if (!m_mesh)
      {
        ImGui::Text("Select a mesh");
        return;
      }

      String name, ext;
      DecomposePath(m_mesh->GetFile(), nullptr, &name, &ext);

      ImGui::Text("Mesh: %s%s", name.c_str(), ext.c_str());
      ImGui::Separator();

      if (ImGui::CollapsingHeader("Mesh Preview", ImGuiTreeNodeFlags_DefaultOpen))
      {
        if (UI::ImageButtonDecorless(UI::m_cameraIcon->m_textureId, Vec2(16.0f), false))
        {
          ResetCamera();
        }
        ImGui::SameLine();
        m_viewport->Update(g_app->GetDeltaTime());
        m_viewport->Show();
      }

      if (ImGui::CollapsingHeader("Mesh Info", ImGuiTreeNodeFlags_DefaultOpen))
      {
        MeshRawCPtrArray submeshes;
        m_mesh->GetAllMeshes(submeshes);
        for (uint i = 0; i < submeshes.size(); i++)
        {
          ImGui::PushID(i);
          const Mesh* submesh = submeshes[i];
          ImGui::Text("Submesh Index: %u\nVertex Count: %u\nIndex Count: %u",
                      i,
                      submesh->m_vertexCount,
                      submesh->m_indexCount);
          DropZone(UI::m_materialIcon->m_textureId,
                   submesh->m_material->GetFile(),
                   [this](const DirectoryEntry& entry)
                   {
                     g_app->m_statusMsg = "Failed.";

                     TK_WRN("You can't change mesh's default material.");
                   });
          if (i < submeshes.size() - 1)
          {
            ImGui::Separator();
          }
          ImGui::PopID();
        }
      }
    }

    void MeshView::SetMesh(MeshPtr mesh)
    {
      m_mesh = mesh;
      m_previewEntity->GetMeshComponent()->SetMeshVal(m_mesh);

      if (m_mesh->IsSkinned())
      {
        SkinMeshPtr skinMesh          = std::static_pointer_cast<SkinMesh>(m_mesh);
        SkeletonComponentPtr skelComp = m_previewEntity->GetComponent<SkeletonComponent>();
        skelComp->SetSkeletonResourceVal(skinMesh->m_skeleton);
        skelComp->Init();
      }

      BoundingBox aabb;
      EntityPtrArray entities = m_viewport->GetScene()->GetEntities();
      for (EntityPtr ntt : entities)
      {
        aabb.UpdateBoundary(ntt->GetAABB(true).min);
        aabb.UpdateBoundary(ntt->GetAABB(true).max);
      }

      m_viewport->GetCamera()->FocusToBoundingBox(aabb, 1.0f);
    }

  } // namespace Editor
} // namespace ToolKit