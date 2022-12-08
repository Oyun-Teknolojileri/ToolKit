#include "PropInspector.h"

#include "AnchorMod.h"
#include "App.h"
#include "ComponentView.h"
#include "ConsoleWindow.h"
#include "CustomDataView.h"
#include "EditorPass.h"
#include "EntityView.h"
#include "ImGui/imgui_stdlib.h"
#include "MaterialView.h"
#include "MeshView.h"
#include "Prefab.h"
#include "PrefabView.h"
#include "TransformMod.h"
#include "Util.h"

#include <DirectionComponent.h>

#include <memory>
#include <utility>

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    // View
    //////////////////////////////////////////////////////////////////////////

    bool View::IsTextInputFinalized()
    {
      return (ImGui::IsKeyPressed(ImGuiKey_KeypadEnter) ||
              ImGui::IsKeyPressed(ImGuiKey_Enter) ||
              ImGui::IsKeyPressed(ImGuiKey_Tab));
    }

    void View::DropZone(
        uint fallbackIcon,
        const String& file,
        std::function<void(const DirectoryEntry& entry)> dropAction,
        const String& dropName)
    {
      DirectoryEntry dirEnt;

      bool fileExist                        = false;
      FolderWindowRawPtrArray folderWindows = g_app->GetAssetBrowsers();
      for (FolderWindow* folderWnd : folderWindows)
      {
        if (folderWnd->GetFileEntry(file, dirEnt))
        {
          fileExist = true;
        }
      }
      uint iconId = fallbackIcon;

      ImVec2 texCoords = ImVec2(1.0f, 1.0f);
      if (RenderTargetPtr thumb = dirEnt.GetThumbnail())
      {
        texCoords = ImVec2(1.0f, -1.0f);
        iconId    = thumb->m_textureId;
      }
      else if (fileExist)
      {
        dirEnt.GenerateThumbnail();

        if (RenderTargetPtr thumb = dirEnt.GetThumbnail())
        {
          iconId = thumb->m_textureId;
        }
      }

      if (!dropName.empty())
      {
        ImGui::Text(dropName.c_str());
      }

      bool clicked =
          ImGui::ImageButton(reinterpret_cast<void*>((intptr_t) iconId),
                             ImVec2(48.0f, 48.0f),
                             ImVec2(0.0f, 0.0f),
                             texCoords);

      if (ImGui::BeginDragDropTarget())
      {
        if (const ImGuiPayload* payload =
                ImGui::AcceptDragDropPayload("BrowserDragZone"))
        {
          IM_ASSERT(payload->DataSize == sizeof(DirectoryEntry));
          DirectoryEntry entry = *(const DirectoryEntry*) payload->Data;
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
          auto textureRepFn = [&info, file](const TexturePtr& t) -> void {
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
              g_app->GetPropInspector()->SetMaterialView(mr);
            }

            info += "File: " + dirEnt.m_fileName + dirEnt.m_ext + "\n";
            textureRepFn(mr->m_diffuseTexture);
          }

          if (man->m_type == ResourceType::Texture)
          {
            TexturePtr t = man->Create<Texture>(file);
            textureRepFn(t);
          }

          if (man->m_type == ResourceType::Mesh ||
              man->m_type == ResourceType::SkinMesh)
          {
            MeshPtr mesh = man->Create<Mesh>(file);

            if (clicked)
            {
              g_app->GetPropInspector()->SetMeshView(mesh);
            }

            info += "File: " + dirEnt.m_fileName + dirEnt.m_ext + "\n";
            info +=
                "Vertex Count: " + std::to_string(mesh->m_vertexCount) + "\n";
            info += "Index Count: " + std::to_string(mesh->m_indexCount) + "\n";
            if (mesh->m_faces.size())
            {
              info +=
                  "Face Count: " + std::to_string(mesh->m_faces.size()) + "\n";
            }
          }
        }
      }

      UI::HelpMarker(TKLoc + file, info.c_str(), 0.1f);
    }

    void View::DropSubZone(
        const String& title,
        uint fallbackIcon,
        const String& file,
        std::function<void(const DirectoryEntry& entry)> dropAction,
        bool isEditable)
    {
      ImGui::EndDisabled();
      bool isOpen = ImGui::TreeNodeEx(title.c_str());
      ImGui::BeginDisabled(!isEditable);
      if (isOpen)
      {
        DropZone(fallbackIcon, file, dropAction);
        ImGui::TreePop();
      }
    }

    View::View(const StringView viewName) : m_viewName(viewName)
    {
    }

    // PreviewViewport
    //////////////////////////////////////////////////////////////////////////

    PreviewViewport::PreviewViewport(uint width, uint height)
        : EditorViewport((float) width, (float) height)
    {
      GetCamera()->m_node->Translate(Vec3(0, 0, 5));
      DirectionComponentPtr directionComp =
          GetCamera()->GetComponent<DirectionComponent>();
      directionComp->LookAt(Vec3(0, 0, 0));
      m_renderPass                                  = new SceneRenderPass;
      m_renderPass->m_params.renderPassParams.Scene = std::make_shared<Scene>();
      m_renderPass->m_params.renderPassParams.FrameBuffer      = m_framebuffer;
      m_renderPass->m_params.renderPassParams.Cam              = GetCamera();
      m_renderPass->m_params.renderPassParams.ClearFrameBuffer = true;

      float intensity         = 1.5f;
      DirectionalLight* light = new DirectionalLight();
      light->SetColorVal(Vec3(0.55f));
      light->SetIntensityVal(intensity);
      light->GetComponent<DirectionComponent>()->Yaw(glm::radians(-20.0f));
      light->GetComponent<DirectionComponent>()->Pitch(glm::radians(-20.0f));
      light->SetCastShadowVal(true);
      m_light = light;

      m_renderPass->m_params.renderPassParams.LightOverride = {light};
      m_renderPass->m_params.shadowPassParams.Lights =
          m_renderPass->m_params.renderPassParams.LightOverride;
    }

    PreviewViewport::~PreviewViewport()
    {
      SafeDel(m_renderPass);
      SafeDel(m_light);
    }

    void PreviewViewport::Show()
    {
      RenderTargetPtr colorRT = m_framebuffer->GetAttachment(
          Framebuffer::Attachment::ColorAttachment0);
      if (colorRT == nullptr)
      {
        return;
      }

      ComitResize();
      HandleStates();
      DrawCommands();

      m_renderPass->m_params.shadowPassParams.Entities.clear();
      for (Entity* ntt :
           m_renderPass->m_params.renderPassParams.Scene->GetEntities())
      {
        if (ntt->GetVisibleVal())
        {
          m_renderPass->m_params.shadowPassParams.Entities.push_back(ntt);
        }
      }
      m_renderPass->Render();
      FramebufferSettings fbs = m_framebuffer->GetSettings();
      ImGui::Image(Convert2ImGuiTexture(colorRT),
                   ImVec2((float) fbs.width, (float) fbs.height),
                   ImVec2(0.0f, 0.0f),
                   ImVec2(1.0f, -1.0f));
    }

    ScenePtr PreviewViewport::GetScene()
    {
      return m_renderPass->m_params.renderPassParams.Scene;
    }

    // PropInspector
    //////////////////////////////////////////////////////////////////////////

    PropInspector::PropInspector(XmlNode* node) : PropInspector()
    {
      DeSerialize(nullptr, node);
    }

    PropInspector::PropInspector()
    {
      m_views.resize((uint) ViewType::ViewCount);
      m_views[(uint) ViewType::Entity]     = new EntityView();
      m_views[(uint) ViewType::Prefab]     = new PrefabView();
      m_views[(uint) ViewType::CustomData] = new CustomDataView();
      m_views[(uint) ViewType::Component]  = new ComponentView();
      m_views[(uint) ViewType::Material]   = new MaterialView();
      m_views[(uint) ViewType::Mesh]       = new MeshView();
    }

    PropInspector::~PropInspector()
    {
      for (ViewRawPtr& view : m_views)
      {
        SafeDel(view);
      }
    }

    void PropInspector::Show()
    {
      ImVec4 windowBg  = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
      ImVec4 childBg   = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
      ImGuiStyle style = ImGui::GetStyle();
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                          ImVec2(2, style.ItemSpacing.y));
      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        HandleStates();

        const ImVec2 windowSize      = ImGui::GetWindowSize();
        const ImVec2 sidebarIconSize = ImVec2(18, 18);

        // Show ViewType sidebar
        ImGui::GetStyle()    = style;
        const ImVec2 spacing = ImGui::GetStyle().ItemSpacing;
        const ImVec2 sidebarSize =
            ImVec2(spacing.x + sidebarIconSize.x + spacing.x, windowSize.y);
        if (ImGui::BeginChildFrame(
                ImGui::GetID("ViewTypeSidebar"), sidebarSize, 0))
        {
          for (uint viewIndx = 0; viewIndx < m_views.size(); viewIndx++)
          {
            ViewRawPtr view = m_views[viewIndx];

            if ((uint) m_activeView == viewIndx)
            {
              ImGui::PushStyleColor(ImGuiCol_Button, windowBg);
            }
            else
            {
              ImGui::PushStyleColor(ImGuiCol_Button, childBg);
            }
            if (ImGui::ImageButton(reinterpret_cast<void*>(
                                       (intptr_t) view->m_viewIcn->m_textureId),
                                   sidebarIconSize))
            {
              m_activeView = (ViewType) viewIndx;
            }
            UI::HelpMarker("View#" + std::to_string(viewIndx),
                           view->m_viewName.data());
            ImGui::PopStyleColor(1);
          }
          ImGui::EndChildFrame();
        }

        ImGui::SameLine();

        if (ImGui::BeginChild(
                ImGui::GetID("PropInspectorActiveView"),
                Vec2(windowSize.x - sidebarSize.x - spacing.x, windowSize.y)))
        {
          m_views[(uint) m_activeView]->Show();
          ImGui::EndChild();
        }
      }
      ImGui::End();
      ImGui::PopStyleVar(2);
    }

    Window::Type PropInspector::GetType() const
    {
      return Window::Type::Inspector;
    }

    void PropInspector::DispatchSignals() const
    {
      ModShortCutSignals();
    }
    void PropInspector::SetMaterialView(MaterialPtr mat)
    {
      m_activeView          = ViewType::Material;
      uint matViewIndx      = (uint) ViewType::Material;
      MaterialView* matView = (MaterialView*) m_views[matViewIndx];
      matView->SetMaterial(mat);
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
