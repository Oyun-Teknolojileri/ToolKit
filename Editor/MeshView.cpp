#include "MeshView.h"

#include "App.h"

#include <DirectionComponent.h>

namespace ToolKit
{
  namespace Editor
  {
    MeshView::MeshView() : View("Mesh View")
    {
      m_viewID   = 3;
      m_viewIcn  = UI::m_meshIcon;
      m_viewport = new PreviewViewport(300, 300);

      Entity* previewEntity = new Entity;
      previewEntity->AddComponent(std::make_shared<MeshComponent>());
      m_viewport->GetScene()->AddEntity(previewEntity);
    }
    MeshView::~MeshView()
    {
      SafeDel(m_viewport);
    }

    void MeshView::ResetCamera()
    {
      m_viewport->GetCamera()->m_node->SetTranslation(Vec3(0, 2.0, 5));
      m_viewport->GetCamera()->GetComponent<DirectionComponent>()->LookAt(
          Vec3(0));
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

      if (ImGui::CollapsingHeader("Mesh Preview",
                                  ImGuiTreeNodeFlags_DefaultOpen))
      {
        if (UI::ImageButtonDecorless(
                UI::m_cameraIcon->m_textureId, Vec2(16.0f), false))
        {
          ResetCamera();
        }
        ImGui::SameLine();
        m_viewport->Update(g_app->GetDeltaTime());
        m_viewport->Show();
      }

      if (ImGui::CollapsingHeader("Mesh Info", ImGuiTreeNodeFlags_DefaultOpen))
      {
        ImGui::Text("Vertex Count: %u\nIndex Count: %u",
                    m_mesh->m_vertexCount,
                    m_mesh->m_indexCount);
        DropZone(UI::m_materialIcon->m_textureId,
                 m_mesh->m_material->GetFile(),
                 [this](const DirectoryEntry& entry) {
                   g_app->m_statusMsg =
                       "You can't change mesh's default material";
                 });
      }
    }

    void MeshView::SetMesh(MeshPtr mesh)
    {
      m_mesh = mesh;
      m_viewport->GetScene()->GetEntities()[0]->GetMeshComponent()->SetMeshVal(
          m_mesh);
      BoundingBox aabb;
      for (Entity* ntt : m_viewport->GetScene()->GetEntities())
      {
        aabb.UpdateBoundary(ntt->GetAABB(true).min);
        aabb.UpdateBoundary(ntt->GetAABB(true).max);
      }
      m_viewport->GetCamera()->FocusToBoundingBox(aabb, -1.0f);
    }
  } // namespace Editor
} // namespace ToolKit