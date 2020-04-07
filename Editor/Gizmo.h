#pragma once

#include "Primative.h"
#include "MathUtil.h"

namespace ToolKit
{
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

		class MoveGizmo : public Billboard
		{
		public:
			MoveGizmo();
			virtual ~MoveGizmo();

			enum class Axis
			{
				None,
				X,
				Y,
				Z,
				XY,
				XZ,
				YZ
			};
			Axis HitTest(const Ray& ray);
			void Update(float deltaTime);

		private:
			void Generate();

		private:
			BoundingBox m_hitBox[3]; // X - Y - Z.
			std::shared_ptr<Mesh> m_solids[3];
		};
	}
}
