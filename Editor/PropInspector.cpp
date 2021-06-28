#include "stdafx.h"
#include "PropInspector.h"
#include "GlobalDef.h"

#include "ImGui/imgui_stdlib.h"
#include "ConsoleWindow.h"
#include "TransformMod.h"

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    // View
    //////////////////////////////////////////////////////////////////////////
    
    void View::DropZone(uint fallbackIcon, const String& file, std::function<void(Entity*)> dropAction)
    {
      DirectoryEntry dirEnt;
      g_app->GetAssetBrowser()->GetFileEntry(file, dirEnt);
      uint iconId = fallbackIcon;

      ImVec2 texCoords = ImVec2(1.0f, 1.0f);
      if (dirEnt.m_thumbNail)
      {
        texCoords = ImVec2(1.0f, -1.0f);
        iconId = dirEnt.m_thumbNail->m_textureId;
      }

      if (ImGui::BeginTable("##DropZone", 2))
      {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::Image((void*)(intptr_t)iconId, ImVec2(48.0f, 48.0f), ImVec2(0.0f, 0.0f), texCoords);
        UI::HelpMarker(LOC + file, "Drop zone", 0.1f);
        ImGui::TableNextColumn();

        String fullPath = dirEnt.m_rootPath + GetPathSeparatorAsStr() + dirEnt.m_fileName + dirEnt.m_ext;
        ImGui::Text(fullPath.c_str());
        UI::HelpMarker(LOC + file, fullPath.c_str(), 0.1f);

        ImGui::EndTable();
      }
    }

    // AssetView
    //////////////////////////////////////////////////////////////////////////

    void AssetView::Show()
    {
      if (ImGui::CollapsingHeader("Asset", ImGuiTreeNodeFlags_DefaultOpen))
      {
        String fullPath = m_entry.m_rootPath + GetPathSeparatorAsStr() + m_entry.m_fileName + m_entry.m_ext;
        ImGui::Text(fullPath.c_str());
        UI::HelpMarker(LOC, fullPath.c_str(), 0.1f);

        if (ImGui::Button("Reload"))
        {
          Resource* res = nullptr;
          if (m_entry.m_ext == MESH)
          {
            res =  GetMeshManager()->Create(fullPath).get();
          }
          else if (m_entry.m_ext == MATERIAL)
          {
            res = GetMaterialManager()->Create(fullPath).get();
          }
          else if (SupportedImageFormat(m_entry.m_ext))
          {
            res = GetTextureManager()->Create(fullPath).get();
          }
            
          if (res)
          {
            res->UnInit();
            res->m_loaded = false;
            res->Load();
          }
        }
      }
    }

    // EntityView
    //////////////////////////////////////////////////////////////////////////

    void EntityView::Show()
    {
      if (m_entry == nullptr)
      {
        return;
      }

      // Entity View
      if (ImGui::CollapsingHeader("Basics"))
      {
        ImGui::InputText("Name", &m_entry->m_name);
        ImGui::InputText("Tag", &m_entry->m_tag);
      }

      if (ImGui::CollapsingHeader("Transforms"))
      {
        Mat3 rotate;
        Vec3 scale, shear;
        Mat4 ts = m_entry->m_node->GetTransform(g_app->m_transformSpace);
        QDUDecomposition(ts, rotate, scale, shear);

        static TransformAction* dragMem = nullptr;
        const auto saveDragMemFn = [this]()
        {
          if (dragMem == nullptr)
          {
            dragMem = new TransformAction(m_entry);
          }
        };

        TransformationSpace space = g_app->m_transformSpace;
        Vec3 translate = glm::column(ts, 3);
        if
          (
            ImGui::DragFloat3("Translate", &translate[0], 0.25f) &&
            (
              ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.1f) ||
              ImGui::IsItemDeactivatedAfterEdit()
            )
          )
        {
          if (ImGui::IsMouseDragging(0, 0.25f))
          {
            saveDragMemFn();
          }
          else
          {
            ActionManager::GetInstance()->AddAction(new TransformAction(m_entry));
          }

          m_entry->m_node->SetTranslation(translate, space);
        }

        Quaternion q0 = glm::toQuat(rotate);
        Vec3 eularXYZ = glm::eulerAngles(q0);
        Vec3 degrees = glm::degrees(eularXYZ);
        if
          (
            ImGui::DragFloat3("Rotate", &degrees[0], 0.25f) &&
            (
              ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.1f) ||
              ImGui::IsItemDeactivatedAfterEdit()
            )
          )
        {
          Vec3 eular = glm::radians(degrees);
          Vec3 change = eular - eularXYZ;

          bool isDrag = ImGui::IsMouseDragging(0, 0.25f);
          if (!isDrag)
          {
            change = eular;
          }

          Quaternion qx = glm::angleAxis(change.x, X_AXIS);
          Quaternion qy = glm::angleAxis(change.y, Y_AXIS);
          Quaternion qz = glm::angleAxis(change.z, Z_AXIS);
          Quaternion q = qz * qy * qx;

          if (isDrag)
          {
            saveDragMemFn();
            m_entry->m_node->Rotate(q, space);
          }
          else
          {
            ActionManager::GetInstance()->AddAction(new TransformAction(m_entry));
            m_entry->m_node->SetOrientation(q, space);
          }
        }

        scale = m_entry->m_node->GetScale();
        if
          (
            ImGui::DragFloat3("Scale", &scale[0], 0.25f) &&
            (
              ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.1f) ||
              ImGui::IsItemDeactivatedAfterEdit()
            )
          )
        {
          if (ImGui::IsMouseDragging(0, 0.25f))
          {
            saveDragMemFn();
          }
          else
          {
            ActionManager::GetInstance()->AddAction(new TransformAction(m_entry));
          }
          m_entry->m_node->SetScale(scale);
        }

        if (ImGui::Checkbox("Inherit Scale", &m_entry->m_node->m_inheritScale))
        {
          m_entry->m_node->SetInheritScaleDeep(m_entry->m_node->m_inheritScale);
        }

        if (dragMem && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
          ActionManager::GetInstance()->AddAction(dragMem);
          dragMem = nullptr;
        }
      }
    }

    // MeshView
    //////////////////////////////////////////////////////////////////////////

    void MeshView::Show()
    {
      if (m_entry)
      {
        if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen))
        {
          if (ImGui::BeginTable("##MeshStats", 2))
          {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            ImGui::Text("Face count:");
            ImGui::TableNextColumn();
            ImGui::Text("%d", m_entry->m_faces.size());
           
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            ImGui::Text("Vertex count:");
            ImGui::TableNextColumn();
            ImGui::Text("%d", m_entry->m_clientSideVertices.size());

            ImGui::EndTable();
          }

          DropZone(UI::m_meshIcon->m_textureId, m_entry->m_file, [](Entity* e) -> void {});
        }

      }
    }

    // MaterialView
    //////////////////////////////////////////////////////////////////////////

    void MaterialView::Show()
    {
      if (m_entry)
      {
        if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
        {
          ImGui::ColorEdit3("MatColor##1", (float*)&m_entry->m_color);
          if (ImGui::TreeNode("Textures"))
          {
            ImGui::LabelText("##diffTexture", "Diffuse Texture: ");
            DropZone(UI::m_imageIcon->m_textureId, m_entry->m_file, [](Entity* e) -> void {});
            ImGui::TreePop();
          }

          if (ImGui::TreeNode("Shaders"))
          {
            ImGui::LabelText("##vertShader", "Vertex Shader: ");
            DropZone(UI::m_codeIcon->m_textureId, m_entry->m_vertexShader->m_file, [](Entity* e) -> void {});

            ImGui::LabelText("##fragShader", "Fragment Shader: ");
            DropZone(UI::m_codeIcon->m_textureId, m_entry->m_fragmetShader->m_file, [](Entity* e) -> void {});
            ImGui::TreePop();
          }
        }
      }
    }

    // PropInspector
    //////////////////////////////////////////////////////////////////////////

    PropInspector::PropInspector()
    {
      m_views.push_back(new EntityView());
      m_views.push_back(new MaterialView());
      m_views.push_back(new MeshView());
      m_views.push_back(new AssetView());
    }

    PropInspector::~PropInspector()
    {
      for (View* v : m_views)
      {
        SafeDel(v);
      }
      m_views.clear();
    }

    void PropInspector::Show()
    {
      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        Entity* curr = g_app->m_scene.GetCurrentSelection();
        if (curr == nullptr)
        {
          ImGui::Text("Select an Entity");
        }
        else
        {
          EntityView* ev = GetView<EntityView>();
          ev->m_entry = curr;
          ev->Show();

          if (curr->IsDrawable())
          {
            Drawable* dw = static_cast<Drawable*> (curr);
            MeshRawPtrArray meshes;
            dw->m_mesh->GetAllMeshes(meshes);
            for (size_t i = 0; i < meshes.size(); i++)
            {
              Mesh* m = meshes[i];
              MeshView* mev = GetView<MeshView>();
              mev->m_entry = m;
              mev->Show();

              MaterialView* mav = GetView <MaterialView>();
              mav->m_entry = m->m_material.get();
              mav->Show();

              if (i > 1 && i != meshes.size() - 1)
              {
                ImGui::Separator();
              }
            }
          }
        }

        ImGui::Separator();

        if (AssetView* av = GetView <AssetView>())
        {
          av->Show();
        }
      }
      ImGui::End();
    }

    Window::Type PropInspector::GetType() const
    {
      return Window::Type::Inspector;
    }

    void PropInspector::DispatchSignals() const
    {
    }

  }
}

