#pragma once

#include "Drawable.h"

namespace ToolKit
{
	class Material;

	namespace Editor
	{
		class Grid : public Drawable
		{
		public:
			Grid(uint size);
			void Resize(uint size);
			bool HitTest(const Ray& ray, Vec3& pos);

		public:
			uint m_size; // m^2 size of the grid.
			MaterialPtr m_material;
		};
	}
}