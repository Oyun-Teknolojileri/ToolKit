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

#include "FolderWindow.h"

#include "App.h"

#include <Animation.h>
#include <Audio.h>
#include <Material.h>
#include <Mesh.h>
#include <Scene.h>
#include <Shader.h>
#include <Texture.h>
#include <ToolKit.h>

#include <DebugNew.h>

namespace ToolKit
{
  namespace Editor
  {
    DirectoryEntry::DirectoryEntry() {}

    DirectoryEntry::DirectoryEntry(const String& fullPath)
    {
      DecomposePath(fullPath, &m_rootPath, &m_fileName, &m_ext);
    }

    String DirectoryEntry::GetFullPath() const { return ConcatPaths({m_rootPath, m_fileName + m_ext}); }

    ResourceManager* DirectoryEntry::GetManager() const
    {
      using GetterFunction = std::function<ResourceManager*()>;

      static std::unordered_map<String, GetterFunction> extToResource {
          {ANIM,     GetAnimationManager},
          {AUDIO,    GetAudioManager    },
          {MATERIAL, GetMaterialManager },
          {MESH,     GetMeshManager     },
          {SKINMESH, GetMeshManager     },
          {SHADER,   GetShaderManager   },
          {HDR,      GetTextureManager  },
          {SCENE,    GetSceneManager    }
      };

      auto resourceManager = extToResource.find(m_ext);
      if (resourceManager != extToResource.end())
      {
        return resourceManager->second(); // call get function
      }

      if (SupportedImageFormat(m_ext))
      {
        return GetTextureManager();
      }

      return nullptr;
    }

    String GetRootPath(const String& folder)
    {
      static std::unordered_set<String> rootMap = {"Fonts", "Materials", "Meshes", "Scenes", "Shaders", "Textures"};

      String path                               = folder;
      String subFolder {};
      // traverse parent paths and search a root folder
      while (path.size() > 0ull)
      {
        subFolder.clear();

        while (path.size() > 0ull)
        {
          // pop last character
          char c = path.back();
          path.erase(path.end() - 1ull);

          if (c == '\\' || c == '/')
          {
            break;
          }
          // push character to subFolder
          subFolder.push_back(c);
        }
        // reverse because we pushed reversely
        std::reverse(subFolder.begin(), subFolder.end());
        // if we found a root folder return this path
        if (rootMap.count(subFolder) > 0)
        {
          return ConcatPaths({path, subFolder});
        }
      }
      return DefaultPath();
    }

    RenderTargetPtr DirectoryEntry::GetThumbnail() const { return g_app->m_thumbnailManager.GetThumbnail(*this); }

    FolderWindow::FolderWindow(XmlNode* node)
    {
      DeSerialize(nullptr, node);
      Iterate(ResourcePath(), true);
    }

    FolderWindow::FolderWindow(bool addEngine) { Iterate(ResourcePath(), true, addEngine); }

    FolderWindow::~FolderWindow() {}

    // destroy old one and create new tree
    void FolderWindow::ReconstructFolderTree()
    {
      m_folderNodes.clear();
      CreateTreeRec(-1, DefaultPath());
      m_resourcesTreeIndex = (int) m_folderNodes.size();
      CreateTreeRec(int(m_folderNodes.size()) - 1, ResourcePath());
    }

    // parent will start with -1
    int FolderWindow::CreateTreeRec(int parent, const std::filesystem::path& path)
    {
      String folderName = path.filename().u8string();
      int index         = (int) m_folderNodes.size();
      m_folderNodes.emplace_back(index, std::filesystem::absolute(path).u8string(), folderName);

      for (const std::filesystem::directory_entry& directory : std::filesystem::directory_iterator(path))
      {
        if (!directory.is_directory())
        {
          continue;
        }

        int childIdx = CreateTreeRec(parent + 1, directory.path());
        m_folderNodes[index].childs.push_back(childIdx);
      }

      return index;
    }

    void FolderWindow::DeactivateNode(const String& name)
    {
      for (int i = 0; i < m_folderNodes.size(); ++i)
      {
        if (m_folderNodes[i].name == name)
        {
          m_folderNodes[i].active = false;
        }
      }
    }

    extern FolderView* g_dragBeginView;

    void FolderWindow::DrawTreeRec(int index, float depth)
    {
      if (index == -1)
      {
        return; // shouldn't happen
      }

      FolderNode& node       = m_folderNodes[index];
      String icon            = node.active ? ICON_FA_FOLDER_OPEN_A : ICON_FA_FOLDER_A;
      String nodeHeader      = icon + ICON_SPACE + node.name;
      float headerLen        = ImGui::CalcTextSize(nodeHeader.c_str()).x;
      headerLen              += (depth * 20.0f) + 70.0f; // depth padding + UI start padding

      m_maxTreeNodeWidth     = glm::max(headerLen, m_maxTreeNodeWidth);

      const auto onClickedFn = [&]() -> void
      {
        // find clicked entity
        int selected = Exist(node.path);
        if (selected != -1)
        {
          FolderView& selectedEntry = m_entries[selected];
          bool rootChanged          = false;

          if (m_lastSelectedTreeNode == -1)
          {
            m_activeFolder = selected;
          }
          else if (m_lastSelectedTreeNode < m_entries.size())
          {
            FolderView& lastSelectedEntry = m_entries[m_lastSelectedTreeNode];

            // set old node active false(icon will change to closed)
            DeactivateNode(lastSelectedEntry.m_folder);
            lastSelectedEntry.m_active = false;

            if (GetRootPath(selectedEntry.GetPath()) != GetRootPath(lastSelectedEntry.GetPath()))
            {
              // root folders different we should switch active folder
              m_activeFolder = selected;

              for (int i : GetVeiws())
              {
                FolderView& v = m_entries[i];
                if (!v.m_currRoot)
                {
                  m_entries[i].m_visible = false;
                }
              }
            }
          }

          AddEntry(selectedEntry);
          node.active                  = true;
          m_lastSelectedTreeNode       = selected;
          selectedEntry.m_active       = true;
          selectedEntry.m_activateNext = true;
          selectedEntry.m_visible      = true;
        }
      };

      const auto acceptDrop = [&]() -> void
      {
        if (ImGui::BeginDragDropTarget())
        {
          if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BrowserDragZone"))
          {
            if (g_dragBeginView != nullptr)
            {
              g_dragBeginView->DropFiles(node.path);
            }
          }
          ImGui::EndDragDropTarget();
        }
      };

      ImGuiTreeNodeFlags nodeFlags = g_treeNodeFlags;
      String stdId                 = "##" + std::to_string(index);

      if (node.childs.size() == 0)
      {
        nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

        if (ImGui::TreeNodeEx(stdId.c_str(), nodeFlags, nodeHeader.c_str()))
        {
          if (ImGui::IsItemClicked())
          {
            onClickedFn();
          }
        }
        acceptDrop();
      }
      else
      {
        if (ImGui::TreeNodeEx(stdId.c_str(), nodeFlags, nodeHeader.c_str()))
        {
          if (ImGui::IsItemClicked())
          {
            onClickedFn();
          }

          for (int i = 0; i < node.childs.size(); ++i)
          {
            DrawTreeRec(node.childs[i], depth + 1.0f);
          }
          ImGui::TreePop();
        }
        acceptDrop();
      }
    }

    void FolderWindow::ShowFolderTree()
    {
      // Show Resource folder structure.
      ImGui::PushID("##FolderStructure");
      ImGui::BeginGroup();

      ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
      ImGui::TextUnformatted("Resources");

      ImGui::SameLine();
      if (ImGui::Button(ICON_FA_ARROW_LEFT))
      {
        m_showStructure = !m_showStructure;
      }

      ImGui::BeginChild("##Folders", ImVec2(m_maxTreeNodeWidth, 0.0f), true);

      // reset tree node default size
      m_maxTreeNodeWidth = 160.0f;
      // draw tree of folders
      DrawTreeRec(m_resourcesTreeIndex, 0.0f);
      DrawTreeRec(0, 0.0f);

      ImGui::EndChild();

      ImGui::PopStyleVar();
      ImGui::EndGroup();
      ImGui::PopID();
    }

    IntArray FolderWindow::GetVeiws()
    {
      String currRootPath;
      auto IsDescendentFn = [&currRootPath](StringView candidate) -> bool
      { return !currRootPath.empty() && candidate.find(currRootPath) != std::string::npos; };

      IntArray views;

      for (int i = 0; i < (int) m_entries.size(); i++)
      {
        FolderView& view = m_entries[i];
        String candidate = view.GetPath();
        if ((view.m_currRoot = (i == m_activeFolder)))
        {
          currRootPath = candidate;
        }

        // Show only current root folder or descendents.
        if (view.m_currRoot || IsDescendentFn(candidate))
        {
          views.push_back(i);
        }
      }
      return views;
    }

    void FolderWindow::Show()
    {
      ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Once);
      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        HandleStates();

        if (!g_app->m_workspace.GetActiveWorkspace().empty() && g_app->m_workspace.GetActiveProject().name.empty())
        {
          ImGui::Text("Load a project.");
          ImGui::End();
          return;
        }

        if (m_showStructure)
        {
          ShowFolderTree();
        }
        else
        {
          if (ImGui::Button(ICON_FA_ARROW_RIGHT))
          {
            m_showStructure = !m_showStructure;
          }
        }

        ImGui::SameLine();

        ImGui::PushID("##FolderContent");
        ImGui::BeginGroup();
        if (ImGui::BeginTabBar("Folders",
                               ImGuiTabBarFlags_NoTooltip | ImGuiTabBarFlags_AutoSelectNewTabs |
                                   ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar))
        {
          for (int i : GetVeiws())
          {
            m_entries[i].Show();
          }
          ImGui::EndTabBar();
        }

        ImGui::EndGroup();
        ImGui::PopID();
      }

      ImGui::End();
    }

    Window::Type FolderWindow::GetType() const { return Window::Type::Browser; }

    void FolderWindow::Iterate(const String& path, bool clear, bool addEngine)
    {
      if (clear)
      {
        m_entries.clear();
        ReconstructFolderTree();
      }

      String resourceRoot = ResourcePath();
      char pathSep        = GetPathSeparator();
      int baseCount       = CountChar(resourceRoot, pathSep);

      for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(path))
      {
        if (entry.is_directory())
        {
          FolderView view(this);
          String path = entry.path().u8string();
          view.m_root = CountChar(path, pathSep) == baseCount + 1;

          view.SetPath(path);
          if (!view.m_folder.compare("Engine"))
          {
            continue;
          }

          if (m_viewSettings.find(path) != m_viewSettings.end())
          {
            ViewSettings vs     = m_viewSettings[path];
            view.m_iconSize     = vs.size;
            view.m_visible      = vs.visible;
            view.m_activateNext = vs.active;
          }

          view.Iterate();
          m_entries.push_back(view);
          Iterate(view.GetPath(), false, false);
        }
      }

      if (addEngine)
      {
        // Engine folder
        FolderView view(this);
        view.SetPath(DefaultAbsolutePath());

        if (m_viewSettings.find(path) != m_viewSettings.end())
        {
          ViewSettings vs     = m_viewSettings[path];
          view.m_iconSize     = vs.size;
          view.m_visible      = vs.visible;
          view.m_activateNext = vs.active;
        }

        view.Iterate();
        m_entries.push_back(view);
        Iterate(view.GetPath(), false, false);
      }
    }

    void FolderWindow::UpdateContent()
    {
      for (FolderView& view : m_entries)
      {
        view.Iterate();
      }
    }

    void FolderWindow::AddEntry(const FolderView& view)
    {
      if (Exist(view.GetPath()) == -1)
      {
        m_entries.push_back(view);
      }
    }

    void FolderWindow::SetViewsDirty()
    {
      for (FolderView& view : m_entries)
      {
        view.SetDirty();
      }
    }

    FolderView& FolderWindow::GetView(int indx) { return m_entries[indx]; }

    FolderView* FolderWindow::GetActiveView(bool deep)
    {
      if (m_activeFolder == -1)
      {
        return nullptr;
      }

      if (deep)
      {
        FolderView& rootView = GetView(m_activeFolder);
        for (FolderView& view : m_entries)
        {
          if (view.m_active && view.m_visible)
          {
            return &view;
          }
        }
      }

      return &GetView(m_activeFolder);
    }

    void FolderWindow::SetActiveView(FolderView* view)
    {
      int viewIndx = Exist(view->GetPath());
      if (viewIndx != -1)
      {
        for (FolderView& view : m_entries)
        {
          view.m_active = false;
        }
        m_entries[viewIndx].m_active = true;
      }
    }

    int FolderWindow::Exist(const String& folder)
    {
      for (size_t i = 0; i < m_entries.size(); i++)
      {
        if (m_entries[i].GetPath() == folder)
        {
          return static_cast<int>(i);
        }
      }

      return -1;
    }

    bool FolderWindow::GetFileEntry(const String& fullPath, DirectoryEntry& entry)
    {
      String path, name, ext;
      DecomposePath(fullPath, &path, &name, &ext);
      int viewIndx = Exist(path);
      if (viewIndx != -1)
      {
        int dirEntry = m_entries[viewIndx].Exist(name, ext);
        if (dirEntry != -1)
        {
          entry = m_entries[viewIndx].m_entries[dirEntry];
          return true;
        }
      }

      return false;
    }

    XmlNode* FolderWindow::SerializeImp(XmlDocument* doc, XmlNode* parent) const
    {
      XmlNode* wndNode = Window::SerializeImp(doc, parent);
      XmlNode* folder  = CreateXmlNode(doc, "FolderWindow", wndNode);

      WriteAttr(folder, doc, "activeFolder", std::to_string(m_activeFolder));
      WriteAttr(folder, doc, "showStructure", std::to_string(m_showStructure));

      for (const FolderView& view : m_entries)
      {
        XmlNode* viewNode = doc->allocate_node(rapidxml::node_element, "FolderView");
        WriteAttr(viewNode, doc, XmlNodePath.data(), view.GetPath());
        WriteAttr(viewNode, doc, "vis", std::to_string(view.m_visible));
        WriteAttr(viewNode, doc, "active", std::to_string(view.m_active));
        folder->append_node(viewNode);
        XmlNode* setting = doc->allocate_node(rapidxml::node_element, "IconSize");
        WriteVec(setting, doc, view.m_iconSize);
        viewNode->append_node(setting);
      }

      return folder;
    }

    void FolderWindow::DeSerializeImp(XmlDocument* doc, XmlNode* parent)
    {
      Window::DeSerializeImp(doc, parent);
      if (XmlNode* node = parent->first_node("FolderWindow"))
      {
        ReadAttr(node, "activeFolder", m_activeFolder);
        ReadAttr(node, "showStructure", m_showStructure);

        if (XmlNode* view = node->first_node("FolderView"))
        {
          do
          {
            String path;
            ReadAttr(view, XmlNodePath.data(), path);

            ViewSettings vs;
            vs.visible = false;
            ReadAttr(view, "vis", vs.visible);

            vs.active = false;
            ReadAttr(view, "active", vs.active);

            vs.size = Vec2(50.0f);
            if (XmlNode* setting = view->first_node("IconSize"))
            {
              ReadVec(setting, vs.size);
            }

            m_viewSettings[path] = vs;
          } while (view = view->next_sibling("FolderView"));
        }
      }
    }

  } // namespace Editor
} // namespace ToolKit
