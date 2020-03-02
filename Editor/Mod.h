#pragma once

#include "ToolKit.h"
#include "StateMachine.h"

namespace ToolKit
{
	namespace Editor
	{
		class BaseMod
		{
		public:
			BaseMod();
			virtual ~BaseMod();

			virtual void Init();
			virtual void Update(float deltaTime);

		public:
			StateMachine* m_stateMachine;
		};

		// Common signals and states.
		class LeftClickDownSgnl : public SignalId
		{
		public:
			LeftClickDownSgnl() : SignalId(101) {}
		};

		class LeftClickUpSgnl : public SignalId
		{
		public:
			LeftClickUpSgnl() : SignalId(102) {}
		};

		class MouseMoveSgnl : public SignalId
		{
		public:
			MouseMoveSgnl() : SignalId(103) {}
		};

		class StateBeginPick : public State
		{
		public:
			virtual void TransitionIn(State* prevState) override {};
			virtual void TransitionOut(State* nextState) override {};
			virtual void Update(float deltaTime) override;
			virtual State* Signaled(SignalId signal) override;
		};

		class StateBeginBoxPick : public State
		{
		public:
			virtual void TransitionIn(State* prevState) override {};
			virtual void TransitionOut(State* nextState) override {};
			virtual void Update(float deltaTime) override;
			virtual State* Signaled(SignalId signal) override;
		};

		class StateEndPick : public State
		{
		public:
			virtual void TransitionIn(State* prevState) override {};
			virtual void TransitionOut(State* nextState) override {};
			virtual void Update(float deltaTime) override;
			virtual State* Signaled(SignalId signal) override;
		};

		class SelectMod : public BaseMod
		{
		public:
			virtual void Init() override;
			virtual void Update(float deltaTime) override;
		};
	}
}
