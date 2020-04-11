#pragma once

#include "ToolKit.h"

namespace ToolKit
{
	namespace Editor
	{
		class Scene
		{
		public:
			// Scene queries.
			struct PickData
			{
				glm::vec3 pickPos;
				Entity* entity = nullptr;
			};

			PickData PickObject(Ray ray, const std::vector<EntityId>& ignoreList = std::vector<EntityId>());
			void PickObject(const Frustum& frustum, std::vector<PickData>& pickedObjects, const std::vector<EntityId>& ignoreList = std::vector<EntityId>(), bool pickPartiallyInside = true);

			// Selection operations.
			bool IsSelected(EntityId id);
			void RemoveFromSelection(EntityId id);
			void AddToSelection(const std::vector<EntityId>& entities, bool additive);
			void AddToSelection(EntityId id);
			void ClearSelection();
			bool IsCurrentSelection(EntityId id);
			void MakeCurrentSelection(EntityId id, bool ifExist);
			uint GetSelectedEntityCount();
			Entity* GetCurrentSelection();

			// Entity operations.
			Entity* GetEntity(EntityId id);
			void AddEntity(Entity* entity);
			Entity* RemoveEntity(EntityId id);
			const std::vector<Entity*>& GetEntities();
			void GetSelectedEntities(std::vector<Entity*>& entities);

		private:
			std::vector<Entity*> m_entitites;
			std::vector<EntityId> m_selectedEntities;
		};
	}
}