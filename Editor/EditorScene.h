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
