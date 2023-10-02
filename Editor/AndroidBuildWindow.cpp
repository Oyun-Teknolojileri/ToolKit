
#include "AndroidBuildWindow.h"
#include "App.h"

namespace ToolKit::Editor
{
  void AndroidBuildWindow::Show()
  {
    ImGuiIO io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(400, 250), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                            ImGuiCond_Once,
                            ImVec2(0.5f, 0.5f));
    
    ImGui::Begin("Android Build", nullptr);
    
    ImGui::InputText("Name", &m_appName);
    
    ImGui::Text("Icon");
    ImGui::SameLine();

    int iconId = m_icon ? m_icon->m_textureId : m_defaultIcon->m_textureId;
    ImGui::ImageButton((void*)(intptr_t)iconId, ImVec2(64, 64)); 
    
    if (ImGui::BeginDragDropTarget())
    {
      if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BrowserDragZone"))
      {
        const FileDragData& dragData = FolderView::GetFileDragData();
        DirectoryEntry& entry        = *dragData.Entries[0]; // get first entry
        m_icon = GetTextureManager()->Create<Texture>(entry.GetFullPath());
        m_icon->Init(false);
      }

      ImGui::EndDragDropTarget();
    }

    ImGui::InputInt("Min SDK", &m_minSdk);
    ImGui::InputInt("Max SDK", &m_maxSdk);

    ImGui::Text("Select Orientation:");
    const char* orientations[] = { "Landscape", "Portrait" };
    ImGui::Combo("##OrientationCombo", (int*)&m_selectedOriantation, orientations, 2);

    if (ImGui::Button("Cancel"))
    {
      UI::RemoveTempWindow(this);
      m_menuOpen = false;
    }

    ImGui::SameLine();
    
    if (ImGui::Button("Build"))
    {
      AndroidPublisher* publisher = g_app->m_publishManager->m_androidPublisher;
      publisher->m_minSdk         = m_minSdk;
      publisher->m_maxSdk         = m_maxSdk;
      publisher->m_appName        = m_appName;
      publisher->m_icon           = m_icon;
      publisher->m_oriantation      = (AndroidPublisher::Oriantation)m_selectedOriantation;

      g_app->m_publishManager->Publish(PublishPlatform::Android);
      UI::RemoveTempWindow(this);
      m_menuOpen = false;
    }

    ImGui::End();
  }
  
  void AndroidBuildWindow::OpenBuildWindow()
  {
    if (m_menuOpen)
    {
      return;
    }
    
    if (m_appName.empty()) 
    {
      m_appName = g_app->m_workspace.GetActiveProject().name;
    }

    if (m_defaultIcon == nullptr) 
    {
      m_defaultIcon = GetTextureManager()->Create<Texture>(TexturePath(ConcatPaths({"ToolKit","Icons", "app.png"})));
      m_defaultIcon->Init(false);
    }

    UI::AddTempWindow(this);
    m_menuOpen = true;
  }
}