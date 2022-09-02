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
      explicit OverlayUI(EditorViewport* owner);
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
       explicit OverlayMods(EditorViewport* owner);
      void Show() override;
    };

    class OverlayLighting : public OverlayUI
    {
     public:
      explicit OverlayLighting(EditorViewport* owner);
      void Show() override;
    };

    class OverlayViewportOptions : public OverlayUI
    {
     public:
      explicit OverlayViewportOptions(EditorViewport* owner);
      void Show() override;
    };

    class Overlay2DViewportOptions : public OverlayUI
    {
     public:
       explicit Overlay2DViewportOptions(EditorViewport* owner);
       void Show() override;
    };

    class StatusBar : public OverlayUI
    {
     public:
      explicit StatusBar(EditorViewport* owner);
      void Show() override;
    };
  }  //  namespace Editor
}  //  namespace ToolKit
