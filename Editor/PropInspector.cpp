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
          
          static String buffer;
          ImGui::InputText("Tag", &buffer);
        }

        if (ImGui::CollapsingHeader("Transforms"))
        {
          Mat3 rotate;
          Vec3 scale, shear;
          Mat4 ts = curr->m_node->GetTransform(g_app->m_transformOrientation);
          QDUDecomposition(ts, rotate, scale, shear);

          Vec3 eularXYZ;
          glm::extractEulerAngleXYZ<float>(rotate, eularXYZ.x, eularXYZ.y, eularXYZ.z);

          bool setTs = false;
          Vec3 translate = ts[3];
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

          if (ImGui::DragFloat3("Scale", &scale[0], 0.01f, 0.0001f))
          {
            setTs = true;
          }

          if (setTs)
          {
            Mat4 ts, scl;
            ts[3].xyz = translate;
            Quaternion x = glm::angleAxis(eularXYZ.x, X_AXIS);
            Quaternion y = glm::angleAxis(eularXYZ.y, Y_AXIS);
            Quaternion z = glm::angleAxis(eularXYZ.z, Z_AXIS);
            Mat4 rt = glm::toMat4(x * y * z);
            //Mat4 rt = glm::eulerAngleXYZ(eularXYZ.x, eularXYZ.y, eularXYZ.z);
            scl = glm::scale(scl, scale);

            curr->m_node->SetTransform(ts * rt * scl, g_app->m_transformOrientation);
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

