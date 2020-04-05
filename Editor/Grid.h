#pragma once

#include "Primative.h"

namespace ToolKit
{
	class Material;

	namespace Editor
	{
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