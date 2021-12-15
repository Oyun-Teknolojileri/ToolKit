#pragma once

#include "ToolKit.h"
#include "Scene.h"

namespace ToolKit
{
  namespace Editor
  {

    typedef std::shared_ptr<class EditorScene> EditorScenePtr;

    const static EntityId FixedLayerIds[2] = { 1, 2 };
    const static String FixedLayerNames[2] = { "3d Layer", "2d Layer" };

    class EditorScene : public Scene
    {
    public:
      EditorScene();
      EditorScene(const String& file);
      virtual ~EditorScene();

      // Scene overrides.
      virtual void AddEntity(Entity* entity) override;

      // Selection operations.
      bool IsSelected(EntityId id) const;
      void RemoveFromSelection(EntityId id);
      void AddToSelection(const EntityIdArray& entities, bool additive);
      void AddToSelection(const EntityRawPtrArray& entities, bool additive);
      void AddToSelection(EntityId id, bool additive);
      void ClearSelection();
      bool IsCurrentSelection(EntityId id) const;
      void MakeCurrentSelection(EntityId id, bool ifExist); // Makes the entity current selection. ifExist true, only works if the entity exist in the selection. Otherwise adds entity to selection list and selects it.
      uint GetSelectedEntityCount() const;
      Entity* GetCurrentSelection() const;

      // Resource operations
      bool IsMaterialInUse(const MaterialPtr& material) const;
      bool IsMeshInUse(const MeshPtr& mesh) const;
      virtual void Save(bool onlyIfDirty) override;
      virtual void Load() override;

      // Entity operations.
      virtual Entity* RemoveEntity(EntityId id) override;
      virtual void Destroy(bool removeResources) override;
      void GetSelectedEntities(EntityRawPtrArray& entities) const;
      void GetSelectedEntities(EntityIdArray& entities) const;
      void SelectByTag(const String& tag);

    private:
      virtual void CopyTo(Resource* other) override;

    public:
      bool m_newScene; // Indicates if this is created via new scene. That is not saved on the disk.
      EntityRawPtrArray m_fixedLayerNodes; // Using entity nodes to group entities.

    private:
      EntityIdArray m_selectedEntities;
    };
  }
}