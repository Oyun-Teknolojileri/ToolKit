#include "Dpad.h"

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
    // Update
    const Vec3 scale = m_node->GetScale();
    if (!VecAllEqual(m_lastScale, scale))
    {
      const float minScaleFactor = glm::min(scale.x, glm::min(scale.y, scale.z));
      const float maxScaleFactor = glm::max(scale.x, glm::max(scale.y, scale.z));
      const float scaleFactor    = (minScaleFactor + maxScaleFactor) * 0.5f;
      m_activeDpadRadius         = GetDpadRadiusVal() * scaleFactor;
    }

    const Vec3 pos = m_node->GetTranslation();
    m_deltaXY.x    = mouseXY.x - pos.x;
    m_deltaXY.y    = mouseXY.y - pos.y;
    m_deltaXY      /= m_activeDpadRadius;

    if (fabs(m_deltaXY.x) > m_activeDpadRadius || fabs(m_deltaXY.y) > m_activeDpadRadius)
    {
      m_deltaXY.x = 0.0f;
      m_deltaXY.y = 0.0f;
    }
  }
} // namespace ToolKit
