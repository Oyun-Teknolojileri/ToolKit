/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "EditorLight.h"
#include "FolderWindow.h"

namespace ToolKit
{
  namespace Editor
  {

    class ThumbnailRenderer : private ForwardSceneRenderPath
    {
     public:
      ThumbnailRenderer();
      virtual ~ThumbnailRenderer();

      /** Renders a thumbnail for given directory entry. Works synchronously. */
      RenderTargetPtr RenderThumbnail(Renderer* renderer, const DirectoryEntry& dirEnt);

     private:
      int m_maxThumbSize;
      FramebufferPtr m_thumbnailBuffer       = nullptr;
      RenderTargetPtr m_thumbnailRT          = nullptr;
      ScenePtr m_thumbnailScene              = nullptr;
      SpherePtr m_sphere                     = nullptr;
      SurfacePtr m_surface                   = nullptr;
      EntityPtr m_entity                     = nullptr;
      CameraPtr m_cam                        = nullptr;
      ThreePointLightSystemPtr m_lightSystem = nullptr;
      GradientSkyPtr m_sky                   = nullptr;
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
       * Creates or retrieve the thumbnail for the given DirectoryEntry.
       * @param iconId icon that you want to replace
       * @param dirEnt DirectoryEntry that will be used to create a thumbnail.
       * @return true if requested thumbnail is valid
       */
      bool TryGetThumbnail(uint& iconId, const DirectoryEntry& dirEnt);

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
      void LoadAsset(const DirectoryEntry& dirEnt);
      void CreateRenderTask(const DirectoryEntry& dirEnt);

     private:
      ThumbnailRenderer m_renderer;
      RenderTargetPtr m_defaultThumbnail = nullptr;
      std::unordered_map<String, RenderTargetPtr> m_thumbnailCache;
    };

  } // namespace Editor
} // namespace ToolKit