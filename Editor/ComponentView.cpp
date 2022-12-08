#include "ComponentView.h"

#include "Action.h"
#include "App.h"
#include "CustomDataView.h"
#include "ImGui/imgui_stdlib.h"

#include <utility>

namespace ToolKit
{
  namespace Editor
  {
    void ShowMultiMaterialComponent(
        ComponentPtr& comp,
        std::function<bool(const String&)> showCompFunc,
        bool modifiableComp)
    {
      MultiMaterialComponent* mmComp = (MultiMaterialComponent*) comp.get();
      MaterialPtrArray& matList      = mmComp->GetMaterialList();
      bool isOpen = showCompFunc(MultiMaterialCompCategory.Name);

      if (isOpen)
      {
        ImGui::BeginDisabled(!modifiableComp);

        uint removeMaterialIndx = TK_UINT_MAX;
        for (uint i = 0; i < matList.size(); i++)
        {
          MaterialPtr& mat = matList[i];
          String path, fileName, ext;
          DecomposePath(mat->GetFile(), &path, &fileName, &ext);
          String uniqueName = std::to_string(i) + "##" + std::to_string(i);
          if (UI::ImageButtonDecorless(
                  UI::m_closeIcon->m_textureId, Vec2(15), false))
          {
            removeMaterialIndx = i;
          }
          ImGui::SameLine();
          CustomDataView::ShowMaterialPtr(
              uniqueName, mat->GetFile(), mat, modifiableComp);
        }
        if (removeMaterialIndx != TK_UINT_MAX)
        {
          mmComp->RemoveMaterial(removeMaterialIndx);
        }

        ImGui::TreePop();

        if (UI::BeginCenteredTextButton("Update"))
        {
          mmComp->UpdateMaterialList(mmComp->m_entity->GetMeshComponent());
        }
        UI::EndCenteredTextButton();
        ImGui::SameLine();
        if (ImGui::Button("Add"))
        {
          mmComp->AddMaterial(GetMaterialManager()->GetCopyOfDefaultMaterial());
        }
        UI::HelpMarker(
            "Update",
            "Update material list by first MeshComponent's mesh list");

        ImGui::EndDisabled();
      }
    }

    void ShowAABBOverrideComponent(
        ComponentPtr& comp,
        std::function<bool(const String&)> showCompFunc,
        bool isEditable)
    {
      AABBOverrideComponent* overrideComp = (AABBOverrideComponent*) comp.get();
      ImGui::BeginDisabled(!isEditable);
      MeshComponentPtr meshComp =
          overrideComp->m_entity->GetComponent<MeshComponent>();
      if (meshComp && ImGui::Button("Update from MeshComponent"))
      {
        overrideComp->SetAABB(meshComp->GetAABB());
      }
      ImGui::EndDisabled();
    }

    void ComponentView::ShowAnimControllerComponent(ParameterVariant* var,
                                                    ComponentPtr comp)
    {
      AnimRecordPtrMap& mref = var->GetVar<AnimRecordPtrMap>();
      String file, id;

      AnimControllerComponent* animPlayerComp =
          reinterpret_cast<AnimControllerComponent*>(comp.get());

      // If component isn't AnimationPlayerComponent, don't show variant
      if (!comp || comp->GetType() != ComponentType::AnimControllerComponent)
      {
        GetLogger()->WriteConsole(
            LogType::Error,
            "AnimRecordPtrMap is for AnimationControllerComponent");
        return;
      }

      if (animPlayerComp->GetActiveRecord())
      {
        String file;
        DecomposePath(animPlayerComp->GetActiveRecord()->m_animation->GetFile(),
                      nullptr,
                      &file,
                      nullptr);

        String text =
            Format("Animation: %s, Duration: %f, T: %f",
                   file.c_str(),
                   animPlayerComp->GetActiveRecord()->m_animation->m_duration,
                   animPlayerComp->GetActiveRecord()->m_currentTime);

        ImGui::Text(text.c_str());
      }

      if (ImGui::BeginTable("Animation Records and Signals",
                            4,
                            ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders |
                                ImGuiTableFlags_Resizable |
                                ImGuiTableFlags_Reorderable |
                                ImGuiTableFlags_ScrollY,
                            ImVec2(ImGui::GetWindowSize().x - 15, 200)))
      {
        float tableWdth = ImGui::GetItemRectSize().x;
        ImGui::TableSetupColumn(
            "Animation", ImGuiTableColumnFlags_WidthStretch, tableWdth / 5.0f);

        ImGui::TableSetupColumn(
            "Name", ImGuiTableColumnFlags_WidthStretch, tableWdth / 2.5f);

        ImGui::TableSetupColumn(
            "Preview", ImGuiTableColumnFlags_WidthStretch, tableWdth / 4.0f);

        ImGui::TableSetupColumn(
            "", ImGuiTableColumnFlags_WidthStretch, tableWdth / 20.0f);

        ImGui::TableHeadersRow();

        uint rowIndx                                     = 0;
        String removedSignalName                         = "";
        String nameUpdated                               = "";
        std::pair<String, AnimRecordPtr> nameUpdatedPair = {};

        static std::pair<String, AnimRecordPtr> extraTrack =
            std::make_pair("", std::make_shared<AnimRecord>());

        // Animation DropZone
        auto showAnimationDropzone =
            [tableWdth, file](uint& columnIndx,
                              const std::pair<String, AnimRecordPtr>& pair) {
              ImGui::TableSetColumnIndex(columnIndx++);
              ImGui::SetCursorPosX(tableWdth / 25.0f);
              DropZone(static_cast<uint>(UI::m_clipIcon->m_textureId),
                       file,
                       [&pair](const DirectoryEntry& entry) -> void {
                         if (GetResourceType(entry.m_ext) ==
                             ResourceType::Animation)
                         {
                           pair.second->m_animation =
                               GetAnimationManager()->Create<Animation>(
                                   entry.GetFullPath());
                           if (pair.first.empty())
                           {
                             extraTrack.first = entry.m_fileName;
                           }
                         }
                         else
                         {
                           GetLogger()->WriteConsole(
                               LogType::Error, "Only animations are accepted.");
                         }
                       });
            };

        auto showSignalName =
            [&nameUpdated, &nameUpdatedPair, tableWdth](
                uint& columnIndx,
                const std::pair<String, AnimRecordPtr>& pair) {
              ImGui::TableSetColumnIndex(columnIndx++);
              ImGui::SetCursorPosY(ImGui::GetCursorPos().y +
                                   (ImGui::GetItemRectSize().y / 4.0f));
              ImGui::PushItemWidth((tableWdth / 2.5f) - 5.0f);
              String readOnly = pair.first;
              if (ImGui::InputText(
                      "##", &readOnly, ImGuiInputTextFlags_EnterReturnsTrue) &&
                  readOnly.length())
              {
                nameUpdated     = readOnly;
                nameUpdatedPair = pair;
              }
              ImGui::PopItemWidth();
            };
        for (auto it = mref.begin(); it != mref.end(); ++it, rowIndx++)
        {
          uint columnIndx = 0;
          ImGui::TableNextRow();
          ImGui::PushID(rowIndx);

          showAnimationDropzone(columnIndx, *it);

          // Signal Name
          showSignalName(columnIndx, *it);

          ImGui::EndDisabled();

          // Play, Pause & Stop Buttons
          ImGui::TableSetColumnIndex(columnIndx++);
          if (it->second->m_animation)
          {
            ImGui::SetCursorPosX(ImGui::GetCursorPos().x +
                                 (ImGui::GetItemRectSize().x / 10.0f));

            ImGui::SetCursorPosY(ImGui::GetCursorPos().y +
                                 (ImGui::GetItemRectSize().y / 5.0f));

            AnimRecordPtr activeRecord = animPlayerComp->GetActiveRecord();

            // Alternate between Play - Pause buttons.
            if (activeRecord == it->second &&
                activeRecord->m_state == AnimRecord::State::Play)
            {
              if (UI::ImageButtonDecorless(
                      UI::m_pauseIcon->m_textureId, Vec2(24, 24), false))
              {
                animPlayerComp->Pause();
              }
            }
            else if (UI::ImageButtonDecorless(
                         UI::m_playIcon->m_textureId, Vec2(24, 24), false))
            {
              animPlayerComp->Play(it->first.c_str());
            }

            // Draw stop button always.
            ImGui::SameLine();
            if (UI::ImageButtonDecorless(
                    UI::m_stopIcon->m_textureId, Vec2(24, 24), false))
            {
              animPlayerComp->Stop();
            }
          }

          ImGui::BeginDisabled(!var->m_editable);

          // Remove Button
          {
            ImGui::TableSetColumnIndex(columnIndx++);
            ImGui::SetCursorPosY(ImGui::GetCursorPos().y +
                                 (ImGui::GetItemRectSize().y / 4.0f));

            if (UI::ImageButtonDecorless(
                    UI::m_closeIcon->m_textureId, Vec2(15, 15), false))
            {
              removedSignalName = it->first;
            }
          }

          ImGui::PopID();
        }

        // Show last extra track.
        uint columnIndx = 0;
        ImGui::TableNextRow();
        ImGui::PushID(rowIndx);

        showAnimationDropzone(columnIndx, extraTrack);

        // Signal Name
        showSignalName(columnIndx, extraTrack);
        ImGui::PopID();

        if (removedSignalName.length())
        {
          animPlayerComp->RemoveSignal(removedSignalName);
        }

        if (nameUpdated.length() && nameUpdatedPair.first != nameUpdated)
        {
          if (mref.find(nameUpdated) != mref.end())
          {
            GetLogger()->WriteConsole(LogType::Error, "SignalName exists");
          }
          else if (nameUpdatedPair.first == extraTrack.first)
          {
            extraTrack.first = nameUpdated;
          }
          else
          {
            auto node  = mref.extract(nameUpdatedPair.first);
            node.key() = nameUpdated;
            mref.insert(std::move(node));

            nameUpdated     = "";
            nameUpdatedPair = {};
          }
        }

        // If extra track is filled properly, add it to the list
        if (extraTrack.first != "" && extraTrack.second->m_animation != nullptr)
        {
          mref.insert(extraTrack);
          extraTrack.first  = "";
          extraTrack.second = std::make_shared<AnimRecord>();
        }

        ImGui::EndTable();
      }
    }

    bool ComponentView::ShowComponentBlock(ComponentPtr& comp,
                                           const bool modifiableComp)
    {
      VariantCategoryArray categories;
      comp->m_localData.GetCategories(categories, true, true);

      bool removeComp   = false;
      auto showCompFunc = [comp, &removeComp, modifiableComp](
                              const String& headerName) -> bool {
        ImGui::PushID(static_cast<int>(comp->m_id));
        String varName = headerName + "##" + std::to_string(modifiableComp);
        bool isOpen    = ImGui::TreeNodeEx(
            varName.c_str(), ImGuiTreeNodeFlags_DefaultOpen | g_treeNodeFlags);

        if (modifiableComp)
        {
          float offset = ImGui::GetContentRegionAvail().x - 30.0f;
          ImGui::SameLine(offset);
          if (UI::ImageButtonDecorless(
                  UI::m_closeIcon->m_textureId, ImVec2(15.0f, 15.0f), false) &&
              !removeComp)
          {
            g_app->m_statusMsg = "Component " + headerName + " removed.";
            removeComp         = true;
          }
        }
        ImGui::PopID();

        return isOpen;
      };
      for (VariantCategory& category : categories)
      {
        bool isOpen = showCompFunc(category.Name);

        if (isOpen)
        {
          ParameterVariantRawPtrArray vars;
          comp->m_localData.GetByCategory(category.Name, vars);

          for (ParameterVariant* var : vars)
          {
            bool editable = var->m_editable;
            if (!modifiableComp)
            {
              var->m_editable = false;
            }
            CustomDataView::ShowVariant(var, comp);
            if (!modifiableComp)
            {
              var->m_editable = true;
            }
          }

          ImGui::TreePop();
        }
      }
      switch (comp->GetType())
      {
      case ComponentType::MultiMaterialComponent:
        ShowMultiMaterialComponent(comp, showCompFunc, modifiableComp);
        break;
      case ComponentType::AABBOverrideComponent:
        ShowAABBOverrideComponent(comp, showCompFunc, modifiableComp);
        break;
      }

      return removeComp;
    }

    // ComponentView
    //////////////////////////////////////////////////////////////////////////

    ComponentView::ComponentView() : View("Component View")
    {
      m_viewID  = 3;
      m_viewIcn = UI::m_collectionIcon;
    }
    ComponentView::~ComponentView()
    {
    }

    void ComponentView::Show()
    {
      m_entity = g_app->GetCurrentScene()->GetCurrentSelection();
      if (m_entity == nullptr)
      {
        ImGui::Text("Select an entity");
        return;
      }

      if (ImGui::CollapsingHeader("Components", ImGuiTreeNodeFlags_DefaultOpen))
      {
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, g_indentSpacing);

        std::vector<ULongID> compRemove;
        for (ComponentPtr& com : m_entity->GetComponentPtrArray())
        {
          if (ShowComponentBlock(com, true))
          {
            compRemove.push_back(com->m_id);
          }
        }

        for (ULongID id : compRemove)
        {
          ActionManager::GetInstance()->AddAction(
              new DeleteComponentAction(m_entity->GetComponent(id)));
        }

        // Remove billboards if necessary
        std::static_pointer_cast<EditorScene>(
            GetSceneManager()->GetCurrentScene())
            ->InitEntityBillboard(m_entity);

        ImGui::PushItemWidth(150);
        static bool addInAction = false;
        if (addInAction)
        {
          int dataType = 0;
          if (ImGui::Combo("##NewComponent",
                           &dataType,
                           "..."
                           "\0Mesh Component"
                           "\0Material Component"
                           "\0Environment Component"
                           "\0Animation Controller Component"
                           "\0Skeleton Component"
                           "\0Multi-Material Component"
                           "\0AABB Override Component"))
          {
            Component* newComponent = nullptr;
            switch (dataType)
            {
            case 1:
              newComponent = new MeshComponent();
              break;
            case 2:
              newComponent = new MaterialComponent;
              break;
            case 3:
              newComponent = new EnvironmentComponent;
              break;
            case 4:
              newComponent = new AnimControllerComponent;
              break;
            case 5:
              newComponent = new SkeletonComponent;
              break;
            case 6: {
              MultiMaterialComponent* mmComp = new MultiMaterialComponent;
              mmComp->UpdateMaterialList(m_entity->GetMeshComponent());
              newComponent = mmComp;
            }
            break;
            case 7:
              newComponent = new AABBOverrideComponent;
              break;
            default:
              break;
            }

            if (newComponent)
            {
              m_entity->AddComponent(newComponent);
              addInAction = false;

              // Add gizmo if needed
              std::static_pointer_cast<EditorScene>(
                  GetSceneManager()->GetCurrentScene())
                  ->AddBillboardToEntity(m_entity);
            }
          }
        }
        ImGui::PopItemWidth();

        ImGui::Separator();
        if (UI::BeginCenteredTextButton("Add Component"))
        {
          addInAction = true;
        }
        UI::EndCenteredTextButton();

        ImGui::PopStyleVar();
      }
    }

  } // namespace Editor
} // namespace ToolKit