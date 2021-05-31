#include "stdafx.h"
#include "PropInspector.h"
#include "GlobalDef.h"

#include "ImGui/imgui_stdlib.h"

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

          Vec3 eularXYZ;
          glm::extractEulerAngleXYZ<float>(rotate, eularXYZ.x, eularXYZ.y, eularXYZ.z);

          bool setTs = false;
          Vec3 translate = glm::column(ts, 3);
          if (ImGui::DragFloat3("Translate", &translate[0], 0.25f))
          {
            setTs = true;
          }
          
          Vec3 degrees = glm::degrees(eularXYZ);
          if (ImGui::DragFloat3("Rotate", &degrees[0]))
          {
            eularXYZ = glm::radians(degrees);
            setTs = true;
          }

          scale = curr->m_node->GetScale();
          if (ImGui::DragFloat3("Scale", &scale[0], 0.01f, 0.0001f))
          {
            curr->m_node->SetScale(scale);
          }

          if (setTs)
          {
            Mat4 ts;
            ts[3].xyz = translate;
            Mat4 rt = glm::eulerAngleXYZ(eularXYZ.x, eularXYZ.y, eularXYZ.z);
            curr->m_node->SetTransform(ts * rt, g_app->m_transformSpace);
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

