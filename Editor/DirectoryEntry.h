/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

namespace ToolKit
{
  namespace Editor
  {

    class DirectoryEntry
    {
     public:
      DirectoryEntry();
      explicit DirectoryEntry(const String& fullPath);
      String GetFullPath() const;
      ResourceManager* GetManager() const;
      RenderTargetPtr GetThumbnail() const;

     public:
      String m_ext;
      String m_fileName;
      String m_rootPath;
      bool m_isDirectory = false;
      bool m_cutting     = false;

     private:
      MaterialPtr m_tempThumbnailMaterial = nullptr;
    };

  } // namespace Editor
} // namespace ToolKit