#include "EntityView.h"

#include "App.h"
#include "CustomDataView.h"
#include "Prefab.h"
#include "TransformMod.h"

namespace ToolKit
{
  namespace Editor
  {
    // EntityView
    //////////////////////////////////////////////////////////////////////////

    EntityView::EntityView() : View("Entity View")
    {
      m_viewID  = 1;
      m_viewIcn = UI::m_arrowsIcon;
    }

    EntityView::~EntityView() {}

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
                auto changeAnchor = [surface](uint anchorPreset)
                {
                  auto gatherAtLeft = [surface]()
                  {
                    surface->m_anchorParams.m_anchorRatios[0] = 0.0f;
                    surface->m_anchorParams.m_anchorRatios[1] = 1.0f;
                  };
                  auto gatherAtRight = [surface]()
                  {
                    surface->m_anchorParams.m_anchorRatios[0] = 1.0f;
                    surface->m_anchorParams.m_anchorRatios[1] = 0.0f;
                  };
                  auto gatherAtMiddle = [surface](bool horizontal)
                  {
                    surface->m_anchorParams
                        .m_anchorRatios[(horizontal) ? 0 : 2] = 0.5f;
                    surface->m_anchorParams
                        .m_anchorRatios[(horizontal) ? 1 : 3] = 0.5f;
                  };
                  auto gatherAtTop = [surface]()
                  {
                    surface->m_anchorParams.m_anchorRatios[2] = 0.0f;
                    surface->m_anchorParams.m_anchorRatios[3] = 1.0f;
                  };
                  auto gatherAtBottom = [surface]()
                  {
                    surface->m_anchorParams.m_anchorRatios[2] = 1.0f;
                    surface->m_anchorParams.m_anchorRatios[3] = 0.0f;
                  };
                  auto scatterToSide = [surface](bool horizontal)
                  {
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
                    uint anchorVerticalPos   = mainMode;
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
            w   = canvasPanel->GetSizeVal().x;
            h   = canvasPanel->GetSizeVal().y;
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

        ImGui::BeginDisabled(m_entity->GetTransformLockVal());

        // Continuous edit utils.
        static TransformAction* dragMem = nullptr;
        const auto saveDragMemFn        = [this]() -> void
        {
          if (dragMem == nullptr)
          {
            dragMem = new TransformAction(m_entity);
          }
        };

        const auto saveTransformActionFn = [this]() -> void
        {
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

        ImGui::EndDisabled();

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
        if (m_entity->GetPrefabRoot())
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
            ValueUpdateFn multiUpdate = CustomDataView::MultiUpdate(var);

            var->m_onValueChangedFn.push_back(multiUpdate);
            CustomDataView::ShowVariant(var, nullptr);
            var->m_onValueChangedFn.pop_back();
          }
        }

        // If entity is gradient sky create a "Update IBL Textures" button
        if (m_entity->GetType() == EntityType::Entity_GradientSky &&
            category.Name.compare("Sky") ==
                0) // TODO This might not be necessary
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

  } // namespace Editor
} // namespace ToolKit