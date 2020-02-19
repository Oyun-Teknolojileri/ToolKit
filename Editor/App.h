#pragma once

#include "ToolKit.h"

namespace ToolKit
{
	class Renderer;
	class Light;
	class Cube;
	class Sphere;

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
			// Viewport Picking Utilities
			struct PickData
			{
				glm::vec3 pickPos;
				Entity* entity = nullptr;
			};

			PickData PickObject(Ray ray);

		public:
			std::vector<Light*> m_lights;
			std::vector<Entity*> m_entitites;
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

		public:
			std::vector<Viewport*> m_viewports;
			Scene m_scene;

			// Editor variables
			float m_camSpeed = 4.0; // Meters per sec.
			float m_mouseSensitivity = 0.5f;

		private:
			Renderer* m_renderer;
			Drawable* m_dummy;
			Cube* m_q1;
			Cube* m_q2;
			Grid* m_grid;
			Axis3d* m_origin;
			Sphere* m_hitMarker;
		};

	}
}
