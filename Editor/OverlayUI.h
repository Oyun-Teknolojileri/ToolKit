#pragma once

#include "ToolKit.h"
#include "UI.h"

namespace ToolKit
{
  namespace Editor
  {
    class EditorViewport;

    class OverlayUI
    {
    public:
      OverlayUI(EditorViewport* owner);
      virtual ~OverlayUI();
      virtual void Show() = 0;

      void SetOwnerState();

    public:
      EditorViewport* m_owner;
      Vec2 m_scroll;
    };

    class OverlayMods : public OverlayUI
    {
    public:
      OverlayMods(EditorViewport* owner);
      virtual void Show() override;
    };

    class OverlayViewportOptions : public OverlayUI
    {
    public:
      OverlayViewportOptions(EditorViewport* owner);
      virtual void Show() override;
    };

    class StatusBar : public OverlayUI
    {
    public:
      StatusBar(EditorViewport* owner);
      virtual void Show() override;
    };

  }
}
