#pragma once

#include "EditorLight.h"
#include "Scene.h"

namespace ToolKit
{
  namespace Editor
  {

    class EditorScene : public Scene
    {
     public:
      EditorScene();
      explicit EditorScene(const String& file);
      virtual ~EditorScene();

      void Load() override;
      void Update(float deltaTime) override;

      // Selection operations.
      bool IsSelected(ULongID id) const;
      void RemoveFromSelection(ULongID id);
      void AddToSelection(const EntityIdArray& entities, bool additive);
      void AddToSelection(const EntityRawPtrArray& entities, bool additive);
      void AddToSelection(ULongID id, bool additive);
      void ClearSelection();
      bool IsCurrentSelection(ULongID id) const;

      // Makes the entity current selection. When ifExist true, only works if
      // the entity exist in the selection.
      // Otherwise adds entity to selection list and selects it.
      void MakeCurrentSelection(ULongID id, bool ifExist);

      uint GetSelectedEntityCount() const;
      Entity* GetCurrentSelection() const;

      // Resource operations
      void Save(bool onlyIfDirty) override;

      // Entity operations.
      void AddEntity(Entity* entity) override;
      void RemoveEntity(const EntityRawPtrArray& entities) override;
      /**
       * remove entity from the scene
       * @param  the id of the entity you want to remove
       * @param  do you want to remove with childs ?
       *         be aware that removed childs transforms preserved
       * @returns the entity that you removed, nullptr if entity is not in scene
       */
      Entity* RemoveEntity(ULongID id, bool deep = true) override;
      void Destroy(bool removeResources) override;
      void GetSelectedEntities(EntityRawPtrArray& entities) const;
      void GetSelectedEntities(EntityIdArray& entities) const;
      void SelectByTag(const String& tag);

      // Pick operations
      PickData PickObject(
          Ray ray,
          const EntityIdArray& ignoreList    = EntityIdArray(),
          const EntityRawPtrArray& extraList = EntityRawPtrArray()) override;
      void PickObject(const Frustum& frustum,
                      std::vector<PickData>& pickedObjects,
                      const EntityIdArray& ignoreList    = EntityIdArray(),
                      const EntityRawPtrArray& extraList = EntityRawPtrArray(),
                      bool pickPartiallyInside           = true) override;

      // Gizmo operations
      void AddBillboard(Entity* entity);
      void RemoveBillboard(Entity* entity);
      EntityRawPtrArray GetBillboards();
      Entity* GetBillboard(Entity* entity);
      void ValidateBillboard(Entity* entity);
      void ValidateBillboard(EntityRawPtrArray& entities);

     private:
      void CopyTo(Resource* other) override;

      /**
       * Updates the billboards to align with current viewports camera for
       * proper picking.
       */
      void UpdateBillboardsForPicking();

     public:
      // Indicates if this is created via new scene.
      // That is not saved on the disk.
      bool m_newScene;

     private:
      EntityIdArray m_selectedEntities;
      // Billboard gizmos
      std::unordered_map<Entity*, EditorBillboardBase*> m_entityBillboardMap;
      EntityRawPtrArray m_billboards;
    };

    class EditorSceneManager : public SceneManager
    {
     public:
      EditorSceneManager();
      virtual ~EditorSceneManager();
      ResourcePtr CreateLocal(ResourceType type) override;
    };

    typedef std::shared_ptr<class EditorScene> EditorScenePtr;

  } // namespace Editor
} // namespace ToolKit
