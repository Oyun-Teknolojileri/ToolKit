#pragma once

#include "UI.h"

namespace ToolKit
{
  namespace Editor
  {

    struct DirectoryEntry
    {
      String m_ext;
      String m_fileName;
      String m_rootPath;
      RenderTargetPtr m_thumbNail = nullptr;
      bool m_isDirectory = false;
    };

    class FolderWindow;

    class FolderView
    {
    public:
      FolderView();
      FolderView(FolderWindow* parent);

      void Show();
      void SetPath(const String& path);
      const String& GetPath() const;
      void Iterate();
      int Exist(const String& file);
      void GenerateThumbNail(DirectoryEntry& entry);

    private:
      FolderWindow* m_parent = nullptr;
      String m_path;

    public:
      bool m_currRoot = false; // Indicates this is a root folder (one level under Resources) and currently selected in the FolderWindow.
      bool m_visible = true;
      bool m_onlyNativeTypes = true;
      static Vec2 m_iconSize;
      Vec2 m_thumbnailSize = Vec2(300.0f, 300.0f);
      std::vector<DirectoryEntry> m_entiries;
      String m_folder;
    };

    class FolderWindow : public Window
    {
    public:
      FolderWindow();
      virtual void Show() override;
      virtual Type GetType() const override;
      void Iterate(const String& path);
      void UpdateContent();
      void AddEntry(const FolderView& view);
      FolderView& GetView(int indx);
      int Exist(const String& folder);
      bool GetFileEntry(const String& fullPath, DirectoryEntry& entry);

    private:
      std::vector<FolderView> m_entiries;
      String m_path;
    };

  }
}