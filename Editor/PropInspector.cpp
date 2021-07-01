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
    
    void View::DropZone(uint fallbackIcon, const String& file, std::function<void(const DirectoryEntry& entry)> dropAction)
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

        if (ImGui::BeginDragDropTarget())
        {
          if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BrowserDragZone"))
          {
            IM_ASSERT(payload->DataSize == sizeof(DirectoryEntry));
            DirectoryEntry entry = *(const DirectoryEntry*)payload->Data;
            dropAction(entry);
          }

          ImGui::EndDragDropTarget();
        }

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
            res = GetMeshManager()->Create(fullPath).get();
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
      if (m_entity == nullptr)
      {
        return;
      }

      // Entity View
      if (ImGui::CollapsingHeader("Basics"))
      {
        ImGui::InputText("Name", &m_entity->m_name);
        ImGui::InputText("Tag", &m_entity->m_tag);
      }

      if (ImGui::CollapsingHeader("Transforms"))
      {
        Mat3 rotate;
        Vec3 scale, shear;
        Mat4 ts = m_entity->m_node->GetTransform(g_app->m_transformSpace);
        QDUDecomposition(ts, rotate, scale, shear);

        static TransformAction* dragMem = nullptr;
        const auto saveDragMemFn = [this]()
        {
          if (dragMem == nullptr)
          {
            dragMem = new TransformAction(m_entity);
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
            ActionManager::GetInstance()->AddAction(new TransformAction(m_entity));
          }

          m_entity->m_node->SetTranslation(translate, space);
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
            m_entity->m_node->Rotate(q, space);
          }
          else
          {
            ActionManager::GetInstance()->AddAction(new TransformAction(m_entity));
            m_entity->m_node->SetOrientation(q, space);
          }
        }

        scale = m_entity->m_node->GetScale();
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
            ActionManager::GetInstance()->AddAction(new TransformAction(m_entity));
          }
          m_entity->m_node->SetScale(scale);
        }

        if (ImGui::Checkbox("Inherit Scale", &m_entity->m_node->m_inheritScale))
        {
          m_entity->m_node->SetInheritScaleDeep(m_entity->m_node->m_inheritScale);
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

          DropZone(UI::m_meshIcon->m_textureId, m_entry->m_file, [](const DirectoryEntry& entry) -> void {});
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
            String target = "\\";
            if (m_entry->m_diffuseTexture)
            {
              target = m_entry->m_diffuseTexture->m_file;
            }

            DropZone
            (
              UI::m_imageIcon->m_textureId, 
              target, 
              [this](const DirectoryEntry& entry) -> void
              {
                // Switch from solid color material to default for texturing.
                if (m_mesh && m_entry->m_diffuseTexture == nullptr)
                {
                  m_mesh->m_material = GetMaterialManager()->GetCopyOfDefaultMaterial();
                  m_entry = m_mesh->m_material.get();
                }
                m_entry->m_diffuseTexture = GetTextureManager()->Create(entry.GetFullPath());
              }
            );

            ImGui::TreePop();
          }

          if (ImGui::TreeNode("Shaders"))
          {
            ImGui::LabelText("##vertShader", "Vertex Shader: ");
            DropZone
            (
              UI::m_codeIcon->m_textureId,
              m_entry->m_vertexShader->m_file,
              [this](const DirectoryEntry& entry) -> void 
              {
                m_entry->m_vertexShader = GetShaderManager()->Create(entry.GetFullPath());
                m_entry->m_vertexShader->Init();
              }
            );

            ImGui::LabelText("##fragShader", "Fragment Shader: ");
            DropZone
            (
              UI::m_codeIcon->m_textureId,
              m_entry->m_fragmetShader->m_file,
              [this](const DirectoryEntry& entry) -> void 
              {
                m_entry->m_fragmetShader = GetShaderManager()->Create(entry.GetFullPath());
                m_entry->m_fragmetShader->Init();
              }
            );
            ImGui::TreePop();
          }

          if (ImGui::TreeNode("Render State"))
          {
            int cullMode = (int)m_entry->GetRenderState()->cullMode;
            if (ImGui::Combo("Cull mode", &cullMode, "Two Sided\0Front\0Back"))
            {
              m_entry->GetRenderState()->cullMode = (CullingType)cullMode;
            }

            int blendMode = (int)m_entry->GetRenderState()->blendFunction;
            if (ImGui::Combo("Blend mode", &blendMode, "None\0Alpha Blending"))
            {
              m_entry->GetRenderState()->blendFunction = (BlendFunction)blendMode;
            }

            int drawType = -1;
            switch (m_entry->GetRenderState()->drawType)
            {
            case DrawType::Triangle:
              drawType = 0;
              break;
            case DrawType::Line:
              drawType = 1;
              break;
            case DrawType::LineStrip:
              drawType = 2;
              break;
            case DrawType::LineLoop:
              drawType = 3;
              break;
            case DrawType::Point:
              drawType = 4;
              break;
            }

            if (ImGui::Combo("Draw mode", &drawType, "Triangle\0Line\0Line Strip\0Line Loop\0Point"))
            {
              switch (drawType)
              {
              case 0:
                m_entry->GetRenderState()->drawType = DrawType::Triangle;
                break;
              case 1:
                m_entry->GetRenderState()->drawType = DrawType::Line;
                break;
              case 2:
                m_entry->GetRenderState()->drawType = DrawType::LineStrip;
                break;
              case 3:
                m_entry->GetRenderState()->drawType = DrawType::LineLoop;
                break;
              case 4:
                m_entry->GetRenderState()->drawType = DrawType::Point;
                break;
              }
            }

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
          ev->m_entity = curr;
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
              mev->m_entity = curr;
              mev->Show();

              MaterialView* mav = GetView <MaterialView>();
              mav->m_entry = m->m_material.get();
              mav->m_mesh = m;
              mav->m_entity = curr;
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

