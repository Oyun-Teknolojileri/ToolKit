#pragma once

#include "ToolKit.h"
#include "Scene.h"
#include "EditorLight.h"
#include "Types.h"

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

      // Selection operations.
      bool IsSelected(ULongID id) const;
      void RemoveFromSelection(ULongID id);
      void AddToSelection(const EntityIdArray& entities, bool additive);
      void AddToSelection(const EntityRawPtrArray& entities, bool additive);
      void AddToSelection(ULongID id, bool additive);
      void ClearSelection();
      bool IsCurrentSelection(ULongID id) const;
      // Makes the entity current selection. ifExist true,
      void MakeCurrentSelection(ULongID id, bool ifExist);

      // only works if the entity exist in the selection.
      // Otherwise adds entity to selection list and selects it.
      uint GetSelectedEntityCount() const;
      Entity* GetCurrentSelection() const;

      // Resource operations
      void Save(bool onlyIfDirty) override;

      // Entity operations.
      Entity* RemoveEntity(ULongID id) override;
      void Destroy(bool removeResources) override;
      void GetSelectedEntities(EntityRawPtrArray& entities) const;
      void GetSelectedEntities(EntityIdArray& entities) const;
      void SelectByTag(const String& tag);

     private:
      void CopyTo(Resource* other) override;

     public:
       // Indicates if this is created via new scene.
       // That is not saved on the disk.
      bool m_newScene;

     private:
      EntityIdArray m_selectedEntities;
    };

    class EditorSceneManager : public SceneManager
    {
     public:
      EditorSceneManager();
      virtual ~EditorSceneManager();
      ResourcePtr CreateLocal(ResourceType type) override;
    };

  }  // namespace Editor
}  // namespace ToolKit
