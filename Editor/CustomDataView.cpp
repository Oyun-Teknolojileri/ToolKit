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

#include "CustomDataView.h"

#include "App.h"
#include "ComponentView.h"
#include "MultiChoiceWindow.h"
#include "Prefab.h"

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {
    // CustomDataView
    //////////////////////////////////////////////////////////////////////////
    MultiChoiceCraeteWindow CustomDataView::m_multiChoiceParamWindow;

    void CustomDataView::ShowMaterialPtr(const String& uniqueName,
                                         const String& file,
                                         MaterialPtr& var,
                                         bool isEditable)
    {
      DropSubZone(
          uniqueName,
          static_cast<uint>(UI::m_materialIcon->m_textureId),
          file,
          [&var](const DirectoryEntry& entry) -> void
          {
            if (GetResourceType(entry.m_ext) == ResourceType::Material)
            {
              var = GetMaterialManager()->Create<Material>(entry.GetFullPath());
            }
            else
            {
              GetLogger()->WriteConsole(LogType::Error, "Only Material Types are accepted.");
            }
          },
          isEditable);
    }

    void CustomDataView::ShowMaterialVariant(const String& uniqueName, const String& file, ParameterVariant* var)
    {
      DropSubZone(
          uniqueName,
          static_cast<uint>(UI::m_materialIcon->m_textureId),
          file,
          [&var](const DirectoryEntry& entry) -> void
          {
            if (GetResourceType(entry.m_ext) == ResourceType::Material)
            {
              *var = GetMaterialManager()->Create<Material>(entry.GetFullPath());
            }
            else
            {
              GetLogger()->WriteConsole(LogType::Error, "Only Material Types are accepted.");
            }
          },
          var->m_editable);
    }

    ValueUpdateFn CustomDataView::MultiUpdate(ParameterVariant* var)
    {
      EntityRawPtrArray entities;
      g_app->GetCurrentScene()->GetSelectedEntities(entities);

      // Remove current selected because its already updated.
      entities.pop_back();

      ValueUpdateFn multiUpdate = [var, entities](Value& oldVal, Value& newVal) -> void
      {
        for (Entity* ntt : entities)
        {
          // If entity is a prefab scene entity, skip it
          Prefab* prefabRoot = Prefab::GetPrefabRoot(ntt);
          if (prefabRoot && prefabRoot != ntt)
          {
            continue;
          }

          ParameterVariant* vLookUp = nullptr;
          if (ntt->m_localData.LookUp(var->m_category.Name, var->m_name, &vLookUp))
          {
            vLookUp->SetValue(newVal);
          }
        }
      };

      return multiUpdate;
    }

    bool CustomDataView::BeginShowVariants(StringView header)
    {
      if (!ImGui::BeginTable(header.data(), 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedSame))
      {
        return false;
      }
      Vec2 xSize = ImGui::CalcTextSize("Name");
      xSize      *= 3.0f;
      ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, xSize.x);
      ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

      xSize = ImGui::CalcTextSize(ICON_FA_MINUS);
      xSize *= 2.5f;
      ImGui::TableSetupColumn("##Remove", ImGuiTableColumnFlags_WidthFixed, xSize.x);

      ImGui::TableHeadersRow();

      ImGui::TableSetColumnIndex(0);
      ImGui::PushItemWidth(-FLT_MIN);

      ImGui::TableSetColumnIndex(1);
      ImGui::PushItemWidth(-FLT_MIN);
      return true;
    }

    void CustomDataView::ShowVariant(ParameterVariant* var, ParameterVariant*& remove, size_t i, bool isListEditable)
    {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);

      ImGui::PushID((int) i);
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
      case ParameterVariant::VariantType::String:
      {
        ImGui::InputText(pId.c_str(), var->GetVarPtr<String>());
      }
      break;
      case ParameterVariant::VariantType::Bool:
      {
        bool val = var->GetVar<bool>();
        if (ImGui::Checkbox(pId.c_str(), &val))
        {
          *var = val;
        }
      }
      break;
      case ParameterVariant::VariantType::Int:
      {
        ImGui::InputInt(pId.c_str(), var->GetVarPtr<int>());
      }
      break;
      case ParameterVariant::VariantType::Float:
      {
        ImGui::DragFloat(pId.c_str(), var->GetVarPtr<float>(), 0.1f);
      }
      break;
      case ParameterVariant::VariantType::Vec3:
      {
        ImGui::DragFloat3(pId.c_str(), &var->GetVar<Vec3>()[0], 0.1f);
      }
      break;
      case ParameterVariant::VariantType::Vec4:
      {
        ImGui::DragFloat4(pId.c_str(), &var->GetVar<Vec4>()[0], 0.1f);
      }
      break;
      case ParameterVariant::VariantType::Mat3:
      {
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
      case ParameterVariant::VariantType::Mat4:
      {
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
      case ParameterVariant::VariantType::MultiChoice:
      {
        MultiChoiceVariant* mcv = var->GetVarPtr<MultiChoiceVariant>();
        if (ImGui::BeginCombo("##MultiChoiceVariant", mcv->Choices[mcv->CurrentVal.Index].m_name.c_str()))
        {
          for (uint i = 0; i < mcv->Choices.size(); ++i)
          {
            bool isSelected = i == mcv->CurrentVal.Index;
            if (ImGui::Selectable(mcv->Choices[i].m_name.c_str(), isSelected))
            {
              mcv->CurrentVal = {i};
            }
          }
          ImGui::EndCombo();
        }
      }
      break;
      }

      ImGui::TableSetColumnIndex(2);
      if (isListEditable && ImGui::Button(ICON_FA_MINUS))
      {
        remove             = var;
        g_app->m_statusMsg = Format("Parameter %d: %s removed.", i + 1, var->m_name.c_str());
      }

      ImGui::PopID();
    }

    void CustomDataView::EndShowVariants()
    {
      ImGui::EndTable();
      ImGui::Separator();
    }

    void CustomDataView::ShowCustomData(Entity* m_entity,
                                        String headerName,
                                        ParameterVariantRawPtrArray& vars,
                                        bool isListEditable)
    {
      if (headerName.length() && !ImGui::CollapsingHeader(headerName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
      {
        return;
      }

      if (BeginShowVariants(headerName))
      {
        ParameterVariant* remove = nullptr;
        for (size_t i = 0; i < vars.size(); i++)
        {
          ParameterVariant* var       = vars[i];
          ValueUpdateFn multiUpdateFn = MultiUpdate(var);
          var->m_onValueChangedFn.push_back(multiUpdateFn);
          ShowVariant(var, remove, i, isListEditable);
          var->m_onValueChangedFn.pop_back();
        }

        if (remove != nullptr)
        {
          m_entity->m_localData.Remove(remove->m_id);
        }
      }
      EndShowVariants();

      static bool addInAction = false;
      if (isListEditable && addInAction)
      {
        ImGui::PushItemWidth(150);
        int dataType = 0;
        if (ImGui::Combo("##NewCustData",
                         &dataType,
                         "Sellect"
                         "Type\0String\0Boolean\0Int\0Float\0Vec2\0Vec3\0Vec4"
                         "\0Mat3\0Mat4\0MultiChoice"))
        {
          ParameterVariant customVar;
          // This makes them only visible in Custom Data dropdown.
          customVar.m_exposed  = true;
          customVar.m_editable = true;
          customVar.m_category = CustomDataCategory;

          bool added           = true;
          switch (dataType)
          {
          case 0:
            added = false;
            break;
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
            customVar = Vec2(0.0f, 0.0f);
          case 6:
            customVar = ZERO;
            break;
          case 7:
            customVar = Vec4();
            break;
          case 8:
            customVar = Mat3();
            break;
          case 9:
            customVar = Mat4();
            break;
          case 10:
            m_multiChoiceParamWindow.OpenCreateWindow(&m_entity->m_localData);
            added       = false;
            addInAction = false;
            break;
          default:
            assert(false && "invalid data type");
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

    void CustomDataView::ShowVariant(ParameterVariant* var, ComponentPtr comp)
    {
      if (!var->m_exposed)
      {
        return;
      }

      ImGui::BeginDisabled(!var->m_editable);

      static bool lastValActive = false;

      switch (var->GetType())
      {
      case ParameterVariant::VariantType::Bool:
      {
        bool val = var->GetVar<bool>();
        if (ImGui::Checkbox(var->m_name.c_str(), &val))
        {
          *var = val;
        }
      }
      break;
      case ParameterVariant::VariantType::Float:
      {
        static float lastVal = 0.0f;
        float val            = var->m_hint.waitForTheEndOfInput && lastValActive ? lastVal : var->GetVar<float>();

        if (!var->m_hint.isRangeLimited)
        {
          if (ImGui::InputFloat(var->m_name.c_str(), &val))
          {
            *var = val;
          }
        }
        else
        {
          bool dragged = false;
          if (ImGui::DragFloat(var->m_name.c_str(),
                               &val,
                               var->m_hint.increment,
                               var->m_hint.rangeMin,
                               var->m_hint.rangeMax))
          {
            if (!var->m_hint.waitForTheEndOfInput)
            {
              *var = val;
            }
            else
            {
              lastVal       = val;
              lastValActive = true;
            }
          }

          if (var->m_hint.waitForTheEndOfInput && ImGui::IsItemDeactivatedAfterEdit())
          {
            *var          = lastVal;
            lastValActive = false;
          }
        }
      }
      break;
      case ParameterVariant::VariantType::Int:
      {
        static int lastVal = 0;
        int val            = var->m_hint.waitForTheEndOfInput && lastValActive ? lastVal : var->GetVar<int>();

        if (var->m_hint.isRangeLimited)
        {
          bool dragged = false;
          if (ImGui::DragInt(var->m_name.c_str(),
                             &val,
                             var->m_hint.increment,
                             static_cast<int>(var->m_hint.rangeMin),
                             static_cast<int>(var->m_hint.rangeMax)))
          {
            if (!var->m_hint.waitForTheEndOfInput)
            {
              *var = val;
            }
            else
            {
              lastVal       = val;
              lastValActive = true;
            }
          }

          if (var->m_hint.waitForTheEndOfInput && ImGui::IsItemDeactivatedAfterEdit())
          {
            *var          = lastVal;
            lastValActive = false;
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
      case ParameterVariant::VariantType::Vec2:
      {
        static Vec2 lastVal = Vec2(0.0f);
        Vec2 val            = var->m_hint.waitForTheEndOfInput && lastValActive ? lastVal : var->GetVar<Vec2>();

        if (var->m_hint.isRangeLimited)
        {
          bool dragged = false;
          if (ImGui::DragFloat2(var->m_name.c_str(),
                                &val[0],
                                var->m_hint.increment,
                                var->m_hint.rangeMin,
                                var->m_hint.rangeMax))
          {
            if (!var->m_hint.waitForTheEndOfInput)
            {
              *var = val;
            }
            else
            {
              lastVal       = val;
              lastValActive = true;
            }
          }

          if (var->m_hint.waitForTheEndOfInput && ImGui::IsItemDeactivatedAfterEdit())
          {
            *var          = lastVal;
            lastValActive = false;
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
      case ParameterVariant::VariantType::Vec3:
      {
        Vec3 val = var->GetVar<Vec3>();
        if (var->m_hint.isColor)
        {
          if (ImGui::ColorEdit3(var->m_name.c_str(), &val[0], ImGuiColorEditFlags_NoLabel))
          {
            *var = val;
          }
        }
        else if (var->m_hint.isRangeLimited)
        {
          static Vec3 lastVal = Vec3(0.0f);
          val                 = var->m_hint.waitForTheEndOfInput && lastValActive ? lastVal : var->GetVar<Vec3>();

          if (ImGui::DragFloat3(var->m_name.c_str(),
                                &val[0],
                                var->m_hint.increment,
                                var->m_hint.rangeMin,
                                var->m_hint.rangeMax))
          {
            if (!var->m_hint.waitForTheEndOfInput)
            {
              *var = val;
            }
            else
            {
              lastVal       = val;
              lastValActive = true;
            }
          }

          if (var->m_hint.waitForTheEndOfInput && ImGui::IsItemDeactivatedAfterEdit())
          {
            *var          = lastVal;
            lastValActive = false;
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
      case ParameterVariant::VariantType::Vec4:
      {
        Vec4 val = var->GetVar<Vec4>();
        if (var->m_hint.isColor)
        {
          if (ImGui::ColorEdit4(var->m_name.c_str(), &val[0], ImGuiColorEditFlags_NoLabel))
          {
            *var = val;
          }
        }
        else if (var->m_hint.isRangeLimited)
        {
          static Vec4 lastVal = Vec4(0.0f);
          val                 = var->m_hint.waitForTheEndOfInput && lastValActive ? lastVal : var->GetVar<Vec4>();

          if (ImGui::DragFloat4(var->m_name.c_str(),
                                &val[0],
                                var->m_hint.increment,
                                var->m_hint.rangeMin,
                                var->m_hint.rangeMax))
          {
            if (!var->m_hint.waitForTheEndOfInput)
            {
              *var = val;
            }
            else
            {
              lastVal       = val;
              lastValActive = true;
            }
          }

          if (var->m_hint.waitForTheEndOfInput && ImGui::IsItemDeactivatedAfterEdit())
          {
            *var          = lastVal;
            lastValActive = false;
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
      case ParameterVariant::VariantType::String:
      {
        String val = var->GetVar<String>();
        if (ImGui::InputText(var->m_name.c_str(), &val) && IsTextInputFinalized())
        {
          *var = val;
        }
      }
      break;
      case ParameterVariant::VariantType::ULongID:
      {
        ULongID val = var->GetVar<ULongID>();
        if (ImGui::InputScalar(var->m_name.c_str(), ImGuiDataType_U32, var->GetVarPtr<ULongID>()) &&
            IsTextInputFinalized())
        {
          *var = val;
        }
      }
      break;
      case ParameterVariant::VariantType::MaterialPtr:
      {
        MaterialPtr& mref = var->GetVar<MaterialPtr>();
        String file, id;
        if (mref)
        {
          id   = std::to_string(mref->m_id);
          file = mref->GetFile();
        }

        String uniqueName = var->m_name + "##" + id;
        ImGui::EndDisabled();
        ShowMaterialVariant(uniqueName, file, var);
        ImGui::BeginDisabled(!var->m_editable);
      }
      break;
      case ParameterVariant::VariantType::MeshPtr:
      {
        MeshPtr mref = var->GetVar<MeshPtr>();
        ImGui::EndDisabled();
        DropSubZone(
            "Mesh##" + std::to_string(mref->m_id),
            static_cast<uint>(UI::m_meshIcon->m_textureId),
            mref->GetFile(),
            [&var](const DirectoryEntry& entry) -> void
            {
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
                GetLogger()->WriteConsole(LogType::Error, "Only meshes are accepted.");
              }
            },
            var->m_editable);
        ImGui::BeginDisabled(!var->m_editable);
      }
      break;
      case ParameterVariant::VariantType::HdriPtr:
      {
        HdriPtr mref = var->GetVar<HdriPtr>();
        String file, id;
        if (mref)
        {
          id   = std::to_string(mref->m_id);
          file = mref->GetFile();
        }

        ImGui::EndDisabled();
        DropSubZone(
            "Hdri##" + id,
            UI::m_imageIcon->m_textureId,
            file,
            [&var](const DirectoryEntry& entry) -> void
            {
              if (GetResourceType(entry.m_ext) == ResourceType::Hdri)
              {
                *var = GetTextureManager()->Create<Hdri>(entry.GetFullPath());
              }
              else
              {
                GetLogger()->WriteConsole(LogType::Error, "Only hdri's are accepted.");
              }
            },
            var->m_editable);
        ImGui::BeginDisabled(!var->m_editable);
      }
      break;
      case ParameterVariant::VariantType::SkeletonPtr:
      {
        SkeletonPtr mref = var->GetVar<SkeletonPtr>();
        String file, id;
        if (mref)
        {
          id   = std::to_string(mref->m_id);
          file = mref->GetFile();
        }

        auto dropZoneFnc = [&var, &comp](const DirectoryEntry& entry) -> void
        {
          if (GetResourceType(entry.m_ext) == ResourceType::Skeleton)
          {
            *var = GetSkeletonManager()->Create<Skeleton>(entry.GetFullPath());
            if (SkeletonComponent* scom = comp->As<SkeletonComponent>())
            {
              scom->Init();
            }
          }
          else
          {
            GetLogger()->WriteConsole(LogType::Error, "Only skeletons are accepted.");
          }
        };
        ImGui::EndDisabled();
        DropSubZone("Skeleton##" + id, UI::m_boneIcon->m_textureId, file, dropZoneFnc, var->m_editable);
        ImGui::BeginDisabled(!var->m_editable);
      }
      break;
      case ParameterVariant::VariantType::AnimRecordPtrMap:
      {
        ComponentView::ShowAnimControllerComponent(var, comp);
      }
      break;
      case ParameterVariant::VariantType::VariantCallback:
      {
        if (UI::BeginCenteredTextButton(var->m_name))
        {
          VariantCallback callback = var->GetVar<VariantCallback>();
          callback();
        }
        UI::EndCenteredTextButton();
      }
      break;
      case ParameterVariant::VariantType::MultiChoice:
      {
        MultiChoiceVariant* mcv = var->GetVarPtr<MultiChoiceVariant>();
        if (ImGui::BeginCombo("##MultiChoiceVariant", mcv->Choices[mcv->CurrentVal.Index].m_name.c_str()))
        {
          for (uint i = 0; i < mcv->Choices.size(); ++i)
          {
            bool isSelected = i == mcv->CurrentVal.Index;
            if (ImGui::Selectable(mcv->Choices[i].m_name.c_str(), isSelected))
            {
              mcv->CurrentVal = {i};
            }
          }
          ImGui::EndCombo();
        }
      }
      break;
      default:
        break;
      }

      ImGui::EndDisabled();
    }

    CustomDataView::CustomDataView() : View("Custom Data View")
    {
      m_viewID  = 4;
      m_viewIcn = UI::m_objectDataIcon;
    }

    CustomDataView::~CustomDataView() {}

    void CustomDataView::Show()
    {
      m_entity = g_app->GetCurrentScene()->GetCurrentSelection();
      if (m_entity == nullptr)
      {
        ImGui::Text("Select an entity");
        return;
      }

      ParameterVariantRawPtrArray customParams;
      m_entity->m_localData.GetByCategory(CustomDataCategory.Name, customParams);
      ShowCustomData(m_entity, "Custom Data", customParams, true);
    }

  } // namespace Editor
} // namespace ToolKit