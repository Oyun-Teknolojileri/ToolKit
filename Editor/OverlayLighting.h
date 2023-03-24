#pragma once
#include "OverlayUI.h"

namespace ToolKit
{
  namespace Editor
  {

    class OverlayLighting : public OverlayUI
    {
     public:
      explicit OverlayLighting(EditorViewport* owner);
      void Show() override;

     private:
      bool m_editorLitModeOn = true;
    };

  } // namespace Editor
} // namespace ToolKit
