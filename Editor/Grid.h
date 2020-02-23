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
		class Cursor : public Drawable
		{
		public:
			Cursor();
			~Cursor();
			void LookAt(Camera* cam);

		public:
			glm::vec3 m_pickPosition;

		private:
			void Generate();
		};

		class Axis3d : public Drawable
		{
		public:
			Axis3d();
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