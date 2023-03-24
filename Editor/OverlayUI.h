#pragma once

#include "ToolKit.h"
#include "UI.h"
#include "EditorViewport.h"

namespace ToolKit
{
  namespace Editor
  {

    class OverlayUI
    {
     public:
      explicit OverlayUI(EditorViewport* owner);
      virtual ~OverlayUI();
      virtual void Show() = 0;

      void SetOwnerState();

     public:
      EditorViewport* m_owner;
    };

  } //  namespace Editor
} //  namespace ToolKit
