#pragma once

#include "ToolKit.h"
#include "Scene.h"

namespace ToolKit
{
  namespace Editor
  {

    class EditorScene : public Scene
    {
    public:
      EditorScene();
      EditorScene(const String& file);
      virtual ~EditorScene();

      // Selection operations.
      bool IsSelected(ULongID id) const;
      void RemoveFromSelection(ULongID id);
      void AddToSelection(const EntityIdArray& entities, bool additive);
      void AddToSelection(const EntityRawPtrArray& entities, bool additive);
      void AddToSelection(ULongID id, bool additive);
      void ClearSelection();
      bool IsCurrentSelection(ULongID id) const;
      void MakeCurrentSelection(ULongID id, bool ifExist); // Makes the entity current selection. ifExist true, only works if the entity exist in the selection. Otherwise adds entity to selection list and selects it.
      uint GetSelectedEntityCount() const;
      Entity* GetCurrentSelection() const;

      // Resource operations
      bool IsMaterialInUse(const MaterialPtr& material) const;
      bool IsMeshInUse(const MeshPtr& mesh) const;
      virtual void Save(bool onlyIfDirty) override;

      // Entity operations.
      virtual Entity* RemoveEntity(ULongID id) override;
      virtual void Destroy(bool removeResources) override;
      void GetSelectedEntities(EntityRawPtrArray& entities) const;
      void GetSelectedEntities(EntityIdArray& entities) const;
      void SelectByTag(const String& tag);

    private:
      virtual void CopyTo(Resource* other) override;

    public:
      bool m_newScene; // Indicates if this is created via new scene. That is not saved on the disk.

    private:
      EntityIdArray m_selectedEntities;
    };

    class EditorSceneManager : public SceneManager
    {
    public:
      EditorSceneManager();
      virtual ~EditorSceneManager();
      virtual ResourcePtr CreateLocal(ResourceType type) override;
    };

  }
}