#include "stdafx.h"
#include "EditorScene.h"
#include "Util.h"
#include "GlobalDef.h"
#include "EditorViewport.h"
#include "App.h"
#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    EditorScene::EditorScene()
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
      assert(!IsSelected(id));
      if (!additive)
      {
        m_selectedEntities.clear();
      }
      m_selectedEntities.push_back(id);
    }

    void EditorScene::AddToSelection(const EntityIdArray& entities, bool additive)
    {
      ULongID currentId = NULL_ENTITY;
      if (entities.size() > 1)
      {
        for (const ULongID& id : entities)
        {
          if (id != NULL_ENTITY)
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
        if (id == NULL_ENTITY)
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

    void EditorScene::AddToSelection(const EntityRawPtrArray& entities, bool additive)
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
      EntityIdArray::iterator itr = std::find(m_selectedEntities.begin(), m_selectedEntities.end(), id);
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

    bool EditorScene::IsMaterialInUse(const MaterialPtr& material) const
    {
      for (Entity* ntt : m_entities)
      {
        if (ntt->IsDrawable())
        {
          if (static_cast<Drawable*> (ntt)->IsMaterialInUse(material))
          {
            return true;
          }
        }
      }

      return false;
    }

    bool EditorScene::IsMeshInUse(const MeshPtr& mesh) const
    {
      for (Entity* ntt : m_entities)
      {
        if (ntt->IsDrawable())
        {
          if (static_cast<Drawable*> (ntt)->IsMeshInUse(mesh))
          {
            return true;
          }
        }
      }

      return false;
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
        RemoveFromSelection(removed->m_id);
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

  }
}
