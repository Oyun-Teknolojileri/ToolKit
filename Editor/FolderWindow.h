#pragma once

#include "UI.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

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
      void GenerateThumbnail() const;
      RenderTargetPtr GetThumbnail() const;

     public:
      String m_ext;
      String m_fileName;
      String m_rootPath;
      bool m_isDirectory = false;

     private:
      MaterialPtr m_tempThumbnailMaterial = nullptr;
    };

    class FolderWindow;

    class FolderView
    {
     public:
      FolderView();
      explicit FolderView(FolderWindow* parent);

      void Show();
      void SetPath(const String& path);
      const String& GetPath() const;
      void Iterate();
      int Exist(const String& file);
      void ShowContextMenu(DirectoryEntry* entry = nullptr);
      void Refresh();
      float GetThumbnailZoomPercent(float thumbnailZoom);

     private:
      void CreateItemActions();
      void MoveTo(const String& dst); // Imgui Drop target.

     public:
      // Indicates this is a root folder (one level under Resources)
      // and currently selected in the FolderWindow.
      bool m_currRoot = false;
      // States if the tab is visible.
      // Doesnt necesserly mean active, its just a tab in the FolderView.
      bool m_visible = false;
      bool m_active  = false; // Active tab, whose content is being displayed.
      // Always false. When set to true,
      // actives the view and becomes false again.
      bool m_activateNext    = false;
      bool m_onlyNativeTypes = true;
      Vec2 m_iconSize        = Vec2(50.0f);
      std::vector<DirectoryEntry> m_entiries;
      String m_folder;

     private:
      FolderWindow* m_parent = nullptr;
      String m_path;
      bool m_dirty            = false;
      ImVec2 m_contextBtnSize = ImVec2(75, 20);
      ImGuiTextFilter m_filter;
      std::unordered_map<String, std::function<void(DirectoryEntry*)>>
          m_itemActions;

      // If you change this value, change the calculaton of thumbnail zoom
      const float m_thumbnailMaxZoom = 300.f;
    };

    class FolderWindow : public Window
    {
     public:
      explicit FolderWindow(XmlNode* node);
      FolderWindow();
      virtual ~FolderWindow();
      void Show() override;
      Type GetType() const override;
      void Iterate(const String& path, bool clear, bool addEngine = true);
      void UpdateContent();
      void AddEntry(const FolderView& view);
      FolderView& GetView(int indx);
      // Returns root level active view, if deep is true,
      // returns sub-active / visible view.
      FolderView* GetActiveView(bool deep);
      void SetActiveView(FolderView* view);
      int Exist(const String& folder);
      bool GetFileEntry(const String& fullPath, DirectoryEntry& entry);

      void Serialize(XmlDocument* doc, XmlNode* parent) const override;
      void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

     private:
      struct ViewSettings
      {
        Vec2 size;
        bool visible;
        bool active;
      };

      std::unordered_map<String, ViewSettings> m_viewSettings;
      std::vector<FolderView> m_entiries;
      int m_activeFolder   = -1;
      bool m_showStructure = true;
    };

  } // namespace Editor
} // namespace ToolKit
