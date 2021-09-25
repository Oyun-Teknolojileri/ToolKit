#pragma once

#include "UI.h"

namespace ToolKit
{
  namespace Editor
  {

    class DirectoryEntry
    {
    public:
      DirectoryEntry();
      DirectoryEntry(const String& fullPath);
      String GetFullPath() const;
      ResourceManager* GetManager() const;
      void GenerateThumbnail() const;
      RenderTargetPtr GetThumbnail() const;

    public:
      String m_ext;
      String m_fileName;
      String m_rootPath;
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
      void ShowContextMenu(DirectoryEntry* entry = nullptr);

    protected:
      void ShowContextForMaterial(DirectoryEntry* entry);
      void ShowContextForMesh(DirectoryEntry* entry);
      void ShowGenericContext();

    public:
      bool m_currRoot = false; // Indicates this is a root folder (one level under Resources) and currently selected in the FolderWindow.
      bool m_visible = false;
      bool m_onlyNativeTypes = true;
      Vec2 m_iconSize = Vec2(50.0f);
      std::vector<DirectoryEntry> m_entiries;
      String m_folder;

    private:
      FolderWindow* m_parent = nullptr;
      String m_path;
      bool m_dirty = false;
      ImVec2 m_contextBtnSize = ImVec2(75, 20);
      ImGuiTextFilter m_filter;
    };

    class FolderWindow : public Window
    {
    public:
      FolderWindow(XmlNode* node);
      FolderWindow();
      virtual ~FolderWindow();
      virtual void Show() override;
      virtual Type GetType() const override;
      void Iterate(const String& path, bool clear);
      void UpdateContent();
      void AddEntry(const FolderView& view);
      FolderView& GetView(int indx);
      int Exist(const String& folder);
      bool GetFileEntry(const String& fullPath, DirectoryEntry& entry);

      virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
      virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

    private:
      std::unordered_map<String, Vec2> m_viewSettings;
      std::vector<FolderView> m_entiries;
      int m_activeFolder = -1;
      bool m_showStructure = true;
    };

  }
}