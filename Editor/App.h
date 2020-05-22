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
	class Animation;

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

			// Import facilities.
			int Import(const String& fullPath, const String& subDir, bool overwrite);
			bool CanImport(const String& fullPath);
			
			Viewport* GetActiveViewport(); // Returns open and active viewport or nullptr.
			Viewport* GetViewport(const String& name);
			ConsoleWindow* GetConsole();

			// Quick selected render implementation.
			void RenderSelected(Viewport *vp);

		public:
			Scene m_scene;

			// UI elements.
			std::vector<Window*> m_windows;

			// Editor variables.
			float m_camSpeed = 8.0; // Meters per sec.
			float m_mouseSensitivity = 0.5f;
			MaterialPtr m_highLightMaterial;
			MaterialPtr m_highLightSecondaryMaterial;

			// Editor objects.
			Grid* m_grid;
			Axis3d* m_origin;
			Cursor* m_cursor;
			std::vector<Drawable*> m_perFrameDebugObjects;

			// 3 point lighting system.
			Node* m_lightMaster;
			LightRawPtrArray m_sceneLights; // { 0:key 1:fill, 2:back }

			// Editor states.
			int m_fps = 0;
			bool m_showPickingDebug = false;
			bool m_showStateTransitionsDebug = false;
			bool m_showOverlayUI = true;
			bool m_showOverlayUIAlways = true;
			bool m_snapsEnabled = false;
			bool m_importSlient = false;
			float m_moveDelta = 0.25f;
			float m_rotateDelta = 5.0f;
			float m_scaleDelta = 0.25f;
			TransformationSpace m_transformOrientation = TransformationSpace::TS_WORLD;

		private:
			Renderer* m_renderer;
			Drawable* m_suzanne;
			Drawable* m_knight;
			std::shared_ptr<Animation> m_knightRunAnim;
			Cube* m_q1;
			Cube* m_q2;
			Cone* m_q3;
			Cube* m_q4;
		};

	}
}
