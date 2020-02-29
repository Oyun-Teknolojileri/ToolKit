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
			virtual void Update(float deltaTime_ms);

		public:
			StateMachine* m_stateMachine;
		};

		// Common signals and states.
		class LeftClickSgnl : public SignalId
		{
		public:
			LeftClickSgnl() : SignalId(101) {}
		};

		class LeftClickDragSgnl : public SignalId
		{
		public:
			LeftClickDragSgnl() : SignalId(102) {}
		};

		class StatePicking : public State
		{

		};

		class SelectMod : public BaseMod
		{
		public:
			virtual void Init() override;
			virtual void Update(float deltaTime_ms) override;
		};
	}
}
