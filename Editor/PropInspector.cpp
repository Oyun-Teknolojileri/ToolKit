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

#include "PropInspector.h"

#include "App.h"
#include "ComponentView.h"
#include "CustomDataView.h"
#include "DirectionComponent.h"
#include "EntityView.h"
#include "GradientSky.h"
#include "MaterialView.h"
#include "MeshView.h"
#include "PrefabView.h"

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    // View
    //////////////////////////////////////////////////////////////////////////

    bool View::IsTextInputFinalized()
    {
      return (ImGui::IsKeyPressed(ImGuiKey_KeypadEnter) || ImGui::IsKeyPressed(ImGuiKey_Enter) ||
              ImGui::IsKeyPressed(ImGuiKey_Tab));
    }

    void View::DropZone(uint fallbackIcon,
                        const String& file,
                        std::function<void(DirectoryEntry& entry)> dropAction,
                        const String& dropName,
                        bool isEditable)
    {
      DirectoryEntry dirEnt;

      bool fileExist                        = GetFileManager()->CheckFileFromResources(file);
      FolderWindowRawPtrArray folderWindows = g_app->GetAssetBrowsers();
      for (FolderWindow* folderWnd : folderWindows)
      {
        if (folderWnd->GetFileEntry(file, dirEnt))
        {
          fileExist = true;
        }
      }

      uint iconId                        = fallbackIcon;
      ImVec2 texCoords                   = ImVec2(1.0f, 1.0f);

      ThumbnailManager& thumbnailManager = g_app->m_thumbnailManager;

      if (dirEnt.m_ext.length())
      {
        if (thumbnailManager.TryGetThumbnail(iconId, dirEnt))
        {
          texCoords = ImVec2(1.0f, -1.0f);
        }
      }
      else if (fileExist)
      {
        DecomposePath(file, &dirEnt.m_rootPath, &dirEnt.m_fileName, &dirEnt.m_ext);

        thumbnailManager.TryGetThumbnail(iconId, dirEnt);
      }

      if (!dropName.empty())
      {
        ImGui::Text(dropName.c_str());
      }

      ImGui::ImageButton(reinterpret_cast<void*>((intptr_t) iconId),
                         ImVec2(48.0f, 48.0f),
                         ImVec2(0.0f, 0.0f),
                         texCoords);

      bool clicked = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
      clicked      &= ImGui::IsItemHovered();

      if (isEditable && ImGui::BeginDragDropTarget())
      {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BrowserDragZone"))
        {
          const FileDragData& dragData = FolderView::GetFileDragData();
          DirectoryEntry& entry        = *dragData.Entries[0]; // get first entry
          dropAction(entry);
        }

        ImGui::EndDragDropTarget();
      }

      // Show file info.
      String info = "Drop zone";
      if (!file.empty() && !dirEnt.m_fileName.empty())
      {
        info = "";
        if (ResourceManager* man = dirEnt.GetManager())
        {
          auto textureRepFn = [&info, file](const TexturePtr& t) -> void
          {
            if (t)
            {
              String file, ext;
              DecomposePath(t->GetFile(), nullptr, &file, &ext);

              info += "Texture: " + file + ext + "\n";
              info += "Width: " + std::to_string(t->m_width) + "\n";
              info += "Height: " + std::to_string(t->m_height);
            }
          };

          if (man->m_type == ResourceType::Material)
          {
            MaterialPtr mr = man->Create<Material>(file);
            if (clicked)
            {
              PropInspector* propInspector = g_app->GetPropInspector();
              propInspector->GetMaterialView()->SetSelectedMaterial(mr);
              propInspector->SetActiveView(ViewType::Material);
            }

            info += "File: " + dirEnt.m_fileName + dirEnt.m_ext + "\n";
            textureRepFn(mr->m_diffuseTexture);
          }

          if (man->m_type == ResourceType::Texture)
          {
            TexturePtr t = man->Create<Texture>(file);
            textureRepFn(t);
          }

          if (man->m_type == ResourceType::Mesh || man->m_type == ResourceType::SkinMesh)
          {
            MeshPtr mesh = man->Create<Mesh>(file);

            if (clicked)
            {
              g_app->GetPropInspector()->SetMeshView(mesh);
            }

            info += "File: " + dirEnt.m_fileName + dirEnt.m_ext + "\n";
            info += "Vertex Count: " + std::to_string(mesh->m_vertexCount) + "\n";
            info += "Index Count: " + std::to_string(mesh->m_indexCount) + "\n";
            if (mesh->m_faces.size())
            {
              info += "Face Count: " + std::to_string(mesh->m_faces.size()) + "\n";
            }
          }
        }
      }

      if (fileExist && info != "")
      {
        UI::HelpMarker(TKLoc + file, info.c_str(), 0.1f);
      }
    }

    void View::DropSubZone(const String& title,
                           uint fallbackIcon,
                           const String& file,
                           std::function<void(const DirectoryEntry& entry)> dropAction,
                           bool isEditable)
    {
      bool isOpen = ImGui::TreeNodeEx(title.c_str());
      if (isOpen)
      {
        DropZone(fallbackIcon, file, dropAction, "", isEditable);
        ImGui::TreePop();
      }
    }

    View::View(const StringView viewName) : m_viewName(viewName) {}

    // PreviewViewport
    //////////////////////////////////////////////////////////////////////////

    PreviewViewport::PreviewViewport(uint width, uint height) : EditorViewport((float) width, (float) height)
    {
      m_scenes[0] = GetSceneManager()->Create<Scene>(ScenePath("ms-sphere.scene", true));
      m_scenes[1] = GetSceneManager()->Create<Scene>(ScenePath("ms-box.scene", true));
      m_scenes[2] = GetSceneManager()->Create<Scene>(ScenePath("ms-ball.scene", true));

      m_previewRenderer                            = std::make_shared<SceneRenderer>();
      m_previewRenderer->m_params.Cam              = GetCamera();
      m_previewRenderer->m_params.ClearFramebuffer = true;
      m_previewRenderer->m_params.MainFramebuffer  = m_framebuffer;
      m_previewRenderer->m_params.Scene            = m_scenes[0];
    }

    PreviewViewport::~PreviewViewport() { m_previewRenderer = nullptr; }

    void PreviewViewport::Show()
    {
      if (m_needsResize)
      {
        OnResizeContentArea((float) m_size.x, (float) m_size.y);
      }

      HandleStates();
      DrawCommands();

      m_previewRenderer->m_params.MainFramebuffer = m_framebuffer;
      GetRenderSystem()->AddRenderTask({[this](Renderer* r) -> void { m_previewRenderer->Render(r); }});

      // Render color attachment as rounded image
      FramebufferSettings fbSettings = m_framebuffer->GetSettings();
      Vec2 imageSize                 = Vec2(fbSettings.width, fbSettings.height);
      Vec2 currentCursorPos =
          Vec2(ImGui::GetWindowContentRegionMin()) + Vec2(ImGui::GetCursorPos()) + Vec2(ImGui::GetWindowPos());

      if (m_isTempView)
      {
        currentCursorPos.y -= 24.0f;
      }

      ImGui::Dummy(imageSize);

      ImGui::GetWindowDrawList()->AddImageRounded(
          Convert2ImGuiTexture(m_framebuffer->GetAttachment(Framebuffer::Attachment::ColorAttachment0)),
          currentCursorPos,
          currentCursorPos + imageSize,
          Vec2(0.0f, 0.0f),
          Vec2(1.0f, -1.0f),
          ImGui::GetColorU32(Vec4(1, 1, 1, 1)),
          5.0f);
    }

    ScenePtr PreviewViewport::GetScene() { return m_previewRenderer->m_params.Scene; }

    void PreviewViewport::SetScene(int i) { m_previewRenderer->m_params.Scene = m_scenes[glm::clamp(0, 2, i)]; }

    void PreviewViewport::ResetCamera()
    {
      Camera* cam = GetCamera();
      cam->m_node->SetTranslation(Vec3(3.0f, 5.0f, 4.0f));
      cam->GetComponent<DirectionComponent>()->LookAt(Vec3(0.0f));
    }

    void PreviewViewport::ResizeWindow(uint width, uint height)
    {
      if (width != m_size.x || height != m_size.y)
      {
        EditorViewport::ResizeWindow(width, height);
      }
    }

    // PropInspector
    //////////////////////////////////////////////////////////////////////////

    PropInspector::PropInspector(XmlNode* node) : PropInspector() { DeSerialize(nullptr, node); }

    PropInspector::PropInspector()
    {
      // order is important, depends on enum viewType
      m_views.resize((uint) ViewType::ViewCount);
      m_views[(uint) ViewType::Entity]     = new EntityView();
      m_views[(uint) ViewType::CustomData] = new CustomDataView();
      m_views[(uint) ViewType::Component]  = new ComponentView();
      m_views[(uint) ViewType::Material]   = new MaterialView();
      m_views[(uint) ViewType::Mesh]       = new MeshView();
      m_views[(uint) ViewType::Prefab]     = new PrefabView();

      m_entityViews.push_back((uint) ViewType::Entity);
      m_entityViews.push_back((uint) ViewType::CustomData);
      m_entityViews.push_back((uint) ViewType::Component);
      m_entityViews.push_back((uint) ViewType::Material);
      m_entityViews.push_back((uint) ViewType::Mesh);

      // prefab view doesn't have CustomDataView or ComponentView
      // because prefab view provides this views. hence no need to add views
      m_prefabViews.push_back((uint) ViewType::Entity);
      m_prefabViews.push_back((uint) ViewType::Prefab);
      m_prefabViews.push_back((uint) ViewType::Material);
      m_prefabViews.push_back((uint) ViewType::Mesh);
    }

    PropInspector::~PropInspector()
    {
      for (ViewRawPtr& view : m_views)
      {
        SafeDel(view);
      }
    }

    void PropInspector::SetActiveView(ViewType viewType) { m_activeView = viewType; }

    MaterialView* PropInspector::GetMaterialView()
    {
      return static_cast<MaterialView*>(m_views[(uint) ViewType::Material]);
    }

    void PropInspector::DeterminateSelectedMaterial(Entity* curEntity)
    {
      // if material view is active. determinate selected material
      MaterialView* matView = dynamic_cast<MaterialView*>(m_views[(uint) m_activeView]);
      if (matView == nullptr)
      {
        return;
      }

      if (curEntity == nullptr)
      {
        // set empty array, there is no material sellected
        matView->SetMaterials({});
        return;
      }

      if (curEntity->GetType() == EntityType::Entity_Prefab)
      {
        PrefabView* prefView = static_cast<PrefabView*>(m_views[(uint) ViewType::Prefab]);
        if (Entity* prefEntity = prefView->GetActiveEntity())
        {
          curEntity = prefEntity;
        }
      }

      if (MaterialComponentPtr mat = curEntity->GetMaterialComponent())
      {
        matView->SetMaterials(mat->GetMaterialList());
      }
      else
      {
        // entity doesn't have material clear material view.
        matView->SetMaterials({});
      }
    }

    void PropInspector::Show()
    {
      ImVec4 windowBg  = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
      ImVec4 childBg   = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
      ImGuiStyle style = ImGui::GetStyle();
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(3.0f, 0.0f));
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, style.ItemSpacing.y));

      Entity* curEntity = g_app->GetCurrentScene()->GetCurrentSelection();
      bool isPrefab     = curEntity != nullptr && curEntity->GetType() == EntityType::Entity_Prefab;

      UIntArray views   = isPrefab ? m_prefabViews : m_entityViews;

      if (ImGui::Begin(m_name.c_str(), &m_visible, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
      {
        HandleStates();

        const ImVec2 windowSize      = ImGui::GetWindowSize();
        const ImVec2 sidebarIconSize = ImVec2(18.0f, 18.0f);

        // Show ViewType sidebar
        ImGui::GetStyle()            = style;
        const ImVec2 spacing         = ImGui::GetStyle().ItemSpacing;
        const ImVec2 sidebarSize     = ImVec2(spacing.x + sidebarIconSize.x + spacing.x, windowSize.y);

        DeterminateSelectedMaterial(curEntity);

        if (ImGui::BeginChildFrame(ImGui::GetID("ViewTypeSidebar"), sidebarSize, 0))
        {
          for (uint viewIndx : views)
          {
            ViewRawPtr view   = m_views[viewIndx];
            bool isActiveView = (uint) m_activeView == viewIndx;

            ImGui::PushStyleColor(ImGuiCol_Button, isActiveView ? windowBg : childBg);

            // is this font icon ? (prefab's icon is font icon)
            if (view->m_fontIcon.size() > 0)
            {
              ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 1.0, 1.0, 1.0));

              if (ImGui::Button(view->m_fontIcon.data(), ImVec2(27, 25)))
              {
                m_activeView = (ViewType) viewIndx;
              }
              ImGui::PopStyleColor();
            }
            else if (ImGui::ImageButton(Convert2ImGuiTexture(view->m_viewIcn), sidebarIconSize))
            {
              m_activeView = (ViewType) viewIndx;
            }
            UI::HelpMarker("View#" + std::to_string(viewIndx), view->m_viewName.data());
            ImGui::PopStyleColor(1);
          }
        }
        ImGui::EndChildFrame();

        ImGui::SameLine();

        if (ImGui::BeginChild(ImGui::GetID("PropInspectorActiveView"), ImGui::GetContentRegionAvail()))
        {
          m_views[(uint) m_activeView]->Show();
        }
        ImGui::EndChild();
      }
      ImGui::End();
      ImGui::PopStyleVar(2);
    }

    Window::Type PropInspector::GetType() const { return Window::Type::Inspector; }

    void PropInspector::DispatchSignals() const { ModShortCutSignals(); }

    void PropInspector::SetMaterials(const MaterialPtrArray& mat)
    {
      m_activeView          = ViewType::Material;
      uint matViewIndx      = (uint) ViewType::Material;
      MaterialView* matView = (MaterialView*) m_views[matViewIndx];
      matView->SetMaterials(mat);
    }

    void PropInspector::SetMeshView(MeshPtr mesh)
    {
      m_activeView       = ViewType::Mesh;
      uint meshViewIndx  = (uint) ViewType::Mesh;
      MeshView* meshView = (MeshView*) m_views[meshViewIndx];
      meshView->SetMesh(mesh);
    }

  } // namespace Editor
} // namespace ToolKit
