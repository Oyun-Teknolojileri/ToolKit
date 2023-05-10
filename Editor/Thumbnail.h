#pragma once

#include "EditorLight.h"
#include "FolderWindow.h"
#include "FrameBuffer.h"
#include "SceneRenderer.h"

namespace ToolKit
{
  namespace Editor
  {

    class ThumbnailRenderer : private SceneRenderer
    {
     public:
      ThumbnailRenderer();
      ~ThumbnailRenderer();

      RenderTargetPtr RenderThumbnail(Renderer* renderer,
                                      const DirectoryEntry& dirEnt);

     private:
      uint m_maxThumbSize;
      FramebufferPtr m_thumbnailBuffer       = nullptr;
      RenderTargetPtr m_thumbnailRT          = nullptr;
      ScenePtr m_thumbnailScene              = nullptr;
      SpherePtr m_sphere                     = nullptr;
      SurfacePtr m_surface                   = nullptr;
      EntityPtr m_entity                     = nullptr;
      CameraPtr m_cam                        = nullptr;
      ThreePointLightSystemPtr m_lightSystem = nullptr;
    };

    class ThumbnailManager
    {
     public:
      ThumbnailManager();
      ~ThumbnailManager();

      /**
       * Creates or retrieve the thumbnail for the given DirectoryEntry.
       * @param dirEnt DirectoryEntry that will be used to create a thumbnail.
       * @return dirEnt Created RenderTarget for DirectoryEntry.
       */
      RenderTargetPtr GetThumbnail(const DirectoryEntry& dirEnt);

      /**
       * Checks if a thumbnail exist for given file.
       * @return True if given file exist in the cache.
       */
      bool Exist(const String& fullPath);

      /**
       * Updates the thumbnail for given DirectoryEntry if exist or creates a
       * new thumbnail.
       * @param dirEnt DirectoryEntry that will be used to create a thumbnail.
       */
      void UpdateThumbnail(const DirectoryEntry& dirEnt);

      bool IsDefaultThumbnail(RenderTargetPtr thumb);
      RenderTargetPtr GetDefaultThumbnail();

     private:
      void CreateRenderTask(const DirectoryEntry& dirEnt);

     private:
      ThumbnailRenderer m_renderer;
      RenderTargetPtr m_defaultThumbnail = nullptr;
      std::unordered_map<String, RenderTargetPtr> m_thumbnailCache;
    };

  } // namespace Editor
} // namespace ToolKit