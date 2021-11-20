#pragma once

#include "ToolKit.h"
#include "Viewport.h"
#include "UI.h"
#include "EditorViewport.h"

namespace ToolKit
{
  class Camera;
  class RenderTarget;

  namespace Editor
  {

    class EditorViewport2d : public EditorViewport
    {
    public:
      EditorViewport2d(XmlNode* node);
      EditorViewport2d(float width, float height);
      virtual ~EditorViewport2d();

      // Window Overrides.
      virtual void Show() override;
      virtual Type GetType() const override;
      virtual void Update(float deltaTime) override;
      virtual void OnResize(float width, float height) override;

    private:
      void Init2dCam();
      void GetGlobalCanvasSize();
      void PanZoom(float deltaTime);
      void AdjustZoom(float z);

    private:
      Vec2 m_canvasSize;
      float m_zoom;
    };

  }

}
