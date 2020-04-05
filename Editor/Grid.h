#pragma once

#include "Primative.h"
#include "Types.h"

namespace ToolKit
{
	class Material;
	class Camera;
	class Surface;
	class Renderer;

	namespace Editor
	{
		class Billboard : public Drawable
		{
		public:
			struct Settings
			{
				bool lookAtCamera;
				bool keepDistanceToCamera;
				float distanceToCamera;
				bool keepScreenSpaceSize;
				float heightScreenSpace;
			};

		public:
			Billboard(const Settings& settings);
			virtual ~Billboard();
			virtual void LookAt(Camera* cam);

		public:
			Settings m_settings;
			glm::vec3 m_worldLocation;
		};

		class Cursor : public Billboard
		{
		public:
			Cursor();
			virtual ~Cursor();

		private:
			void Generate();
		};

		class Axis3d : public Billboard
		{
		public:
			Axis3d();

		private:
			void Generate();
		};

		class Grid : public Drawable
		{
		public:
			Grid(uint size);
			void Resize(uint size);

		public:
			uint m_size; // m^2 size of the grid.
			std::shared_ptr<Material> m_material;
		};
	}
}