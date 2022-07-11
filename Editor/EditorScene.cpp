#include "EditorScene.h"
#include "Util.h"
#include "GlobalDef.h"
#include "EditorViewport.h"
#include "App.h"
#include "EditorCamera.h"
#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    EditorScene::EditorScene()
      : Scene()
    {
      m_newScene = true;
    }

    EditorScene::EditorScene(const String& file)
      : EditorScene()
    {
      SetFile(file);
      m_newScene = false;
    }

    EditorScene::~EditorScene()
    {
    }

    void EditorScene::Load()
    {
      Scene::Load();

      for (int i = static_cast<int>(m_entities.size()) - 1; i > -1; i--)
      {
        Entity* ntt = m_entities[i];

        // Replace cameras with EditorCamera.
        if (ntt->GetType() == EntityType::Entity_Camera)
        {
          // Cheating here just to copy the Camera entity into EditorCamera.
          EditorCamera* upCasted = reinterpret_cast<EditorCamera*> (ntt);
          EditorCamera* cam = new EditorCamera(upCasted);
          m_entities[i] = cam;
          SafeDel(ntt);
        }
        else if (ntt->GetType() == EntityType::Entity_Light)
        {
          // Replace lights with EditorLights
          Light* el = reinterpret_cast<Light*>(ntt);
          LightTypeEnum type = static_cast<LightTypeEnum>
            (el->GetLightTypeVal());

          if (type == LightTypeEnum::LightDirectional)
          {
            EditorDirectionalLight* upCasted =
            reinterpret_cast<EditorDirectionalLight*> (ntt);
            EditorDirectionalLight* light =
            new EditorDirectionalLight(upCasted);
            light->Init();
            m_entities[i] = light;
            SafeDel(ntt);
          }
          else if (type == LightTypeEnum::LightSpot)
          {
            EditorPointLight* upCasted =
            reinterpret_cast<EditorPointLight*> (ntt);
            EditorPointLight* light = new EditorPointLight(upCasted);
            light->Init();
            m_entities[i] = light;
            SafeDel(ntt);
          }
          else if (type == LightTypeEnum::LightSpot)
          {
            EditorSpotLight* upCasted =
            reinterpret_cast<EditorSpotLight*> (ntt);
            EditorSpotLight* light = new EditorSpotLight(upCasted);
            light->Init();
            m_entities[i] = light;
            SafeDel(ntt);
          }
        }
      }
    }

    bool EditorScene::IsSelected(ULongID id) const
    {
      return std::find
      (
        m_selectedEntities.begin(),
        m_selectedEntities.end(),
        id
      ) != m_selectedEntities.end();
    }

    void EditorScene::RemoveFromSelection(ULongID id)
    {
      auto nttIt = std::find
      (
        m_selectedEntities.begin(),
        m_selectedEntities.end(),
        id
      );
      if (nttIt != m_selectedEntities.end())
      {
        m_selectedEntities.erase(nttIt);
      }
    }

    void EditorScene::AddToSelection(ULongID id, bool additive)
    {
      assert(!IsSelected(id));
      if (!additive)
      {
        m_selectedEntities.clear();
      }
      m_selectedEntities.push_back(id);
    }

    void EditorScene::AddToSelection
    (
      const EntityIdArray& entities,
      bool additive
    )
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
        else  // Add, make current or toggle selection.
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

    void EditorScene::AddToSelection
    (
      const EntityRawPtrArray& entities,
      bool additive
    )
    {
      EntityIdArray ids;
      ToEntityIdArray(ids, entities);
      AddToSelection(ids, additive);
    }

    void EditorScene::ClearSelection()
    {
      m_selectedEntities.clear();
    }

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
      EntityIdArray::iterator itr = std::find
      (
        m_selectedEntities.begin(),
        m_selectedEntities.end(), id
      );
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
      return (uint)m_selectedEntities.size();
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

    Entity* EditorScene::RemoveEntity(ULongID id)
    {
      Entity* removed = nullptr;
      if ((removed = Scene::RemoveEntity(id)))
      {
        RemoveFromSelection(removed->GetIdVal());
      }

      return removed;
    }

    void EditorScene::Destroy(bool removeResources)
    {
      Scene::Destroy(removeResources);
      m_selectedEntities.clear();
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

    void EditorScene::CopyTo(Resource* other)
    {
      // Clear fixed layers.
      EditorScene* cpy = static_cast<EditorScene*> (other);
      Scene::CopyTo(cpy);
      cpy->m_newScene = true;
    }

    EditorSceneManager::EditorSceneManager()
    {
    }

    EditorSceneManager::~EditorSceneManager()
    {
    }

    ResourcePtr EditorSceneManager::CreateLocal(ResourceType type)
    {
      return ResourcePtr(new EditorScene());
    }

  }  // namespace Editor
}  // namespace ToolKit
