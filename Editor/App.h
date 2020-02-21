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
			
			// Selection operations
			bool IsSelected(EntityId id);
			void RemoveFromSelection(EntityId id);
			void AddToSelection(EntityId id);
			void ClearSelection();
			bool IsCurrentSelection(EntityId id);

		public:
			std::vector<Light*> m_lights;
			std::vector<Entity*> m_entitites;

		private:
			std::vector<EntityId> m_selectedEntities;
		};

		class App
		{
		public:
			App(int windowWidth, int windowHeight);
			~App();

			void Init();
			void Frame(int deltaTime);
			void OnResize(int width, int height);
			void OnQuit();

			// Quick selected render implementation.
			void RenderSelected(Drawable* e, Camera* c);

		public:
			Scene m_scene;
			std::vector<Viewport*> m_viewports;

			// Editor variables.
			float m_camSpeed = 4.0; // Meters per sec.
			float m_mouseSensitivity = 0.5f;
			std::shared_ptr<Material> m_highLightMaterial;
			std::shared_ptr<Material> m_highLightSecondaryMaterial;

			// Editor objects.
			Sphere* m_hitMarker;
			Grid* m_grid;
			Axis3d* m_origin;

		private:
			Renderer* m_renderer;
			Drawable* m_suzanne;
			Cube* m_q1;
			Cube* m_q2;
		};

	}
}
