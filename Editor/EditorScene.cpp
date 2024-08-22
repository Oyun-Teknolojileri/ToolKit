/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "EditorScene.h"

#include "Action.h"
#include "App.h"
#include "Prefab.h"

namespace ToolKit
{
  namespace Editor
  {

    TKDefineClass(EditorScene, Scene);

    EditorScene::EditorScene() { m_newScene = false; }

    EditorScene::~EditorScene() { Destroy(false); }

    void EditorScene::Load()
    {
      Scene::Load();

      for (EntityPtr& ntt : m_entities)
      {
        // Create gizmos
        if (ntt->IsA<DirectionalLight>())
        {
          ntt->As<EditorDirectionalLight>()->InitController();
        }
        else if (ntt->IsA<PointLight>())
        {
          ntt->As<EditorPointLight>()->InitController();
        }
        else if (ntt->IsA<SpotLight>())
        {
          ntt->As<EditorSpotLight>()->InitController();
        }
      }
    }

    void EditorScene::Update(float deltaTime)
    {
      Super::Update(deltaTime);

      // Show selected light gizmos.
      EntityPtrArray selecteds;
      GetSelectedEntities(selecteds);

      LightPtrArray sceneLights   = GetLights();

      bool foundFirstLight        = false;
      LightPtr firstSelectedLight = nullptr;
      for (LightPtr light : sceneLights)
      {
        bool found = false;
        for (EntityPtr ntt : selecteds)
        {
          if (light->GetIdVal() == ntt->GetIdVal())
          {
            if (!foundFirstLight)
            {
              firstSelectedLight = light;
              foundFirstLight    = true;
            }
            EnableLightGizmo(light.get(), true);
            found = true;
            break;
          }
        }

        if (!found)
        {
          EnableLightGizmo(light.get(), false);
        }
      }

      // Update billboards attached to entities.
      for (EntityPtr billboard : m_billboards)
      {
        BillboardPtr bb     = Cast<Billboard>(billboard);
        bb->m_worldLocation = bb->m_entity->m_node->GetTranslation();
      }
    }

    bool EditorScene::IsSelected(ULongID id) const
    {
      return std::find(m_selectedEntities.begin(), m_selectedEntities.end(), id) != m_selectedEntities.end();
    }

    void EditorScene::RemoveFromSelection(ULongID id)
    {
      auto nttIt = std::find(m_selectedEntities.begin(), m_selectedEntities.end(), id);
      if (nttIt != m_selectedEntities.end())
      {
        m_selectedEntities.erase(nttIt);
      }
    }

    void EditorScene::AddToSelection(ULongID id, bool additive)
    {
      if (!additive)
      {
        m_selectedEntities.clear();
      }

      if (IsSelected(id))
      {
        if (!additive) // we've cleared the array, must add again
        {
          m_selectedEntities.push_back(id);
        }
        return;
      }

      EntityPtr ntt = GetEntity(id);

      // If the entity comes from a prefab scene, swap the child ntt with prefab.
      if (Entity* prefabRoot = ntt->GetPrefabRoot())
      {
        ntt = prefabRoot->Self<Entity>();
        id  = ntt->GetIdVal();
      }

      if (g_app->m_selectEffectingLights && !ntt->IsA<Light>())
      {
        LightPtrArray lights = GetLights();
        for (LightPtr& light : lights)
        {
          light->UpdateShadowCamera();
        }

        RenderJobArray jobs;
        RenderJobProcessor::CreateRenderJobs(jobs, {ntt.get()});

        if (!jobs.empty())
        {
          RenderJobProcessor::AssignLight(jobs.begin(), jobs.end(), lights);
          RenderJob& job = jobs.front();

          for (int i = 0; i < job.lights.size(); i++)
          {
            Light* light = job.lights[i];
            if (!IsSelected(light->GetIdVal()))
            {
              AddToSelection(light->GetIdVal(), true);
            }
          }
        }
      }

      m_selectedEntities.push_back(id);
    }

    void EditorScene::AddToSelection(const IDArray& entities, bool additive)
    {
      ULongID currentId = NULL_HANDLE;
      if (entities.size() > 1)
      {
        for (const ULongID& id : entities)
        {
          if (id != NULL_HANDLE)
          {
            if (IsCurrentSelection(id))
            {
              currentId = id;
            }
          }
        }
      }

      // Start with a clear selection list.
      if (!additive)
      {
        ClearSelection();
      }

      for (const ULongID& id : entities)
      {
        if (id == NULL_HANDLE)
        {
          continue;
        }

        // Add to selection.
        if (!additive)
        {
          AddToSelection(id, true);
        }
        else // Add, make current or toggle selection.
        {
          if (IsSelected(id))
          {
            if (GetSelectedEntityCount() > 1)
            {
              if (entities.size() == 1)
              {
                if (IsCurrentSelection(id))
                {
                  RemoveFromSelection(id);
                }
                else
                {
                  MakeCurrentSelection(id, false);
                }
              }
            }
            else
            {
              RemoveFromSelection(id);
            }
          }
          else
          {
            AddToSelection(id, true);
          }
        }
      }

      MakeCurrentSelection(currentId, true);
    }

    void EditorScene::AddToSelection(const EntityPtrArray& entities, bool additive)
    {
      IDArray ids;
      ToEntityIdArray(ids, entities);
      AddToSelection(ids, additive);
    }

    void EditorScene::ClearSelection() { m_selectedEntities.clear(); }

    bool EditorScene::IsCurrentSelection(ULongID id) const
    {
      if (!m_selectedEntities.empty())
      {
        return m_selectedEntities.back() == id;
      }

      return false;
    }

    void EditorScene::MakeCurrentSelection(ULongID id, bool ifExist)
    {
      IDArray::iterator itr = std::find(m_selectedEntities.begin(), m_selectedEntities.end(), id);
      if (itr != m_selectedEntities.end())
      {
        std::iter_swap(itr, m_selectedEntities.end() - 1);
      }
      else
      {
        if (!ifExist)
        {
          m_selectedEntities.push_back(id);
        }
      }
    }

    uint EditorScene::GetSelectedEntityCount() const { return (uint) m_selectedEntities.size(); }

    EntityPtr EditorScene::GetCurrentSelection() const
    {
      if (!m_selectedEntities.empty())
      {
        return GetEntity(m_selectedEntities.back());
      }

      return nullptr;
    }

    void EditorScene::Save(bool onlyIfDirty)
    {
      Scene::Save(onlyIfDirty);
      m_newScene = false;
    }

    void EditorScene::AddEntity(EntityPtr entity, int index)
    {
      Scene::AddEntity(entity, index);

      // Add billboard gizmo.
      AddBillboard(entity);
    }

    void EditorScene::RemoveEntity(const EntityPtrArray& entities, bool deep)
    {
      Scene::RemoveEntity(entities, deep);

      for (EntityPtr ntt : entities)
      {
        RemoveFromSelection(ntt->GetIdVal());
        RemoveBillboard(ntt);
      }
    }

    EntityPtr EditorScene::RemoveEntity(ULongID id, bool deep)
    {
      EntityPtr removed = nullptr;
      if (removed = Scene::RemoveEntity(id, deep))
      {
        RemoveFromSelection(removed->GetIdVal());
        RemoveBillboard(removed);
      }

      return removed;
    }

    void EditorScene::Destroy(bool removeResources)
    {
      ActionManager::GetInstance()->ClearAllActions();

      // If current scene getting destroyed, clear outliner.
      if (ScenePtr curr = GetSceneManager()->GetCurrentScene())
      {
        if (curr->IsSame(this))
        {
          if (OutlinerWindowPtr wnd = g_app->GetOutliner())
          {
            wnd->ClearOutliner();
          }
        }
      }

      Scene::Destroy(removeResources);

      m_selectedEntities.clear();

      // Destroy gizmos too.
      m_entityBillboardMap.clear();
      m_billboards.clear();
    }

    void EditorScene::GetSelectedEntities(EntityPtrArray& entities) const
    {
      for (ULongID id : m_selectedEntities)
      {
        EntityPtr ntt = GetEntity(id);
        assert(ntt != nullptr && "Null entity found in the scene.");

        entities.push_back(ntt);
      }
    }

    void EditorScene::GetSelectedEntities(IDArray& entities) const { entities = m_selectedEntities; }

    void EditorScene::SelectByTag(const String& tag) { AddToSelection(GetByTag(tag), false); }

    Scene::PickData EditorScene::PickObject(Ray ray, const IDArray& ignoreList, const EntityPtrArray& extraList)
    {
      // Add billboards to scene
      EntityPtrArray temp = extraList;
      temp.insert(temp.end(), m_billboards.begin(), m_billboards.end());
      UpdateBillboardsForPicking();

      Scene::PickData pdata = Scene::PickObject(ray, ignoreList, temp);

      // If the billboards are picked, pick the entity
      if (pdata.entity != nullptr && pdata.entity->IsA<Billboard>() &&
          static_cast<Billboard*>(pdata.entity.get())->m_entity != nullptr)
      {
        pdata.entity = static_cast<Billboard*>(pdata.entity.get())->m_entity;
      }

      return pdata;
    }

    void EditorScene::PickObject(const Frustum& frustum,
                                 PickDataArray& pickedObjects,
                                 const IDArray& ignoreList,
                                 const EntityPtrArray& extraList,
                                 bool pickPartiallyInside)
    {
      // Add billboards to scene
      EntityPtrArray temp = extraList;
      temp.insert(temp.end(), m_billboards.begin(), m_billboards.end());
      UpdateBillboardsForPicking();

      Scene::PickObject(frustum, pickedObjects, ignoreList, temp, pickPartiallyInside);

      // If the billboards are picked, pick the entity.
      pickedObjects.erase(std::remove_if(pickedObjects.begin(),
                                         pickedObjects.end(),
                                         [&pickedObjects](PickData& pd) -> bool
                                         {
                                           if (pd.entity != nullptr && pd.entity->IsA<Billboard>() &&
                                               static_cast<Billboard*>(pd.entity.get())->m_entity != nullptr)
                                           {
                                             // Check if the entity is already picked
                                             bool found = false;
                                             for (PickData& pd2 : pickedObjects)
                                             {
                                               if (pd2.entity->GetIdVal() ==
                                                   static_cast<Billboard*>(pd.entity.get())->m_entity->GetIdVal())
                                               {
                                                 found = true;
                                                 break;
                                               }
                                             }

                                             if (found)
                                             {
                                               return true;
                                             }
                                             else
                                             {
                                               pd.entity = static_cast<Billboard*>(pd.entity.get())->m_entity;
                                               return false;
                                             }
                                           }
                                           return false;
                                         }),
                          pickedObjects.end());
    }

    void EditorScene::AddBillboard(EntityPtr entity)
    {
      auto addBillboardFn = [this, &entity](EditorBillboardPtr billboard)
      {
        if (m_entityBillboardMap.find(entity) != m_entityBillboardMap.end())
        {
          RemoveBillboard(entity);
        }

        // Note: Only 1 billboard per entity is supported.
        billboard->m_entity          = entity;
        m_entityBillboardMap[entity] = billboard;
        m_billboards.push_back(billboard);
      };

      // Check environment component
      bool envExist = entity->GetComponent<EnvironmentComponent>() != nullptr;
      if (envExist || entity->IsA<Sky>())
      {
        SkyBillboardPtr billboard = MakeNewPtr<SkyBillboard>();
        addBillboardFn(billboard);
        return;
      }

      // Check light
      if (entity->IsA<Light>())
      {
        LightBillboardPtr billboard = MakeNewPtr<LightBillboard>();
        addBillboardFn(billboard);
        return;
      }
    }

    void EditorScene::RemoveBillboard(EntityPtr entity)
    {
      if (m_entityBillboardMap.find(entity) != m_entityBillboardMap.end())
      {
        EntityPtr billboard = m_entityBillboardMap[entity];
        m_entityBillboardMap.erase(entity);
        for (auto it = m_billboards.begin(); it != m_billboards.end(); ++it)
        {
          if ((*it)->GetIdVal() == billboard->GetIdVal())
          {
            m_billboards.erase(it);
            break;
          }
        }
      }
    }

    EntityPtrArray EditorScene::GetBillboards() { return m_billboards; }

    EntityPtr EditorScene::GetBillboard(EntityPtr entity)
    {
      auto billBoard = m_entityBillboardMap.find(entity);
      if (billBoard != m_entityBillboardMap.end())
      {
        return billBoard->second;
      }
      else
      {
        return nullptr;
      }
    }

    void EditorScene::ValidateBillboard(EntityPtr entity)
    {
      bool addBillboard = false;
      auto billMap      = m_entityBillboardMap.find(entity);

      auto sanitizeFn   = [&addBillboard, billMap, this](EditorBillboardBase::BillboardType type) -> void
      {
        if (billMap != m_entityBillboardMap.end())
        {
          // Wrong type.
          if (billMap->second->GetBillboardType() != type)
          {
            // Override it with sky billboard.
            addBillboard = true;
          }
        }
        else
        {
          // Missing.
          addBillboard = true;
        }
      };

      // Check Sky billboard. Precedence over light billboard.
      if (entity->GetComponent<EnvironmentComponent>())
      {
        sanitizeFn(EditorBillboardBase::BillboardType::Sky);
      }
      else if (entity->IsA<Sky>())
      {
        sanitizeFn(EditorBillboardBase::BillboardType::Sky);
      }
      else if (entity->IsA<Light>())
      {
        sanitizeFn(EditorBillboardBase::BillboardType::Light);
      }
      else
      {
        if (billMap != m_entityBillboardMap.end())
        {
          // This entity should not have a billboard.
          RemoveBillboard(entity);
        }
      }

      if (addBillboard)
      {
        AddBillboard(entity);
      }
    }

    void EditorScene::ValidateBillboard(EntityPtrArray& entities)
    {
      for (EntityPtr ntt : entities)
      {
        ValidateBillboard(ntt);
      }
    }

    void EditorScene::CopyTo(Resource* other)
    {
      EditorScene* cpy = static_cast<EditorScene*>(other);
      Scene::CopyTo(cpy);
      cpy->m_newScene = true;
    }

    void EditorScene::UpdateBillboardsForPicking()
    {
      if (EditorViewportPtr vp = g_app->GetActiveViewport())
      {
        if (CameraPtr cam = vp->GetCamera())
        {
          for (EntityPtr billboard : m_billboards)
          {
            static_cast<Billboard*>(billboard.get())->LookAt(cam, vp->GetBillboardScale());
          }
        }
      }
    }

    EditorSceneManager::EditorSceneManager() {}

    EditorSceneManager::~EditorSceneManager() {}

  } // namespace Editor
} // namespace ToolKit
