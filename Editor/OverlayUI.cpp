#include "OverlayUI.h"

#include "App.h"
#include "OverlayUI.h"

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    OverlayUI::OverlayUI(EditorViewport* owner) { m_owner = owner; }

    OverlayUI::~OverlayUI() {}

    void OverlayUI::SetOwnerState()
    {
      if (m_owner && m_owner->IsActive() && m_owner->IsVisible())
      {
        if (ImGui::IsWindowHovered())
        {
          m_owner->m_mouseOverOverlay = true;
        }
      }
    }

  } // namespace Editor
} // namespace ToolKit
