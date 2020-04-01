#pragma once

#include "ToolKit.h"

namespace ToolKit
{
	class Renderer;
	class Light;
	class Cube;
	class Sphere;
	class Camera;

	namespace Editor
	{
		class Viewport;
		class Grid;
		class Axis3d;
		class Cursor;
		class ConsoleWindow;

		using namespace ToolKit;
		using namespace glm;
		using namespace std;

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
			void AddToSelection(EntityId id);
			void ClearSelection();
			bool IsCurrentSelection(EntityId id);
			void MakeCurrentSelection(EntityId id, bool ifExist);
			uint GetSelectedEntityCount();

			// Entity operations.
			Entity* GetEntity(EntityId id);
			void AddEntity(Entity* entity);
			Entity* RemoveEntity(EntityId id);
			const std::vector<Entity*>& GetEntities();

		private:
			std::vector<Entity*> m_entitites;
			std::vector<EntityId> m_selectedEntities;
		};

		class App
		{
		public:
			App(int windowWidth, int windowHeight);
			~App();

			void Init();
			void Frame(float deltaTime);
			void OnResize(int width, int height);
			void OnQuit();
			
			Viewport* GetActiveViewport(); // Returns open and active viewport or nullptr.

			// Quick selected render implementation.
			void RenderSelected(Drawable* e, Camera* c);

		public:
			Scene m_scene;
			
			// UI elements.
			std::vector<Viewport*> m_viewports;
			ConsoleWindow* m_console;

			// Editor variables.
			float m_camSpeed = 4.0; // Meters per sec.
			float m_mouseSensitivity = 0.5f;
			std::shared_ptr<Material> m_highLightMaterial;
			std::shared_ptr<Material> m_highLightSecondaryMaterial;

			// Editor objects.
			Grid* m_grid;
			Axis3d* m_origin;
			Cursor* m_cursor;

			// Editor debug states.
			bool m_pickingDebug = false;

		private:
			Renderer* m_renderer;
			Drawable* m_suzanne;
			Cube* m_q1;
			Cube* m_q2;
		};

	}
}
