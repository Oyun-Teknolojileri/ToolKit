#pragma once

#include "Mod.h"
#include "Gizmo.h"

namespace ToolKit
{
	namespace Editor
	{
		// States.
		class StateTransformBase : public State
		{
		public:
			StateTransformBase();
			virtual void Update(float deltaTime) override;
			virtual void TransitionIn(State* prevState) override;
			virtual void TransitionOut(State* nextState) override;

		protected:
			void MakeSureGizmoIsValid();
			Vec3 GetGrabbedAxis(int n); // {0: grabbed 1: orthogonal axis}.
			bool IsPlaneMod();

		public:
			Quaternion m_axisOrientation;
			std::vector<Vec2> m_mouseData;
			std::shared_ptr<Gizmo> m_gizmo;
			PlaneEquation m_intersectionPlane;
		};

		class StateTransformBegin : public StateTransformBase
		{
		public:
			virtual void TransitionIn(State* prevState) override;
			virtual void TransitionOut(State* nextState) override;

			virtual void Update(float deltaTime) override;
			virtual String Signaled(SignalId signal) override;
			virtual String GetType() override { return StateType::StateTransformBegin; }

		private:
			void CalculateIntersectionPlane();
		};

		class StateTransformTo : public StateTransformBase
		{
		public:
			virtual void TransitionIn(State* prevState) override;
			virtual void TransitionOut(State* prevState) override;
			virtual void Update(float deltaTime) override;
			virtual String Signaled(SignalId signal) override;
			virtual String GetType() override { return StateType::StateTransformTo; }

		private:
			void CalculateDelta();

		public:
			Vec3 m_delta;

		private:
			std::shared_ptr<LineBatch> m_guideLine;
		};

		class StateTransformEnd : public StateTransformBase
		{
		public:
			virtual void TransitionOut(State* nextState) override;
			virtual void Update(float deltaTime) override;
			virtual String Signaled(SignalId signal) override;
			virtual String GetType() override { return StateType::StateTransformEnd; }
		};

		// Mod.
		class TransformMod : public BaseMod
		{
		public:
			TransformMod(ModId id);
			virtual ~TransformMod();

			virtual void Init() override;
			virtual void Update(float deltaTime) override;

		private:
			void Transform(const Vec3& delta) const;

			// Signals.
			static SignalId m_linkToTransformBeginSgnl;
		};
	}
}