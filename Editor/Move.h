#pragma once

#include "Mod.h"
#include "Gizmo.h"

namespace ToolKit
{
	namespace Editor
	{
		// States.
		class StateGizmoBase : public State
		{
		public:
			StateGizmoBase();
			virtual void Update(float deltaTime) override;
			virtual void TransitionIn(State* prevState) override;
			virtual void TransitionOut(State* nextState) override;

		protected:
			void MakeSureGizmoIsValid();
			Vec3 GetGrabbedAxis(int n); // {0: grabbed 1: orthogonal axis}.
			bool IsPlaneMod();

		public:
			std::vector<Vec2> m_mouseData;
			std::shared_ptr<Gizmo> m_gizmo;
			AxisLabel m_grabbedAxis;
			PlaneEquation m_intersectionPlane;
		};

		class StateGizmoBegin : public StateGizmoBase
		{
		public:
			virtual void TransitionOut(State* nextState) override;

			virtual void Update(float deltaTime) override;
			virtual String Signaled(SignalId signal) override;
			virtual String GetType() override { return StateType::StateBeginMove; }

		private:
			void CalculateIntersectionPlane();
		};

		class StateMoveTo : public StateGizmoBase
		{
		public:
			virtual void TransitionIn(State* prevState) override;
			virtual void TransitionOut(State* prevState) override;
			virtual void Update(float deltaTime) override;
			virtual String Signaled(SignalId signal) override;
			virtual String GetType() override { return StateType::StateMoveTo; }

		private:
			void Move();

		private:
			std::shared_ptr<LineBatch> m_guideLine;
		};

		class StateGizmoEnd : public StateGizmoBase
		{
		public:
			virtual void TransitionOut(State* nextState) override;
			virtual String Signaled(SignalId signal) override;
			virtual String GetType() override { return StateType::StateEndMove; }
		};

		// Mod.
		class MoveMod : public BaseMod
		{
		public:
			MoveMod() : BaseMod(ModId::Move) { Init(); }
			virtual ~MoveMod();

			virtual void Init() override;
			virtual void Update(float deltaTime) override;

		public:
			// Signals.
			static SignalId m_linkToMoveBeginSgnl;
		};
	}
}