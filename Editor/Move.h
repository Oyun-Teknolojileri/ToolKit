#pragma once

#include "Mod.h"
#include "Gizmo.h"

namespace ToolKit
{
	namespace Editor
	{
		// States.
		class StateMoveBase : public State
		{
		public:
			StateMoveBase();
			virtual void Update(float deltaTime) override;
			virtual void TransitionIn(State* prevState) override;
			virtual void TransitionOut(State* nextState) override;

		protected:
			void MakeSureGizmoIsValid();

		public:
			std::shared_ptr<MoveGizmo> m_gizmo;
			MoveGizmo::Axis m_grabbedAxis;
			std::vector<glm::vec2> m_mouseData;
		};

		class StateBeginMove : public StateMoveBase
		{
		public:
			virtual void TransitionIn(State* prevState) override;
			virtual void TransitionOut(State* nextState) override;

			virtual void Update(float deltaTime) override;
			virtual std::string Signaled(SignalId signal) override;
			virtual std::string GetType() override { return StateType::StateBeginMove; }
		};

		class StateMoveTo : public StateMoveBase
		{
		public:
			virtual void TransitionIn(State* prevState) override;
			virtual void TransitionOut(State* nextState) override;

			virtual void Update(float deltaTime) override;
			virtual std::string Signaled(SignalId signal) override;
			virtual std::string GetType() override { return StateType::StateMoveTo; }

		private:
			void Move();
		};

		class StateEndMove : public StateMoveBase
		{
		public:
			virtual std::string Signaled(SignalId signal) override;
			virtual std::string GetType() override { return StateType::StateEndMove; }
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