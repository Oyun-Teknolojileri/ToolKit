#pragma once

#include "ToolKit.h"

namespace ToolKit
{
	namespace Editor
	{
		class Scene
		{
		public:
			Scene();
			~Scene();

			// Scene queries.
			struct PickData
			{
				Vec3 pickPos;
				Entity* entity = nullptr;
			};

			PickData PickObject(Ray ray, const EntityIdArray& ignoreList = EntityIdArray()) const;
			void PickObject(const Frustum& frustum, std::vector<PickData>& pickedObjects, const EntityIdArray& ignoreList = EntityIdArray(), bool pickPartiallyInside = true) const;

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

			// Entity operations.
			Entity* GetEntity(EntityId id) const;
			void AddEntity(Entity* entity);
			Entity* RemoveEntity(EntityId id);
			const EntityRawPtrArray& GetEntities() const;
			void GetSelectedEntities(EntityRawPtrArray& entities) const;
			void GetSelectedEntities(EntityIdArray& entities) const;
			void Destroy();

			void Serialize(XmlDocument* doc, XmlNode* parent);

		public:
			String m_name;

		private:
			EntityRawPtrArray m_entitites;
			EntityIdArray m_selectedEntities;
		};
	}
}