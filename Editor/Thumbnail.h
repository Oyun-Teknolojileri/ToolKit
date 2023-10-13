/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "EditorLight.h"
#include "FolderWindow.h"
#include "SceneRenderPath.h"

namespace ToolKit
{
  namespace Editor
  {

    class ThumbnailRenderer : private SceneRenderPath
    {
     public:
      ThumbnailRenderer();
      ~ThumbnailRenderer();

      RenderTargetPtr RenderThumbnail(Renderer* renderer, const DirectoryEntry& dirEnt);

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
      void CreateRenderTask(const DirectoryEntry& dirEnt);

     private:
      ThumbnailRenderer m_renderer;
      RenderTargetPtr m_defaultThumbnail = nullptr;
      TKMap<String, RenderTargetPtr> m_thumbnailCache;
    };

  } // namespace Editor
} // namespace ToolKit