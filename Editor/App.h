#pragma once

#include "ToolKit.h"

namespace ToolKit
{
	class Renderer;
	class Light;
	class Cube;
}

namespace Editor
{
	class Viewport;

	using namespace ToolKit;
	using namespace glm;
	using namespace std;
	
	class Scene
	{
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

	private:
		Renderer* m_renderer;
		Cube* m_dummy;
	};
}
