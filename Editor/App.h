#pragma once

#include "ToolKit.h"
#include "Scene.h"

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
		class Window;

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
			ConsoleWindow* GetConsole();

			// Quick selected render implementation.
			void RenderSelected(Drawable* e, Camera* c);

		public:
			Scene m_scene;
			
			// UI elements.
			std::vector<Window*> m_windows;

			// Editor variables.
			float m_camSpeed = 4.0; // Meters per sec.
			float m_mouseSensitivity = 0.5f;
			std::shared_ptr<Material> m_highLightMaterial;
			std::shared_ptr<Material> m_highLightSecondaryMaterial;

			// Editor objects.
			Grid* m_grid;
			Axis3d* m_origin;
			Cursor* m_cursor;

			// Editor states.
			bool m_showPickingDebug = false;
			bool m_showOverlayUI = true;

		private:
			Renderer* m_renderer;
			Drawable* m_suzanne;
			Cube* m_q1;
			Cube* m_q2;
		};

	}
}
