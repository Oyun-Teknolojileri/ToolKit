#include "EditorScene.h"

#include "App.h"
#include "EditorCamera.h"
#include "EditorViewport.h"
#include "Global.h"
#include "Prefab.h"
#include "Util.h"

#include "DebugNew.h"
#include "Action.h"

namespace ToolKit
{
  namespace Editor
  {

    EditorScene::EditorScene() : Scene() { m_newScene = true; }

    EditorScene::EditorScene(const String& file) : EditorScene()
    {
      SetFile(file);
      m_newScene = false;
    }

    EditorScene::~EditorScene() { Destroy(false); }

    void EditorScene::Load()
    {
      Scene::Load();

      for (Entity* ntt : m_entities)
      {
        // Add billboards
        AddBillboardToEntity(ntt);

        // Create gizmos
        if (ntt->GetType() == EntityType::Entity_DirectionalLight)
        {
          static_cast<EditorDirectionalLight*>(ntt)->Init();
        }
        else if (ntt->GetType() == EntityType::Entity_PointLight)
        {
          static_cast<EditorPointLight*>(ntt)->Init();
        }
        else if (ntt->GetType() == EntityType::Entity_SpotLight)
        {
          static_cast<EditorSpotLight*>(ntt)->Init();
        }
      }
    }

    void EditorScene::Update(float deltaTime)
    {
      // Update animations.
      GetAnimationPlayer()->Update(MillisecToSec(deltaTime));

      // Show selected light gizmos.
      EntityRawPtrArray selecteds;
      GetSelectedEntities(selecteds);

      LightRawPtrArray sceneLights = GetLights();

      bool foundFirstLight         = false;
      Light* firstSelectedLight    = nullptr;
      for (Light* light : sceneLights)
      {
        bool found = false;
        for (Entity* ntt : selecteds)
        {
          if (light->GetIdVal() == ntt->GetIdVal())
          {
            if (!foundFirstLight)
            {
              firstSelectedLight = light;
              foundFirstLight    = true;
            }
            EnableLightGizmo(light, true);
            found = true;
            break;
          }
        }

        if (!found)
        {
          EnableLightGizmo(light, false);
        }
      }

      // Update billboards attached to entities.
      for (Billboard* billboard : m_billboards)
      {
        billboard->m_worldLocation =
            billboard->m_entity->m_node->GetTranslation();
      }
    }

    bool EditorScene::IsSelected(ULongID id) const
    {
      return std::find(m_selectedEntities.begin(),
                       m_selectedEntities.end(),
                       id) != m_selectedEntities.end();
    }

    void EditorScene::RemoveFromSelection(ULongID id)
    {
      auto nttIt =
          std::find(m_selectedEntities.begin(), m_selectedEntities.end(), id);
      if (nttIt != m_selectedEntities.end())
      {
        m_selectedEntities.erase(nttIt);
      }
    }

    void TraverseChildNodes(Node* parent,
                            const std::function<void(Node* node)>& callbackFn)
    {
      for (Node* childNode : parent->m_children)
      {
        TraverseChildNodes(childNode, callbackFn);
      }
      callbackFn(parent);
    }

    void EditorScene::AddToSelection(ULongID id, bool additive)
    {
      if (IsSelected(id))
      {
        GetLogger()->WriteConsole(LogType::Error, "%d already selected !", id);
      }

      if (!additive)
      {
        m_selectedEntities.clear();
      }

      Entity* ntt = GetEntity(id);

      // If selected entity belongs to a prefab
      //  select all childs of the prefab entity too
      if (Prefab* mainPrefab = Prefab::GetPrefabRoot(ntt))
      {
        auto addToSelectionFn = [this](Node* node)
        { m_selectedEntities.push_back(node->m_entity->GetIdVal()); };
        TraverseChildNodes(mainPrefab->m_node, addToSelectionFn);
        return;
      }

      if (g_app->m_selectEffectingLights && !ntt->IsLightInstance())
      {
        LightRawPtrArray lights          = GetLights();
        LightRawPtrArray effectingLights = GetBestLights(ntt, lights);

        for (Light* light : effectingLights)
        {
          if (!IsSelected(light->GetIdVal()))
          {
            AddToSelection(light->GetIdVal(), true);
          }
        }
      }

      m_selectedEntities.push_back(id);
    }

    void EditorScene::AddToSelection(const EntityIdArray& entities,
                                     bool additive)
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

    void EditorScene::AddToSelection(const EntityRawPtrArray& entities,
                                     bool additive)
    {
      EntityIdArray ids;
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
      EntityIdArray::iterator itr =
          std::find(m_selectedEntities.begin(), m_selectedEntities.end(), id);
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

    uint EditorScene::GetSelectedEntityCount() const
    {
      return (uint) m_selectedEntities.size();
    }

    Entity* EditorScene::GetCurrentSelection() const
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

    void EditorScene::AddEntity(Entity* entity)
    {
      Scene::AddEntity(entity);

      // Add bilboard gizmo
      AddBillboardToEntity(entity);
    }

    void EditorScene::RemoveEntity(const EntityRawPtrArray& entities) 
    {
      Scene::RemoveEntity(entities);
      
      // TODO: Anil
      // Cache billboards with ntt id
      // Can we do remove if rather than erasing one by one.
      for (Entity* ntt : entities) 
      {
        RemoveFromSelection(ntt->GetIdVal());
        RemoveBillboardFromEntity(ntt);
      }
    }

    Entity* EditorScene::RemoveEntity(ULongID id)
    {
      Entity* removed = nullptr;
      if ((removed = Scene::RemoveEntity(id)))
      {
        RemoveFromSelection(removed->GetIdVal());
        RemoveBillboardFromEntity(removed);
      }

      return removed;
    }

    void EditorScene::Destroy(bool removeResources)
    {
      Scene::Destroy(removeResources);
      ActionManager::GetInstance()->ClearAllActions();
      m_selectedEntities.clear();

      // Destroy gizmos too
      for (auto it = m_billboards.begin(); it != m_billboards.end(); ++it)
      {
        SafeDel(*it);
      }
      m_entityBillboardMap.clear();
      m_billboards.clear();
    }

    void EditorScene::GetSelectedEntities(EntityRawPtrArray& entities) const
    {
      for (ULongID id : m_selectedEntities)
      {
        Entity* e = GetEntity(id);
        assert(e);

        entities.push_back(e);
      }
    }

    void EditorScene::GetSelectedEntities(EntityIdArray& entities) const
    {
      entities = m_selectedEntities;
    }

    void EditorScene::SelectByTag(const String& tag)
    {
      AddToSelection(GetByTag(tag), false);
    }

    Scene::PickData EditorScene::PickObject(Ray ray,
                                            const EntityIdArray& ignoreList,
                                            const EntityRawPtrArray& extraList)
    {
      // Add billboards to scene
      EntityRawPtrArray temp = extraList;
      temp.insert(temp.end(), m_billboards.begin(), m_billboards.end());

      Scene::PickData pdata = Scene::PickObject(ray, ignoreList, temp);

      // If the billboards are picked, pick the entity
      if (pdata.entity != nullptr &&
          pdata.entity->GetType() == EntityType::Entity_Billboard &&
          static_cast<Billboard*>(pdata.entity)->m_entity != nullptr)
      {
        pdata.entity = static_cast<Billboard*>(pdata.entity)->m_entity;
      }

      return pdata;
    }

    void EditorScene::PickObject(const Frustum& frustum,
                                 std::vector<PickData>& pickedObjects,
                                 const EntityIdArray& ignoreList,
                                 const EntityRawPtrArray& extraList,
                                 bool pickPartiallyInside)
    {
      // Add billboards to scene
      EntityRawPtrArray temp = extraList;
      temp.insert(temp.end(), m_billboards.begin(), m_billboards.end());

      Scene::PickObject(frustum,
                        pickedObjects,
                        ignoreList,
                        temp,
                        pickPartiallyInside);

      // If the billboards are picked, pick the entity

      // Dropout non visible & drawable entities.
      pickedObjects.erase(
          std::remove_if(
              pickedObjects.begin(),
              pickedObjects.end(),
              [&pickedObjects](PickData& pd) -> bool
              {
                if (pd.entity != nullptr &&
                    pd.entity->GetType() == EntityType::Entity_Billboard &&
                    static_cast<Billboard*>(pd.entity)->m_entity != nullptr)
                {
                  // Check if the entity is already picked
                  bool found = false;
                  for (PickData& pd2 : pickedObjects)
                  {
                    if (pd2.entity->GetIdVal() ==
                        static_cast<Billboard*>(pd.entity)
                            ->m_entity->GetIdVal())
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
                    pd.entity = static_cast<Billboard*>(pd.entity)->m_entity;
                    return false;
                  }
                }
                return false;
              }),
          pickedObjects.end());
    }

    void EditorScene::AddBillboardToEntity(Entity* entity)
    {
      auto addBillboardFn = [this, &entity](EditorBillboardBase* billboard)
      {
        billboard->m_entity          = entity;
        m_entityBillboardMap[entity] = billboard;
        m_billboards.push_back(billboard);
      };

      // Note: Only 1 billboard per entity is supported.

      // Check environment component
      EnvironmentComponentPtr envCom =
          entity->GetComponent<EnvironmentComponent>();
      if (envCom != nullptr ||
          entity->GetType() == EntityType::Entity_GradientSky)
      {
        SkyBillboard* billboard = new SkyBillboard();
        addBillboardFn(billboard);
        return;
      }

      // Check light
      if (entity->IsLightInstance())
      {
        LightBillboard* billboard = new LightBillboard();
        addBillboardFn(billboard);
        return;
      }
    }

    void EditorScene::RemoveBillboardFromEntity(Entity* entity)
    {
      if (m_entityBillboardMap.find(entity) != m_entityBillboardMap.end())
      {
        Entity* bb = m_entityBillboardMap[entity];
        m_entityBillboardMap.erase(entity);
        for (auto it = m_billboards.begin(); it != m_billboards.end(); ++it)
        {
          if ((*it)->GetIdVal() == bb->GetIdVal())
          {
            m_billboards.erase(it);
            break;
          }
        }
        SafeDel(bb);
      }
    }

    std::vector<EditorBillboardBase*> EditorScene::GetBillboards()
    {
      return m_billboards;
    }

    Entity* EditorScene::GetBillboardOfEntity(Entity* entity)
    {
      if (m_entityBillboardMap.find(entity) != m_entityBillboardMap.end())
      {
        return m_entityBillboardMap[entity];
      }
      else
      {
        return nullptr;
      }
    }

    void EditorScene::InitEntityBillboard(Entity* entity)
    {
      if (InitBillboard(entity, EditorBillboardBase::BillboardType::Sky))
      {
        return;
      }

      if (InitBillboard(entity, EditorBillboardBase::BillboardType::Light))
      {
        return;
      }
    }

    void EditorScene::CopyTo(Resource* other)
    {
      // Clear fixed layers.
      EditorScene* cpy = static_cast<EditorScene*>(other);
      Scene::CopyTo(cpy);
      cpy->m_newScene = true;
    }

    bool EditorScene::InitBillboard(Entity* entity,
                                    EditorBillboardBase::BillboardType type)
    {
      bool found    = false;
      Billboard* bb = nullptr;

      if (m_entityBillboardMap.find(entity) == m_entityBillboardMap.end())
      {
        found = false;
      }
      else
      {
        found = true;
      }

      bool check = false;
      if (type == EditorBillboardBase::BillboardType::Sky)
      {
        check = entity->GetComponent<EnvironmentComponent>() != nullptr ||
                entity->GetType() == EntityType::Entity_GradientSky;
      }
      else if (type == EditorBillboardBase::BillboardType::Light)
      {
        check = entity->IsLightInstance();
      }

      if (!check && found)
      {
        // Remove billboard if the entity is not the correct type
        RemoveBillboardFromEntity(entity);
        return false;
      }
      else if (found)
      {
        // Check if the billboard is the correct one and change it if it is not
        if (m_entityBillboardMap[entity]->GetBillboardType() == type)
        {
          return true;
        }
        else
        {
          RemoveBillboardFromEntity(entity);
          return false;
        }
      }
      else
      {
        // Add billboard
        AddBillboardToEntity(entity);
        return true;
      }
    }

    EditorSceneManager::EditorSceneManager() {}

    EditorSceneManager::~EditorSceneManager() {}

    ResourcePtr EditorSceneManager::CreateLocal(ResourceType type)
    {
      return ResourcePtr(new EditorScene());
    }

  } // namespace Editor
} // namespace ToolKit
