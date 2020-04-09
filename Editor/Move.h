#pragma once

#include "Mod.h"

namespace ToolKit
{
	namespace Editor
	{
		class MoveGizmo;

		// States.
		class StateMoveBase : public State
		{
		public:
			StateMoveBase(std::string name);
			virtual void TransitionIn(State* prevState) override;
			virtual void TransitionOut(State* nextState) override;

		public:
			std::shared_ptr<MoveGizmo> m_gizmo = nullptr;
			std::vector<glm::vec2> m_mouseData;
		};

		class StateBeginMove : public StateMoveBase
		{
		public:
			StateBeginMove() : StateMoveBase("BeginMove") {}

			virtual void TransitionIn(State* prevState) override;
			virtual void TransitionOut(State* nextState) override;

			virtual void Update(float deltaTime) override;
			virtual std::string Signaled(SignalId signal) override;
		};

		class StateMoveTo : public StateMoveBase
		{
		public:
			StateMoveTo() : StateMoveBase("MoveTo") {}

		};

		class StateEndMove : public StateMoveBase
		{
		public:
			StateEndMove() : StateMoveBase("EndMove") {}

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