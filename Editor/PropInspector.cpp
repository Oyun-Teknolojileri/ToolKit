#include "PropInspector.h"

#include "AnchorMod.h"
#include "App.h"
#include "ConsoleWindow.h"
#include "ImGui/imgui_stdlib.h"
#include "MaterialInspector.h"
#include "Prefab.h"
#include "TransformMod.h"
#include "Util.h"

#include <memory>
#include <utility>

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    // View
    //////////////////////////////////////////////////////////////////////////

    bool IsTextInputFinalized()
    {
      return (ImGui::IsKeyPressed(ImGuiKey_KeypadEnter) ||
              ImGui::IsKeyPressed(ImGuiKey_Enter) ||
              ImGui::IsKeyPressed(ImGuiKey_Tab));
    }

    void DropZone(uint fallbackIcon,
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
              g_app->GetMaterialInspector()->m_material = mr;
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

    void DropSubZone(
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

    void ShowMaterialPtr(const String& uniqueName,
                         const String& file,
                         MaterialPtr& var,
                         bool isEditable)
    {
      DropSubZone(
          uniqueName,
          static_cast<uint>(UI::m_materialIcon->m_textureId),
          file,
          [&var](const DirectoryEntry& entry) -> void {
            if (GetResourceType(entry.m_ext) == ResourceType::Material)
            {
              var = GetMaterialManager()->Create<Material>(entry.GetFullPath());
            }
            else
            {
              GetLogger()->WriteConsole(LogType::Error,
                                        "Only Material Types are accepted.");
            }
          },
          isEditable);
    }

    void ShowMaterialVariant(const String& uniqueName,
                             const String& file,
                             ParameterVariant* var)
    {
      DropSubZone(
          uniqueName,
          static_cast<uint>(UI::m_materialIcon->m_textureId),
          file,
          [&var](const DirectoryEntry& entry) -> void {
            if (GetResourceType(entry.m_ext) == ResourceType::Material)
            {
              *var =
                  GetMaterialManager()->Create<Material>(entry.GetFullPath());
            }
            else
            {
              GetLogger()->WriteConsole(LogType::Error,
                                        "Only Material Types are accepted.");
            }
          },
          var->m_editable);
    }

    void ShowAnimControllerComponent(ParameterVariant* var, ComponentPtr comp)
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

    void ShowVariant(ParameterVariant* var, ComponentPtr comp)
    {
      if (!var->m_exposed)
      {
        return;
      }

      ImGui::BeginDisabled(!var->m_editable);

      switch (var->GetType())
      {
      case ParameterVariant::VariantType::Bool: {
        bool val = var->GetVar<bool>();
        if (ImGui::Checkbox(var->m_name.c_str(), &val))
        {
          *var = val;
        }
      }
      break;
      case ParameterVariant::VariantType::Float: {
        float val = var->GetVar<float>();
        if (!var->m_hint.isRangeLimited)
        {
          if (ImGui::InputFloat(var->m_name.c_str(), &val))
          {
            *var = val;
          }
        }
        else
        {
          if (ImGui::DragFloat(var->m_name.c_str(),
                               &val,
                               var->m_hint.increment,
                               var->m_hint.rangeMin,
                               var->m_hint.rangeMax))
          {
            *var = val;
          }
        }
      }
      break;
      case ParameterVariant::VariantType::Int: {
        int val = var->GetVar<int>();
        if (var->m_hint.isRangeLimited)
        {
          if (ImGui::DragInt(var->m_name.c_str(),
                             &val,
                             var->m_hint.increment,
                             static_cast<int>(var->m_hint.rangeMin),
                             static_cast<int>(var->m_hint.rangeMax)))
          {
            *var = val;
          }
        }
        else
        {
          if (ImGui::InputInt(var->m_name.c_str(), &val))
          {
            *var = val;
          }
        }
      }
      break;
      case ParameterVariant::VariantType::Vec2: {
        Vec2 val = var->GetVar<Vec2>();
        if (var->m_hint.isRangeLimited)
        {
          if (ImGui::DragFloat2(var->m_name.c_str(),
                                &val[0],
                                var->m_hint.increment,
                                var->m_hint.rangeMin,
                                var->m_hint.rangeMax))
          {
            *var = val;
          }
        }
        else
        {
          if (ImGui::DragFloat2(var->m_name.c_str(), &val[0], 0.1f))
          {
            *var = val;
          }
        }
      }
      break;
      case ParameterVariant::VariantType::Vec3: {
        Vec3 val = var->GetVar<Vec3>();
        if (var->m_hint.isColor)
        {
          if (ImGui::ColorEdit3(
                  var->m_name.c_str(), &val[0], ImGuiColorEditFlags_NoLabel))
          {
            *var = val;
          }
        }
        else if (var->m_hint.isRangeLimited)
        {
          if (ImGui::DragFloat3(var->m_name.c_str(),
                                &val[0],
                                var->m_hint.increment,
                                var->m_hint.rangeMin,
                                var->m_hint.rangeMax))
          {
            *var = val;
          }
        }
        else
        {
          if (ImGui::DragFloat3(var->m_name.c_str(), &val[0], 0.1f))
          {
            *var = val;
          }
        }
      }
      break;
      case ParameterVariant::VariantType::Vec4: {
        Vec4 val = var->GetVar<Vec4>();
        if (var->m_hint.isColor)
        {
          if (ImGui::ColorEdit4(
                  var->m_name.c_str(), &val[0], ImGuiColorEditFlags_NoLabel))
          {
            *var = val;
          }
        }
        else if (var->m_hint.isRangeLimited)
        {
          if (ImGui::DragFloat4(var->m_name.c_str(),
                                &val[0],
                                var->m_hint.increment,
                                var->m_hint.rangeMin,
                                var->m_hint.rangeMax))
          {
            *var = val;
          }
        }
        else
        {
          if (ImGui::DragFloat4(var->m_name.c_str(), &val[0], 0.1f))
          {
            *var = val;
          }
        }
      }
      break;
      case ParameterVariant::VariantType::String: {
        String val = var->GetVar<String>();
        if (ImGui::InputText(var->m_name.c_str(), &val) &&
            IsTextInputFinalized())
        {
          *var = val;
        }
      }
      break;
      case ParameterVariant::VariantType::ULongID: {
        ULongID val = var->GetVar<ULongID>();
        if (ImGui::InputScalar(var->m_name.c_str(),
                               ImGuiDataType_U32,
                               var->GetVarPtr<ULongID>()) &&
            IsTextInputFinalized())
        {
          *var = val;
        }
      }
      break;
      case ParameterVariant::VariantType::MaterialPtr: {
        MaterialPtr& mref = var->GetVar<MaterialPtr>();
        String file, id;
        if (mref)
        {
          id   = std::to_string(mref->m_id);
          file = mref->GetFile();
        }

        String uniqueName = var->m_name + "##" + id;
        ShowMaterialVariant(uniqueName, file, var);
      }
      break;
      case ParameterVariant::VariantType::MeshPtr: {
        MeshPtr mref = var->GetVar<MeshPtr>();
        DropSubZone(
            "Mesh##" + std::to_string(mref->m_id),
            static_cast<uint>(UI::m_meshIcon->m_textureId),
            mref->GetFile(),
            [&var](const DirectoryEntry& entry) -> void {
              if (GetResourceType(entry.m_ext) == ResourceType::Mesh)
              {
                *var = GetMeshManager()->Create<Mesh>(entry.GetFullPath());
              }
              else if (GetResourceType(entry.m_ext) == ResourceType::SkinMesh)
              {
                *var = GetMeshManager()->Create<SkinMesh>(entry.GetFullPath());
              }
              else
              {
                GetLogger()->WriteConsole(LogType::Error,
                                          "Only meshes are accepted.");
              }
            },
            var->m_editable);
      }
      break;
      case ParameterVariant::VariantType::HdriPtr: {
        HdriPtr mref = var->GetVar<HdriPtr>();
        String file, id;
        if (mref)
        {
          id   = std::to_string(mref->m_id);
          file = mref->GetFile();
        }

        DropSubZone(
            "Hdri##" + id,
            UI::m_imageIcon->m_textureId,
            file,
            [&var](const DirectoryEntry& entry) -> void {
              if (GetResourceType(entry.m_ext) == ResourceType::Hdri)
              {
                *var = GetTextureManager()->Create<Hdri>(entry.GetFullPath());
              }
              else
              {
                GetLogger()->WriteConsole(LogType::Error,
                                          "Only hdri's are accepted.");
              }
            },
            var->m_editable);
      }
      break;
      case ParameterVariant::VariantType::SkeletonPtr: {
        SkeletonPtr mref = var->GetVar<SkeletonPtr>();
        String file, id;
        if (mref)
        {
          id   = std::to_string(mref->m_id);
          file = mref->GetFile();
        }

        auto dropZoneFnc = [&var, &comp](const DirectoryEntry& entry) -> void {
          if (GetResourceType(entry.m_ext) == ResourceType::Skeleton)
          {
            *var = GetSkeletonManager()->Create<Skeleton>(entry.GetFullPath());
            if (comp->GetType() == ComponentType::SkeletonComponent)
            {
              SkeletonComponent* skelComp = (SkeletonComponent*) comp.get();
              skelComp->Init();
            }
          }
          else
          {
            GetLogger()->WriteConsole(LogType::Error,
                                      "Only skeletons are accepted.");
          }
        };
        DropSubZone("Skeleton##" + id,
                    UI::m_boneIcon->m_textureId,
                    file,
                    dropZoneFnc,
                    var->m_editable);
      }
      break;
      case ParameterVariant::VariantType::AnimRecordPtrMap: {
        ShowAnimControllerComponent(var, comp);
      }
      break;
      default:
        break;
      }

      ImGui::EndDisabled();
    }

    ValueUpdateFn MultiUpdate(ParameterVariant* var)
    {
      EntityRawPtrArray entities;
      g_app->GetCurrentScene()->GetSelectedEntities(entities);

      // Remove current selected because its already updated.
      entities.pop_back();

      ValueUpdateFn multiUpdate = [var, entities](Value& oldVal,
                                                  Value& newVal) -> void {
        for (Entity* ntt : entities)
        {
          ParameterVariant* vLookUp = nullptr;
          if (ntt->m_localData.LookUp(
                  var->m_category.Name, var->m_name, &vLookUp))
          {
            vLookUp->SetValue(newVal);
          }
        }
      };

      return multiUpdate;
    }

    void ShowCustomData(Entity* m_entity,
                        String headerName,
                        ParameterVariantRawPtrArray& vars,
                        bool isListEditable)
    {
      if (headerName.length() &&
          !ImGui::CollapsingHeader(headerName.c_str(),
                                   ImGuiTreeNodeFlags_DefaultOpen))
      {
        return;
      }
      if (ImGui::BeginTable(headerName.c_str(),
                            3,
                            ImGuiTableFlags_Resizable |
                                ImGuiTableFlags_SizingFixedSame))
      {
        Vec2 xSize = ImGui::CalcTextSize("Name");
        xSize *= 3.0f;
        ImGui::TableSetupColumn(
            "Name", ImGuiTableColumnFlags_WidthFixed, xSize.x);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

        xSize = ImGui::CalcTextSize("X");
        xSize *= 2.5f;
        ImGui::TableSetupColumn(
            "##Remove", ImGuiTableColumnFlags_WidthFixed, xSize.x);

        ImGui::TableHeadersRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::PushItemWidth(-FLT_MIN);

        ImGui::TableSetColumnIndex(1);
        ImGui::PushItemWidth(-FLT_MIN);

        ParameterVariant* remove = nullptr;
        for (size_t i = 0; i < vars.size(); i++)
        {
          ValueUpdateFn multiUpdateFn = MultiUpdate(vars[i]);
          vars[i]->m_onValueChangedFn.push_back(multiUpdateFn);

          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);

          ImGui::PushID(static_cast<int>(i));
          ParameterVariant* var = vars[i];
          static char buff[1024];
          strcpy_s(buff, sizeof(buff), var->m_name.c_str());

          String pNameId = "##Name" + std::to_string(i);
          if (isListEditable)
          {
            ImGui::InputText(pNameId.c_str(), buff, sizeof(buff));
          }
          else
          {
            ImGui::Text(var->m_name.c_str());
          }
          var->m_name = buff;

          ImGui::TableSetColumnIndex(1);

          String pId = "##" + std::to_string(i);
          switch (var->GetType())
          {
          case ParameterVariant::VariantType::String: {
            ImGui::InputText(pId.c_str(), var->GetVarPtr<String>());
          }
          break;
          case ParameterVariant::VariantType::Bool: {
            bool val = var->GetVar<bool>();
            if (ImGui::Checkbox(pId.c_str(), &val))
            {
              *var = val;
            }
          }
          break;
          case ParameterVariant::VariantType::Int: {
            ImGui::InputInt(pId.c_str(), var->GetVarPtr<int>());
          }
          break;
          case ParameterVariant::VariantType::Float: {
            ImGui::DragFloat(pId.c_str(), var->GetVarPtr<float>(), 0.1f);
          }
          break;
          case ParameterVariant::VariantType::Vec3: {
            ImGui::DragFloat3(pId.c_str(), &var->GetVar<Vec3>()[0], 0.1f);
          }
          break;
          case ParameterVariant::VariantType::Vec4: {
            ImGui::DragFloat4(pId.c_str(), &var->GetVar<Vec4>()[0], 0.1f);
          }
          break;
          case ParameterVariant::VariantType::Mat3: {
            Vec3 vec;
            Mat3 val = var->GetVar<Mat3>();
            for (int j = 0; j < 3; j++)
            {
              pId += std::to_string(j);
              vec = glm::row(val, j);
              ImGui::InputFloat3(pId.c_str(), &vec[0]);
              val  = glm::row(val, j, vec);
              *var = val;
            }
          }
          break;
          case ParameterVariant::VariantType::Mat4: {
            Vec4 vec;
            Mat4 val = var->GetVar<Mat4>();
            for (int j = 0; j < 4; j++)
            {
              pId += std::to_string(j);
              vec = glm::row(val, j);
              ImGui::InputFloat4(pId.c_str(), &vec[0]);
              val  = glm::row(val, j, vec);
              *var = val;
            }
          }
          break;
          }

          ImGui::TableSetColumnIndex(2);
          if (isListEditable && ImGui::Button("X"))
          {
            remove = vars[i];
            g_app->m_statusMsg =
                Format("Parameter %d: %s removed.", i + 1, var->m_name.c_str());
          }

          vars[i]->m_onValueChangedFn.pop_back();
          ImGui::PopID();
        }

        if (remove != nullptr)
        {
          m_entity->m_localData.Remove(remove->m_id);
        }

        ImGui::EndTable();
        ImGui::Separator();

        static bool addInAction = false;
        if (isListEditable && addInAction)
        {
          ImGui::PushItemWidth(150);
          int dataType = 0;
          if (ImGui::Combo(
                  "##NewCustData",
                  &dataType,
                  "..."
                  "\0String\0Boolean\0Int\0Float\0Vec3\0Vec4\0Mat3\0Mat4"))
          {
            ParameterVariant customVar;
            // This makes them only visible in Custom Data dropdown.
            customVar.m_exposed  = true;
            customVar.m_editable = true;
            customVar.m_category = CustomDataCategory;

            bool added = true;
            switch (dataType)
            {
            case 1:
              customVar = "";
              break;
            case 2:
              customVar = false;
              break;
            case 3:
              customVar = 0;
              break;
            case 4:
              customVar = 0.0f;
              break;
            case 5:
              customVar = ZERO;
              break;
            case 6:
              customVar = Vec4();
              break;
            case 7:
              customVar = Mat3();
              break;
            case 8:
              customVar = Mat4();
              break;
            default:
              added = false;
              break;
            }

            if (added)
            {
              m_entity->m_localData.Add(customVar);
              addInAction = false;
            }
          }
          ImGui::PopItemWidth();
        }

        if (isListEditable)
        {
          if (UI::BeginCenteredTextButton("Add Custom Data"))
          {
            addInAction = true;
          }
          UI::EndCenteredTextButton();
        }
      }
    };

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

        uint removeMaterialIndx = UINT32_MAX;
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
          ShowMaterialPtr(uniqueName, mat->GetFile(), mat, modifiableComp);
        }
        if (removeMaterialIndx != UINT32_MAX)
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

    const StringView& boolToString(bool b)
    {
      return b ? "true" : "false";
    }

    bool ShowComponentBlock(ComponentPtr& comp, const bool modifiableComp)
    {
      VariantCategoryArray categories;
      comp->m_localData.GetCategories(categories, true, true);

      bool removeComp   = false;
      auto showCompFunc = [comp, &removeComp, modifiableComp](
                              const String& headerName) -> bool {
        ImGui::PushID(static_cast<int>(comp->m_id));
        String varName =
            headerName + "##" + boolToString(modifiableComp).data();
        bool isOpen = ImGui::TreeNodeEx(
            varName.c_str(), ImGuiTreeNodeFlags_DefaultOpen | g_treeNodeFlags);

        if (modifiableComp)
        {
          float offset = ImGui::GetContentRegionAvail().x - 10.0f;
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
            ShowVariant(var, comp);
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

    // PrefabView
    //////////////////////////////////////////////////////////////////////////

    PrefabView::PrefabView()
    {
      m_viewID  = 2;
      m_viewIcn = UI::m_prefabIcn;
    }

    PrefabView::~PrefabView()
    {
    }

    bool PrefabView::DrawHeader(Entity* ntt, ImGuiTreeNodeFlags flags)
    {
      const String sId = "##" + std::to_string(ntt->GetIdVal());
      if (m_activeChildEntity == ntt)
      {
        flags |= ImGuiTreeNodeFlags_Selected;
      }
      bool isOpen = ImGui::TreeNodeEx(sId.c_str(), flags);
      if (ImGui::IsItemClicked())
      {
        m_activeChildEntity = ntt;
      }

      TexturePtr icon  = nullptr;
      EntityType eType = ntt->GetType();
      switch (eType)
      {
      case EntityType::Entity_Node:
        icon = UI::m_arrowsIcon;
        break;
      case EntityType::Entity_Prefab:
        icon = UI::m_prefabIcn;
        break;
      }

      if (icon)
      {
        ImGui::SameLine();
        ImGui::Image(Convert2ImGuiTexture(icon), ImVec2(20.0f, 20.0f));
      }

      ImGui::SameLine();
      ImGui::Text(ntt->GetNameVal().c_str());

      // Hiearchy visibility
      float offset = ImGui::GetContentRegionAvail().x - 40.0f;
      ImGui::SameLine(offset);
      icon = ntt->GetVisibleVal() ? UI::m_visibleIcon : UI::m_invisibleIcon;

      // Texture only toggle button.
      ImGui::PushID(static_cast<int>(ntt->GetIdVal()));
      if (UI::ImageButtonDecorless(
              icon->m_textureId, ImVec2(15.0f, 15.0f), false))
      {
        ntt->SetVisibility(!ntt->GetVisibleVal(), true);
      }

      ImGui::PopID();

      return isOpen;
    }

    void PrefabView::ShowNode(Entity* e)
    {
      ImGuiTreeNodeFlags nodeFlags = g_treeNodeFlags;

      if (e->m_node->m_children.empty() ||
          e->GetType() == EntityType::Entity_Prefab)
      {
        nodeFlags |=
            ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        DrawHeader(e, nodeFlags);
      }
      else
      {
        if (DrawHeader(e, nodeFlags))
        {
          for (Node* n : e->m_node->m_children)
          {
            Entity* childNtt = n->m_entity;
            if (childNtt != nullptr)
            {
              ShowNode(childNtt);
            }
          }

          ImGui::TreePop();
        }
      }
    }

    void PrefabView::Show()
    {
      Entity* cur = g_app->GetCurrentScene()->GetCurrentSelection();
      if (cur != m_entity)
      {
        m_entity            = cur;
        m_activeChildEntity = nullptr;
      }
      if (m_entity == nullptr || Prefab::GetPrefabRoot(m_entity) == nullptr)
      {
        ImGui::Text("Select a prefab entity");
        return;
      }

      // Display scene hierarchy
      if (ImGui::CollapsingHeader("Prefab Scene View",
                                  ImGuiTreeNodeFlags_DefaultOpen))
      {
        if (ImGui::BeginChild("##Prefab Scene Nodes", ImVec2(0, 200), true))
        {
          if (DrawHeader(m_entity,
                         g_treeNodeFlags | ImGuiTreeNodeFlags_DefaultOpen))
          {
            for (Node* n : m_entity->m_node->m_children)
            {
              if (n->m_entity)
              {
                ShowNode(n->m_entity);
              }
            }

            ImGui::TreePop();
          }
        }
        ImGui::EndChild();
      }

      Entity* shownEntity = m_entity;
      if (m_activeChildEntity)
      {
        shownEntity = m_activeChildEntity;
      }

      ParameterVariantRawPtrArray inheritedParams;
      shownEntity->m_localData.GetByCategory(CustomDataCategory.Name,
                                             inheritedParams);
      ShowCustomData(shownEntity, "Custom Data##1", inheritedParams, false);

      if (ImGui::CollapsingHeader("Components##1",
                                  ImGuiTreeNodeFlags_DefaultOpen))
      {
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, g_indentSpacing);

        std::vector<ULongID> compRemove;
        for (ComponentPtr& com : shownEntity->GetComponentPtrArray())
        {
          ShowComponentBlock(com, false);
        }

        ImGui::PopStyleVar();
      }
    }

    // EntityView
    //////////////////////////////////////////////////////////////////////////

    EntityView::EntityView()
    {
      m_viewID  = 1;
      m_viewIcn = UI::m_arrowsIcon;
    }
    EntityView::~EntityView()
    {
    }

    void EntityView::ShowAnchorSettings()
    {
      Surface* surface = static_cast<Surface*>(m_entity);
      Canvas* canvasPanel =
          static_cast<Canvas*>(surface->m_node->m_parent->m_entity);

      if (ImGui::CollapsingHeader("Anchor", ImGuiTreeNodeFlags_DefaultOpen))
      {
        if (ImGui::Button("Presets"))
        {
          ImGui::OpenPopup(ImGui::GetID("Preset Popup"));
        }

        if (ImGui::BeginPopup("Preset Popup"))
        {
          if (ImGui::BeginTable("Anchor Preset Table", 4))
          {
            for (uint itemIndx = 0;
                 itemIndx < UI::AnchorPresetImages::presetCount;
                 itemIndx++)
            {
              ImGui::TableNextColumn();
              ImGui::PushID(itemIndx);
              if (ImGui::ImageButton(
                      reinterpret_cast<void*>((intptr_t) UI::m_anchorPresetIcons
                                                  .m_presetImages[itemIndx]
                                                  ->m_textureId),
                      Vec2(32, 32)))
              {
                auto changeAnchor = [surface](uint anchorPreset) {
                  auto gatherAtLeft = [surface]() {
                    surface->m_anchorParams.m_anchorRatios[0] = 0.0f;
                    surface->m_anchorParams.m_anchorRatios[1] = 1.0f;
                  };
                  auto gatherAtRight = [surface]() {
                    surface->m_anchorParams.m_anchorRatios[0] = 1.0f;
                    surface->m_anchorParams.m_anchorRatios[1] = 0.0f;
                  };
                  auto gatherAtMiddle = [surface](bool horizontal) {
                    surface->m_anchorParams
                        .m_anchorRatios[(horizontal) ? 0 : 2] = 0.5f;
                    surface->m_anchorParams
                        .m_anchorRatios[(horizontal) ? 1 : 3] = 0.5f;
                  };
                  auto gatherAtTop = [surface]() {
                    surface->m_anchorParams.m_anchorRatios[2] = 0.0f;
                    surface->m_anchorParams.m_anchorRatios[3] = 1.0f;
                  };
                  auto gatherAtBottom = [surface]() {
                    surface->m_anchorParams.m_anchorRatios[2] = 1.0f;
                    surface->m_anchorParams.m_anchorRatios[3] = 0.0f;
                  };
                  auto scatterToSide = [surface](bool horizontal) {
                    surface->m_anchorParams
                        .m_anchorRatios[(horizontal) ? 0 : 2] = 0.0f;
                    surface->m_anchorParams
                        .m_anchorRatios[(horizontal) ? 1 : 3] = 0.0f;
                  };

                  uint subMode  = anchorPreset % 4;
                  uint mainMode = (anchorPreset - subMode) / 4;
                  // Horizontal Modes
                  if (mainMode < 3)
                  {
                    // 0: Left, 1: Middle, 2: Right, 3: Side-to-Side
                    uint anchorHorizontalPos = subMode;
                    // 0: Top, 1: Middle, 2: Bottom
                    uint anchorVerticalPos = mainMode;
                    switch (anchorHorizontalPos)
                    {
                    case 0:
                      gatherAtLeft();
                      break;
                    case 1:
                      gatherAtMiddle(true);
                      break;
                    case 2:
                      gatherAtRight();
                      break;
                    case 3:
                      scatterToSide(true);
                      break;
                    default:
                      assert(0 && "This shouldn't happen!");
                    }
                    switch (anchorVerticalPos)
                    {
                    case 0:
                      gatherAtTop();
                      break;
                    case 1:
                      gatherAtMiddle(false);
                      break;
                    case 2:
                      gatherAtBottom();
                      break;
                    default:
                      assert(0 && "This shouldn't happen!");
                    }
                  }
                  // Vertical Modes
                  else if (mainMode == 3)
                  {
                    // 0: Left, 1: Middle, 2: Right, 3: Whole
                    uint anchorHorizontalPos = subMode;
                    scatterToSide(false);
                    switch (anchorHorizontalPos)
                    {
                    case 0:
                      gatherAtLeft();
                      break;
                    case 1:
                      gatherAtMiddle(true);
                      break;
                    case 2:
                      gatherAtRight();
                      break;
                    case 3:
                      scatterToSide(true);
                      break;
                    default:
                      assert(0 && "This shouldn't happen!");
                    }
                  }
                };

                changeAnchor(itemIndx);
                ImGui::CloseCurrentPopup();
              }
              ImGui::PopID();
            }

            ImGui::EndTable();
          }

          ImGui::EndPopup();
        }

        const Vec2 size = {canvasPanel->GetAABB(true).GetWidth(),
                           canvasPanel->GetAABB(true).GetHeight()};
        float res[]     = {size.x, size.y};
        if (ImGui::InputFloat2("New resolution:", res))
        {
          canvasPanel->ApplyRecursiveResizePolicy(res[0], res[1]);
        }

        if (((surface->m_anchorParams.m_anchorRatios[0] +
              surface->m_anchorParams.m_anchorRatios[1]) > 0.99f) &&
            ((surface->m_anchorParams.m_anchorRatios[2] +
              surface->m_anchorParams.m_anchorRatios[3]) > 0.99f))
        {
          float position[2];
          Vec3 pos;
          float w = 0, h = 0;

          {
            pos = canvasPanel->m_node->GetTranslation(
                TransformationSpace::TS_WORLD);
            w = canvasPanel->GetSizeVal().x;
            h = canvasPanel->GetSizeVal().y;
            pos -= Vec3(w / 2.f, h / 2.f, 0.f);
            const Vec3 surfacePos =
                surface->m_node->GetTranslation(TransformationSpace::TS_WORLD);
            position[0] =
                surfacePos.x -
                (pos.x + w * surface->m_anchorParams.m_anchorRatios[0]);
            position[1] =
                surfacePos.y -
                (pos.y + h * surface->m_anchorParams.m_anchorRatios[2]);
          }
          ImGui::DragFloat("Position X", &position[0], 0.25f, pos.x, pos.x + w);
          ImGui::DragFloat("Position Y", &position[1], 0.25f, pos.y, pos.y + h);
        }
        else
        {
          ImGui::DragFloat2("Horizontal",
                            &surface->m_anchorParams.m_anchorRatios[0],
                            0.25f,
                            0.f,
                            1.f);

          ImGui::DragFloat2("Vertical",
                            &surface->m_anchorParams.m_anchorRatios[2],
                            0.25f,
                            0.f,
                            1.f);
        }

        for (int i = 0; i < 4; i++)
        {
          ImGui::DragFloat(("Ratio " + std::to_string(i)).c_str(),
                           &surface->m_anchorParams.m_anchorRatios[i],
                           0.25f,
                           0,
                           1);
        }

        if (((surface->m_anchorParams.m_anchorRatios[2] +
              surface->m_anchorParams.m_anchorRatios[3]) < 0.99f))
        {
          if (ImGui::DragFloat("Offset Top",
                               &surface->m_anchorParams.m_offsets[0]) ||
              ImGui::DragFloat("Offset Bottom",
                               &surface->m_anchorParams.m_offsets[1]))
          {
            canvasPanel->ApplyRecursiveResizePolicy(res[0], res[1]);
          }
        }

        if (((surface->m_anchorParams.m_anchorRatios[0] +
              surface->m_anchorParams.m_anchorRatios[1]) < 0.99f))
        {
          if (ImGui::DragFloat("Offset Left",
                               &surface->m_anchorParams.m_offsets[2]) ||
              ImGui::DragFloat("Offset Right",
                               &surface->m_anchorParams.m_offsets[3]))
          {
            canvasPanel->ApplyRecursiveResizePolicy(res[0], res[1]);
          }
        }

        ImGui::Separator();
      }
    }

    void EntityView::Show()
    {
      m_entity = g_app->GetCurrentScene()->GetCurrentSelection();
      if (m_entity == nullptr)
      {
        ImGui::Text("Select an entity");
        return;
      }

      ShowParameterBlock();

      // Missing data reporter.
      if (m_entity->IsDrawable())
      {
        Drawable* dw = static_cast<Drawable*>(m_entity);
        MeshPtr mesh = dw->GetMesh();

        StringArray missingData;
        MeshRawCPtrArray meshes;
        mesh->GetAllMeshes(meshes);

        for (const Mesh* subMesh : meshes)
        {
          if (!subMesh->_missingFile.empty())
          {
            missingData.push_back(subMesh->_missingFile);
          }

          if (MaterialPtr mat = subMesh->m_material)
          {
            if (!mat->_missingFile.empty())
            {
              missingData.push_back(mat->_missingFile);
            }

            if (TexturePtr tex = mat->m_diffuseTexture)
            {
              if (!tex->_missingFile.empty())
              {
                missingData.push_back(tex->_missingFile);
              }
            }
          }
        }

        if (!missingData.empty())
        {
          ImGui::Text("Missing Data: ");
          ImGui::Separator();
          ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
          for (const String& data : missingData)
          {
            ImGui::Text(data.c_str());
          }
          ImGui::PopStyleColor();
        }
      }

      if (m_entity->IsSurfaceInstance() &&
          m_entity->m_node->m_parent != nullptr &&
          m_entity->m_node->m_parent->m_entity != nullptr &&
          m_entity->m_node->m_parent->m_entity->GetType() ==
              EntityType::Entity_Canvas)
      {
        ShowAnchorSettings();
      }

      if (ImGui::CollapsingHeader("Transforms", ImGuiTreeNodeFlags_DefaultOpen))
      {
        Quaternion rotate;
        Vec3 translate;
        Mat4 ts = m_entity->m_node->GetTransform(g_app->m_transformSpace);
        DecomposeMatrix(ts, &translate, &rotate, nullptr);

        Vec3 scale = m_entity->m_node->GetScale();

        // Continuous edit utils.
        static TransformAction* dragMem = nullptr;
        const auto saveDragMemFn        = [this]() -> void {
          if (dragMem == nullptr)
          {
            dragMem = new TransformAction(m_entity);
          }
        };

        const auto saveTransformActionFn = [this]() -> void {
          if (ImGui::IsItemDeactivatedAfterEdit())
          {
            ActionManager::GetInstance()->AddAction(dragMem);
            dragMem = nullptr;
          }
        };

        TransformationSpace space = g_app->m_transformSpace;
        Vec3 newTranslate         = translate;
        if (ImGui::DragFloat3("Translate", &newTranslate[0], 0.25f))
        {
          saveDragMemFn();

          bool isDrag = ImGui::IsMouseDragging(0, 0.25f);
          if (isDrag)
          {
            m_entity->m_node->Translate(newTranslate - translate, space);
          }
          else if (!isDrag && IsTextInputFinalized())
          {
            m_entity->m_node->SetTranslation(newTranslate, space);
          }
        }

        saveTransformActionFn();

        Quaternion q0 = rotate;
        Vec3 eularXYZ = glm::eulerAngles(q0);
        Vec3 degrees  = glm::degrees(eularXYZ);
        if (ImGui::DragFloat3("Rotate", &degrees[0], 0.25f))
        {
          saveDragMemFn();

          Vec3 eular  = glm::radians(degrees);
          Vec3 change = eular - eularXYZ;

          bool isDrag = ImGui::IsMouseDragging(0, 0.25f);
          if (!isDrag)
          {
            change = eular;
          }

          Quaternion qx = glm::angleAxis(change.x, X_AXIS);
          Quaternion qy = glm::angleAxis(change.y, Y_AXIS);
          Quaternion qz = glm::angleAxis(change.z, Z_AXIS);
          Quaternion q  = qz * qy * qx;

          if (isDrag)
          {
            m_entity->m_node->Rotate(q, space);
          }
          else if (IsTextInputFinalized())
          {
            m_entity->m_node->SetOrientation(q, space);
          }
        }

        saveTransformActionFn();

        if (ImGui::DragFloat3("Scale", &scale[0], 0.1f))
        {
          bool exceed = false;
          Vec3 abScl  = glm::abs(scale);
          if (abScl.x <= 0.001f || abScl.y <= 0.001f || abScl.z <= 0.001f)
          {
            exceed = true;
          }

          if (!exceed)
          {
            saveDragMemFn();

            bool isDrag = ImGui::IsMouseDragging(0, 0.25f);
            if (isDrag || IsTextInputFinalized())
            {
              m_entity->m_node->SetScale(scale);
            }
          }
        }

        saveTransformActionFn();

        if (ImGui::Checkbox("Inherit Scale", &m_entity->m_node->m_inheritScale))
        {
          m_entity->m_node->SetInheritScaleDeep(
              m_entity->m_node->m_inheritScale);
        }

        ImGui::Separator();

        BoundingBox bb = m_entity->GetAABB(true);
        Vec3 dim       = bb.max - bb.min;
        ImGui::Text("Bounding box dimensions:");
        ImGui::Text("x: %.2f", dim.x);
        ImGui::SameLine();
        ImGui::Text("\ty: %.2f", dim.y);
        ImGui::SameLine();
        ImGui::Text("\tz: %.2f", dim.z);
      }

      ParameterVariantRawPtrArray customParams;
      m_entity->m_localData.GetByCategory(CustomDataCategory.Name,
                                          customParams);
      ShowCustomData(m_entity, "Custom Data", customParams, true);

      // If entity belongs to a prefab, don't show components
      if (Prefab::GetPrefabRoot(m_entity))
      {
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

    void EntityView::ShowParameterBlock()
    {
      VariantCategoryArray categories;
      m_entity->m_localData.GetCategories(categories, true, true);

      for (VariantCategory& category : categories)
      {
        if (category.Name == CustomDataCategory.Name)
        {
          continue;
        }

        // If entity belongs to a prefab,
        // don't show transform lock and transformation.
        bool isFromPrefab = false;
        if (Prefab::GetPrefabRoot(m_entity))
        {
          isFromPrefab = true;
        }

        String varName =
            category.Name + "##" + std::to_string(m_entity->GetIdVal());
        if (ImGui::CollapsingHeader(varName.c_str(),
                                    ImGuiTreeNodeFlags_DefaultOpen))
        {
          ParameterVariantRawPtrArray vars;
          m_entity->m_localData.GetByCategory(category.Name, vars);

          for (ParameterVariant* var : vars)
          {
            ValueUpdateFn multiUpdate = MultiUpdate(var);

            var->m_onValueChangedFn.push_back(multiUpdate);
            ShowVariant(var, nullptr);
            var->m_onValueChangedFn.pop_back();
          }
        }

        // If entity is gradient sky create a "Update IBL Textures" button
        if (m_entity->GetType() == EntityType::Entity_GradientSky &&
            category.Name.compare("Sky") == 0) // TODO(Osman) test without this
        {
          if (UI::BeginCenteredTextButton("Update IBL Textures"))
          {
            static_cast<Sky*>(m_entity)->ReInit();
          }
          UI::EndCenteredTextButton();
        }

        if (category.Name == PrefabCategory.Name)
        {
          continue;
        }
      }
    }

    // PropInspector
    //////////////////////////////////////////////////////////////////////////

    PropInspector::PropInspector(XmlNode* node) : PropInspector()
    {
      DeSerialize(nullptr, node);
    }

    PropInspector::PropInspector()
    {
      m_views.push_back(new EntityView());
      m_views.push_back(new PrefabView());
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

            if (m_activeViewIndx == viewIndx)
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
              m_activeViewIndx = viewIndx;
            }
            ImGui::PopStyleColor(1);
          }
          ImGui::EndChildFrame();
        }

        ImGui::SameLine();

        if (ImGui::BeginChild(
                ImGui::GetID("PropInspectorActiveView"),
                Vec2(windowSize.x - sidebarSize.x - spacing.x, windowSize.y)))
        {
          m_views[m_activeViewIndx]->Show();
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

  } // namespace Editor
} // namespace ToolKit
