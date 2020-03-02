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

		class StatePickingBase : public State
		{
		public:
			std::vector<EntityId> m_pickedNtties;
			glm::ivec2 m_screenSpacePickingCoordinates[2];
		};

		class StateBeginPick : public StatePickingBase
		{
		public:
			virtual void TransitionIn(State* prevState) override {};
			virtual void TransitionOut(State* nextState) override {};
			virtual void Update(float deltaTime) override;
			virtual State* Signaled(SignalId signal) override;
		};

		class StateBeginBoxPick : public StatePickingBase
		{
		public:
			virtual void TransitionIn(State* prevState) override {};
			virtual void TransitionOut(State* nextState) override {};
			virtual void Update(float deltaTime) override;
			virtual State* Signaled(SignalId signal) override;
		};

		class StateEndPick : public StatePickingBase
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
