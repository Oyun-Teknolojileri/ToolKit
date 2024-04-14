/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Dpad.h"

#include "Material.h"
#include "MathUtil.h"



namespace ToolKit
{

  TKDefineClass(Dpad, Surface);

  Dpad::Dpad() {}

  Dpad::~Dpad() {}

  void Dpad::ParameterConstructor()
  {
    Super::ParameterConstructor();

    DpadRadius_Define(100.0f, DpadCategory.Name, DpadCategory.Priority, true, true);
  }

  void Dpad::UpdateDpad(const Vec2& mouseXY)
  {
    if (!m_active)
    {
      return;
    }

    // Update
    const Vec3 scale = m_node->GetScale();
    if (!VecAllEqual(m_lastScale, scale))
    {
      const float maxScaleFactor = glm::max(scale.x, glm::max(scale.y, scale.z));
      m_activeDpadRadius         = GetDpadRadiusVal() * maxScaleFactor;
    }

    const Vec3 pos = m_node->GetTranslation();
    m_deltaXY.x    = mouseXY.x - pos.x;
    m_deltaXY.y    = mouseXY.y - pos.y;

    if (fabs(m_deltaXY.x) > m_activeDpadRadius || fabs(m_deltaXY.y) > m_activeDpadRadius)
    {
      m_deltaXY.x = 0.0f;
      m_deltaXY.y = 0.0f;
    }
    else
    {
      m_deltaXY /= m_activeDpadRadius;
    }
  }

  void Dpad::ComponentConstructor()
  {
    MaterialComponentPtr matCom = GetComponent<MaterialComponent>();
    if (matCom == nullptr)
    {
      AddComponent<MaterialComponent>();
      MaterialPtr material = GetMaterialManager()->Create<Material>(MaterialPath("dpad.material", true));
      matCom               = GetComponent<MaterialComponent>();
      matCom->SetFirstMaterial(material);
    }
    MeshComponentPtr meshCom = GetComponent<MeshComponent>();
    if (meshCom == nullptr)
    {
      AddComponent<MeshComponent>();
      meshCom                              = GetComponent<MeshComponent>();
      meshCom->ParamMesh().m_exposed       = false;
      meshCom->ParamCastShadow().m_exposed = false;
      meshCom->SetCastShadowVal(false);
    }
  }

  void Dpad::SetDefaultMaterialIfMaterialIsNotOverriden()
  {
    // Re assign default material.
    MaterialComponentPtr matCom = GetMaterialComponent();
    MaterialPtr mat             = matCom->GetFirstMaterial();
    if (mat->IsDynamic() || !mat->_missingFile.empty())
    {
      matCom->SetFirstMaterial(GetMaterialManager()->Create<Material>(MaterialPath("dpad.material", true)));
    }
  }

} // namespace ToolKit
