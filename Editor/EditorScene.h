/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

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
      TKDeclareClass(EditorScene, Scene);

      EditorScene();
      explicit EditorScene(const String& file);
      virtual ~EditorScene();

      void Load() override;
      void Update(float deltaTime) override;

      // Selection operations.
      bool IsSelected(ULongID id) const;
      void RemoveFromSelection(ULongID id);
      void AddToSelection(const EntityIdArray& entities, bool additive);
      void AddToSelection(const EntityPtrArray& entities, bool additive);
      void AddToSelection(ULongID id, bool additive);
      void ClearSelection();
      bool IsCurrentSelection(ULongID id) const;

      // Makes the entity current selection. When ifExist true, only works if
      // the entity exist in the selection.
      // Otherwise adds entity to selection list and selects it.
      void MakeCurrentSelection(ULongID id, bool ifExist);

      uint GetSelectedEntityCount() const;
      EntityPtr GetCurrentSelection() const;

      // Resource operations
      void Save(bool onlyIfDirty) override;

      // Entity operations.
      void AddEntity(EntityPtr entity) override;
      void RemoveEntity(const EntityPtrArray& entities) override;

      /**
       * remove entity from the scene
       * @param  the id of the entity you want to remove
       * @param  do you want to remove with childs ?
       *         be aware that removed childs transforms preserved
       * @returns the entity that you removed, nullptr if entity is not in scene
       */
      EntityPtr RemoveEntity(ULongID id, bool deep = true) override;
      void Destroy(bool removeResources) override;
      void GetSelectedEntities(EntityPtrArray& entities) const;
      void GetSelectedEntities(EntityIdArray& entities) const;
      void SelectByTag(const String& tag);

      // Pick operations
      PickData PickObject(Ray ray, const EntityIdArray& ignoreList = {}, const EntityPtrArray& extraList = {}) override;

      void PickObject(const Frustum& frustum,
                      PickDataArray& pickedObjects,
                      const EntityIdArray& ignoreList = {},
                      const EntityPtrArray& extraList = {},
                      bool pickPartiallyInside        = true) override;

      // Gizmo operations
      void AddBillboard(EntityPtr entity);
      void RemoveBillboard(EntityPtr entity);
      EntityPtrArray GetBillboards();
      EntityPtr GetBillboard(EntityPtr entity);
      void ValidateBillboard(EntityPtr entity);
      void ValidateBillboard(EntityPtrArray& entities);

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
      std::unordered_map<EntityPtr, EditorBillboardPtr> m_entityBillboardMap;
      EntityPtrArray m_billboards;
    };

    typedef std::shared_ptr<EditorScene> EditorScenePtr;

    class EditorSceneManager : public SceneManager
    {
     public:
      EditorSceneManager();
      virtual ~EditorSceneManager();
      ResourcePtr CreateLocal(ClassMeta* Class) override;
    };

  } // namespace Editor
} // namespace ToolKit
