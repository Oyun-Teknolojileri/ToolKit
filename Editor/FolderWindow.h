/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "DirectoryEntry.h"
#include "MaterialView.h"
#include "UI.h"

namespace ToolKit
{
  namespace Editor
  {

    struct FileDragData
    {
      int NumFiles             = 0;
      DirectoryEntry** Entries = nullptr;
    };

    class FolderWindow;

    /** The class that is responsible for showing / managing contents of a folder. */
    class FolderView
    {
     public:
      FolderView();
      ~FolderView();
      FolderView(FolderWindow* parent);

      void Show();
      void SetDirty();
      void SetPath(const String& path);
      const String& GetPath() const;

      /** Root part of the path is returned. Root is one level after Resource. Ex: ".../Resource/Audio" */
      String GetRoot() const;

      void Iterate();
      int Exist(const String& file, const String& ext);
      void ShowContextMenu(DirectoryEntry* entry = nullptr);
      void Refresh();
      float GetThumbnailZoomPercent(float thumbnailZoom);
      int SelectFolder(FolderWindow* window, const String& path);
      void DropFiles(const String& dst); //!< drop selectedFiles

      static const FileDragData& GetFileDragData();

     private:
      void HandleCopyPasteDelete();
      static void PasteFiles(const String& path);
      void DrawSearchBar();
      void CreateItemActions();
      void MoveTo(const String& dst); // Imgui Drop target.

      void DetermineAndSetBackgroundColor(bool isSelected, int index);
      bool IsMultiSelecting();

      /** Selects the files between two entry index(including a and b) */
      void SelectFilesInRange(int a, int b);

     public:
      int m_folderIndex      = -1;

      // Indicates this is a root folder (one level under Resources) and currently selected in the FolderWindow.
      bool m_currRoot        = false;
      // Indicates this is a root folder (one level under Resources)
      bool m_root            = false;
      // States if the tab is visible. Doesn't necessarily mean active, its just a tab in the FolderView.
      bool m_visible         = false;
      // Active tab, whose content is being displayed.
      bool m_active          = false;

      bool m_onlyNativeTypes = true;

      std::vector<DirectoryEntry> m_entries;
      Vec2 m_iconSize           = Vec2(50.0f);

      int m_lastClickedEntryIdx = -1;
      String m_folder;
      String m_path;

     private:
      FolderWindow* m_parent  = nullptr;
      bool m_dirty            = false;
      ImVec2 m_contextBtnSize = ImVec2(75, 20);
      String m_filter         = "";
      std::unordered_map<String, std::function<void(DirectoryEntry*, FolderView*)>> m_itemActions;

      // If you change this value, change the calculation of thumbnail zoom
      const float m_thumbnailMaxZoom = 300.f;
    };

    /** Window that is responsible of showing folder views and a structure of all folders in provided path. */
    class FolderWindow : public Window
    {
     public:
      TKDeclareClass(FolderWindow, Window);

      FolderWindow();
      virtual ~FolderWindow();
      void Show() override;

      /** Reiterate all the views and updates their content. */
      void UpdateContent();
      /** Returns the view in the given index of the entry list. */
      FolderView& GetView(int indx);
      /** Returns active folder view. */
      FolderView* GetActiveView();
      /** Sets the active view. */
      void SetActiveView(int index);
      /** Sets the active view. */
      void SetActiveView(FolderView* view);
      /** Checks if the given path exist in folder views. */
      int Exist(const String& path);
      /** Returns the DirectoryEntry for the given path. */
      bool GetFileEntry(const String& fullPath, DirectoryEntry& entry);
      /** Adds a folder view to entries. */
      void AddEntry(FolderView& view);
      /** Invalidates all views which causes them to be re filled with file content. */
      void SetViewsDirty();
      /** Reconstructs hierarchic folder tree. */
      void ReconstructFolderTree();
      /** Iterates the folders in the resource path. */
      void IterateFolders(bool includeEngine);

     protected:
      XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
      XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;

     private:
      /** Returns active folder's ascendants views. */
      IntArray GetAscendants();
      /** Returns active folder's sibling views. */
      IntArray GetSiblings();
      /** Based on selected folder, updates the current root folder. */
      void UpdateCurrentRoot();

      void ShowFolderTree();
      int CreateTreeRec(int parent, const std::filesystem::path& path);
      void DrawTreeRec(int index, float depth);
      void Iterate(const String& path, bool clear, bool addEngine = true);

     private:
      struct FolderNode
      {
        FolderNode() {}

        FolderNode(int idx, String p, String n) : index(idx), path(std::move(p)), name(std::move(n)) {}

        String path = "undefined";
        String name = "undefined";
        IntArray childs;
        int index = -1;
      };

      /** Flat resource content. */
      std::vector<FolderView> m_entries;
      /** Hierarchic resource content. */
      std::vector<FolderNode> m_folderNodes;
      /**  Active folder whose contents get shown. */
      int m_activeFolder       = 0;
      /** Hierarchy tree maximum width. */
      float m_maxTreeNodeWidth = 160.0f;
      /**  Whether show the tree structure of the resource. */
      bool m_showStructure     = true;
      int m_resourcesTreeIndex = 0;
    };

    typedef std::shared_ptr<FolderWindow> FolderWindowPtr;
    typedef std::vector<FolderWindow*> FolderWindowRawPtrArray;

  } // namespace Editor
} // namespace ToolKit
