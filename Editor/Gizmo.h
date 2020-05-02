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
			virtual ~Axis3d();

		private:
			void Generate();
		};

		class GizmoHandle
		{
		public:
			enum class SolidType
			{
				Cube,
				Cone
			};

			struct HandleParams
			{
				AxisLabel axis;
				Mat3 normalVectors;
				Ray dir;
				Vec2 toeTip;
				Vec3 solidDim;
				Vec3 color;
				SolidType type;
			};

		public:
			GizmoHandle();
			~GizmoHandle();

			void Generate(const HandleParams& params);
			bool HitTest(const Ray& ray) const;

		public:
			MeshPtr m_mesh;

		private:
			HandleParams m_params;
		};

		class Gizmo : public Billboard
		{
		public:
			Gizmo(const Billboard::Settings& set);
			virtual ~Gizmo();

			virtual AxisLabel HitTest(const Ray& ray) const;
			virtual void Update(float deltaTime) = 0;
			bool IsLocked(AxisLabel axis) const;
			void Lock(AxisLabel axis);
			void UnLock(AxisLabel axis);
			bool IsGrabbed(AxisLabel axis) const;
			void Grab(AxisLabel axis);
			AxisLabel GetGrabbedAxis() const;

		public:
			Mat3 m_normalVectors;

		protected:
			std::vector<GizmoHandle> m_handles;
			std::vector<AxisLabel> m_lockedAxis;
			AxisLabel m_grabbedAxis;
		};

		class LinearGizmo : public Gizmo
		{
		public:
			LinearGizmo();
			virtual ~LinearGizmo();

			virtual AxisLabel HitTest(const Ray& ray) const;
			virtual void Update(float deltaTime) override;

		protected:
			virtual GizmoHandle::HandleParams GetParam() const;
		};

		class MoveGizmo : public LinearGizmo
		{
		public:
			MoveGizmo();
			virtual ~MoveGizmo();
		};

		class ScaleGizmo : public LinearGizmo
		{
		public:
			ScaleGizmo();
			virtual ~ScaleGizmo();

		protected:
			virtual GizmoHandle::HandleParams GetParam() const override;
		};
	}
}
