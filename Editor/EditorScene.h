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
      virtual ~EditorScene();

      void Load() override;
      void Update(float deltaTime) override;

      // Selection operations.
      bool IsSelected(ULongID id) const;
      void RemoveFromSelection(ULongID id);
      void AddToSelection(const IDArray& entities, bool additive);
      void AddToSelection(const EntityPtrArray& entities, bool additive);
      void AddToSelection(ULongID id, bool additive);
      void ClearSelection();
      bool IsCurrentSelection(ULongID id) const;

      // Makes the entity current selection.
      void MakeCurrentSelection(ULongID id);

      uint GetSelectedEntityCount() const;
      EntityPtr GetCurrentSelection() const;

      // Resource operations
      void Save(bool onlyIfDirty) override;

      // Entity operations.
      void AddEntity(EntityPtr entity, int index = -1) override;
      void RemoveEntity(const EntityPtrArray& entities, bool deep = true) override;

      /**
       * remove entity from the scene
       * @param  the id of the entity you want to remove
       * @param  do you want to remove with children ?
       *         be aware that removed children transforms preserved
       * @returns the entity that you removed, nullptr if entity is not in scene
       */
      EntityPtr RemoveEntity(ULongID id, bool deep = true) override;

      void Destroy(bool removeResources) override;
      void GetSelectedEntities(EntityPtrArray& entities) const;
      void GetSelectedEntities(IDArray& entities) const;
      void SelectByTag(const String& tag);

      // Pick operations
      PickData PickObject(const Ray& ray, const IDArray& ignoreList = {}, const EntityPtrArray& extraList = {}) override;

      void PickObject(const Frustum& frustum,
                      PickDataArray& pickedObjects,
                      const IDArray& ignoreList       = {},
                      const EntityPtrArray& extraList = {}) override;

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

      /** Internally used to sanitize selection before adding it. */
      void AddToSelectionSane(ULongID id);

     public:
      // Indicates if this is created via new scene.
      // That is not saved on the disk.
      bool m_newScene;

     private:
      IDArray m_selectedEntities;

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
    };

  } // namespace Editor
} // namespace ToolKit
