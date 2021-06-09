#include "stdafx.h"

#include "FolderWindow.h"
#include "GlobalDef.h"
#include "DebugNew.h"

#include <filesystem>

namespace ToolKit
{
  namespace Editor
  {

    FolderView::FolderView()
    {
    }

    FolderView::FolderView(class FolderWindow* parent)
    {
      m_parent = parent;
    }

    void FolderView::Show()
    {
      bool* visCheck = nullptr;
      if (!m_currRoot)
      {
        visCheck = &m_visible;
      }

      if (ImGui::BeginTabItem(m_folder.c_str(), visCheck))
      {
        if (ImGui::IsItemHovered())
        {
          ImGui::SetTooltip(m_path.c_str());
        }

        ImGui::BeginChild("##Content", ImVec2(0, 0), true);
        for (int i = 0; i < (int)m_entiries.size(); i++)
        {
          ImGuiStyle& style = ImGui::GetStyle();
          float visX2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

          DirectoryEntry& de = m_entiries[i];

          uint iconId = UI::m_fileIcon->m_textureId;
          if (de.m_isDirectory)
          {
            iconId = UI::m_folderIcon->m_textureId;
          }
          else if (de.m_ext == SCENE)
          {
            iconId = UI::m_worldIcon->m_textureId;
          }
          else if (de.m_ext == MESH)
          {
            if (de.m_thumbNail == nullptr)
            {
              GenerateThumbNail(de);
            }
            //iconId = UI::m_meshIcon->m_textureId;
            iconId = de.m_thumbNail->m_textureId;
          }
          else if (de.m_ext == ANIM)
          {
            iconId = UI::m_clipIcon->m_textureId;
          }
          else if (de.m_ext == SKINMESH)
          {
            iconId = UI::m_armatureIcon->m_textureId;
          }
          else if (de.m_ext == AUDIO)
          {
            iconId = UI::m_audioIcon->m_textureId;
          }
          else if (de.m_ext == SHADER)
          {
            iconId = UI::m_codeIcon->m_textureId;
          }
          else if (de.m_ext == SKELETON)
          {
            iconId = UI::m_boneIcon->m_textureId;
          }
          else if (de.m_ext == MATERIAL)
          {
            iconId = UI::m_materialIcon->m_textureId;
          }
          else if (de.m_ext == PNG)
          {
            iconId = UI::m_imageIcon->m_textureId;
          }
          else if (de.m_ext == JPEG)
          {
            iconId = UI::m_imageIcon->m_textureId;
          }
          else if (de.m_ext == TGA)
          {
            iconId = UI::m_imageIcon->m_textureId;
          }
          else if (de.m_ext == BMP)
          {
            iconId = UI::m_imageIcon->m_textureId;
          }
          else if (de.m_ext == PSD)
          {
            iconId = UI::m_imageIcon->m_textureId;
          }
          else
          {
            if (m_onlyNativeTypes)
            {
              continue;
            }
          }

          ImGui::PushID(i);
          ImGui::BeginGroup();
          ImGui::ImageButton((void*)(intptr_t)iconId, GLM2IMVEC(m_iconSize));
          if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
          {
            if (ImGui::IsItemHovered())
            {
              if (de.m_isDirectory)
              {
                if (m_parent != nullptr)
                {
                  String path = de.m_rootPath + "\\" + de.m_fileName;
                  int indx = m_parent->Exist(path);
                  if (indx == -1)
                  {
                    FolderView view(m_parent);
                    view.SetPath(path);
                    view.Iterate();
                    m_parent->AddEntry(view);
                  }
                  else
                  {
                    m_parent->GetView(indx).m_visible = true;
                  }
                }
              }
            }
          }

          String fullName = de.m_fileName + de.m_ext;
          if (ImGui::IsItemHovered())
          {
            ImGui::SetTooltip(fullName.c_str());
          }

          if (!de.m_isDirectory)
          {
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
            {
              ImGui::SetDragDropPayload("BrowserDragZone", &de, sizeof(DirectoryEntry));
              //ImGui::Text("Copy %s", fullName.c_str());
              ImGui::SetTooltip("Copy %s", fullName.c_str());
              ImGui::EndDragDropSource();
            }
          }

          ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + m_iconSize.x);
          ImGui::TextWrapped(de.m_fileName.c_str());
          ImGui::PopTextWrapPos();
          ImGui::EndGroup();
          ImGui::PopID();

          float lastBtnX2 = ImGui::GetItemRectMax().x;
          float nextBtnX2 = lastBtnX2 + style.ItemSpacing.x + m_iconSize.x;
          if (nextBtnX2 < visX2)
          {
            ImGui::SameLine();
          }
        }
        ImGui::EndChild();
        ImGui::EndTabItem();
      }
    }

    void FolderView::SetPath(const String& path)
    {
      m_path = path;
      StringArray splits;
      Split(path, "\\", splits);
      m_folder = splits.back();
    }

    const String& FolderView::GetPath() const
    {
      return m_path;
    }

    void FolderView::Iterate()
    {
      using namespace std::filesystem;

      m_entiries.clear();
      for (const directory_entry& e : directory_iterator(m_path))
      {
        DirectoryEntry de;
        de.m_isDirectory = e.is_directory();
        de.m_rootPath = e.path().parent_path().u8string();
        de.m_fileName = e.path().stem().u8string();
        de.m_ext = e.path().filename().extension().u8string();

        m_entiries.push_back(de);
      }
    }

    int FolderView::Exist(const String& file)
    {
      for (int i = 0; i < (int)m_entiries.size(); i++)
      {
        if (m_entiries[i].m_fileName == file)
        {
          return i;
        }
      }

      return -1;
    }

    void FolderView::GenerateThumbNail(DirectoryEntry& entry)
    {
      if (entry.m_ext == MESH)
      {
        Drawable dw;
        String fullpath = entry.m_rootPath + GetPathSeparator() + entry.m_fileName + entry.m_ext;
        dw.m_mesh = Main::GetInstance()->m_meshMan.Create(fullpath);
        dw.m_mesh->Init();

        BoundingBox bb = dw.GetAABB();
        Vec3 geoCenter = (bb.max - bb.min) * 0.5f;
        geoCenter = Vec3(glm::compMax(geoCenter));

        Camera cam;
        cam.SetLens(25.0f, m_iconSize.x, m_iconSize.y);
        cam.Translate(geoCenter * 10.0f);
        cam.LookAt(geoCenter);

        RenderTarget* thumb = new RenderTarget((uint)m_iconSize.x, (uint)m_iconSize.y);
        thumb->Init();
        g_app->m_renderer->SwapRenderTarget(&thumb);
        g_app->m_renderer->Render(&dw, &cam, g_app->m_sceneLights);
        g_app->m_renderer->SwapRenderTarget(&thumb, false);
        entry.m_thumbNail = RenderTargetPtr(thumb);
      }
    }

    FolderWindow::FolderWindow()
    {
    }

    void FolderWindow::Show()
    {
      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        HandleStates();

        auto IsRootFn = [](const String& path)
        {
          return std::count(path.begin(), path.end(), '\\') == 2;
        };

        // Show Resource folder structure.
        ImGui::PushID("##FolderStructure");
        ImGui::BeginGroup();
        ImGui::TextUnformatted("Resources");
        ImGui::BeginChild("##Folders", ImVec2(130, 0), true);
        static int selectedFolder = -1;
        for (int i = 0; i < (int)m_entiries.size(); i++)
        {
          if (!IsRootFn(m_entiries[i].GetPath()))
          {
            continue;
          }

          bool currSel = false;
          if (selectedFolder == i)
          {
            currSel = true;
          }

          currSel = UI::ToggleButton
          (
            m_entiries[i].m_folder,
            ImVec2(100, 25),
            currSel
          );

          // Selection switch.
          if (currSel)
          {
            selectedFolder = (int)i;
          }
        }
        ImGui::EndChild();
        ImGui::EndGroup();
        ImGui::PopID();

        ImGui::SameLine();

        ImGui::PushID("##FolderContent");
        ImGui::BeginGroup();
        if (ImGui::BeginTabBar("Folders", ImGuiTabBarFlags_NoTooltip | ImGuiTabBarFlags_AutoSelectNewTabs))
        {
          for (int i = 0; i < (int)m_entiries.size(); i++)
          {
            m_entiries[i].m_currRoot = i == selectedFolder;
            if (IsRootFn(m_entiries[i].GetPath()))
            {
              if (!m_entiries[i].m_currRoot)
              {
                // Show only current root folder and all sub folders.
                continue;
              }
            }
            m_entiries[i].Show();
          }
          ImGui::EndTabBar();
        }
        ImGui::EndGroup();
        ImGui::PopID();
      }

      ImGui::End();
    }

    Window::Type FolderWindow::GetType() const
    {
      return Window::Type::Browser;
    }

    void FolderWindow::Iterate(const String& path)
    {
      using namespace std::filesystem;

      m_entiries.clear();
      for (const directory_entry& e : directory_iterator(path))
      {
        if (e.is_directory())
        {
          FolderView view(this);
          view.SetPath(e.path().u8string());
          view.Iterate();
          m_entiries.push_back(view);
        }
      }
    }

    void FolderWindow::UpdateContent()
    {
      for (FolderView& view : m_entiries)
      {
        view.Iterate();
      }
    }

    void FolderWindow::AddEntry(const FolderView& view)
    {
      if (Exist(view.GetPath()) == -1)
      {
        m_entiries.push_back(view);
      }
    }

    FolderView& FolderWindow::GetView(int indx)
    {
      return m_entiries[indx];
    }

    int FolderWindow::Exist(const String& folder)
    {
      for (size_t i = 0; i < m_entiries.size(); i++)
      {
        if (m_entiries[i].GetPath() == folder)
        {
          return (int)i;
        }
      }

      return -1;
    }

  }
}
