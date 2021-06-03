#include "stdafx.h"
#include "PropInspector.h"
#include "GlobalDef.h"

#include "ImGui/imgui_stdlib.h"
#include "ConsoleWindow.h"

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    PropInspector::PropInspector()
    {
    }

    void PropInspector::Show()
    {
      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        Entity* curr = g_app->m_scene.GetCurrentSelection();
        if (curr == nullptr)
        {
          ImGui::Text("Select an Entity");
          ImGui::End();
          return;
        }

        if (ImGui::CollapsingHeader("Basics"))
        {
          ImGui::InputText("Name", &curr->m_name);
          ImGui::InputText("Tag", &curr->m_tag);
        }

        if (ImGui::CollapsingHeader("Transforms"))
        {
          Mat3 rotate;
          Vec3 scale, shear;
          Mat4 ts = curr->m_node->GetTransform(g_app->m_transformSpace);
          QDUDecomposition(ts, rotate, scale, shear);

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
            curr->m_node->SetTranslation(translate, space);
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
              curr->m_node->Rotate(q, space);
            }
            else
            {
              curr->m_node->SetOrientation(q, space);
            }
          }

          scale = curr->m_node->GetScale();
          if
          (
            ImGui::DragFloat3("Scale", &scale[0], 0.25f) &&
            (
              ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.1f) ||
              ImGui::IsItemDeactivatedAfterEdit()
            )
          )
          {
            curr->m_node->SetScale(scale);
          }
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

