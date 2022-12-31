#pragma once

#include "FrameBuffer.h"
#include "SceneRenderer.h"

namespace ToolKit
{
  namespace Editor
  {

    class ThumbnailRenderer : public SceneRenderer
    {
     public:
      ThumbnailRenderer();
      ~ThumbnailRenderer();
      RenderTargetPtr RenderThumbnail(const String& fullPath);

     private:
      void Render(Renderer* renderer) override;
      void PreRender();

     private:
      FramebufferPtr m_thumbnailBuffer = nullptr;
      RenderTargetPtr m_thumbnailRT    = nullptr;
      ScenePtr m_thumbnailScene        = nullptr;
      float m_maxThumbSize             = 300.0f;
      SpherePtr m_sphere               = nullptr;
      CameraPtr m_cam                  = nullptr;
    };

  } // namespace Editor
} // namespace ToolKit