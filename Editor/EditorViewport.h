#pragma once

#include "ToolKit.h"
#include "Viewport.h"
#include "UI.h"
#include <functional>

namespace ToolKit
{
  class Camera;
  class RenderTarget;

  namespace Editor
  {
    class EditorViewport : public Viewport, public Window
    {
    public:
      EditorViewport(XmlNode* node);
      EditorViewport(float width, float height);
      virtual ~EditorViewport();

      // Window Overrides.
      virtual void Show() override;
      virtual Type GetType() const override;
      virtual void Update(float deltaTime) override;
      bool IsViewportQueriable();
      virtual void DispatchSignals() const override;

      virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
      virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

      virtual void OnResize(float width, float height) override;

    protected:
      // Mods.
      void FpsNavigationMode(float deltaTime);
      void OrbitPanMod(float deltaTime);

    public:
      // Window properties.
      static std::vector<class OverlayUI*> m_overlays;
      bool m_mouseOverOverlay = false;
      int m_cameraAlignment = 0; // 0: perspective, 1: top, 2: front, 3:left.
      int m_additionalWindowFlags = 0;

      // UI Draw commands.
      std::vector<std::function<void(ImDrawList*)>> m_drawCommands;

    private:
      // States.
      bool m_relMouseModBegin = true;
    };

  }
}
