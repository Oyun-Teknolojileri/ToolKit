#pragma once

#include "ToolKit.h"
#include "StateMachine.h"

namespace ToolKit
{
	namespace Editor
	{
		enum class ModId
		{
			Base,
			Select,
			Move,
			Rotate,
			Scale
		};

		class BaseMod
		{
		public:
			BaseMod(ModId id);
			virtual ~BaseMod();

			virtual void Init();
			virtual void UnInit();
			virtual void Update(float deltaTime);
			virtual void Signal(SignalId signal);

		public:
			ModId m_id;
			StateMachine* m_stateMachine;
			bool m_terminate; // When set to true, ModManager will terminate the mod.
		};

		class ModManager
		{
		public:
			~ModManager();

			ModManager(ModManager const&) = delete;
			void operator=(ModManager const&) = delete;

			static ModManager* GetInstance();
			void Update(float deltaTime);
			void DispatchSignal(SignalId signal);
			void SetMod(bool set, ModId mod); // If set true, sets the given mod. Else does nothing.

		private:
			ModManager();
			
		private:
			static ModManager m_instance;

		public:
			std::vector<BaseMod*> m_modStack;
		};

		// Common signals and states.
		class LeftMouseBtnDownSgnl : public SignalId
		{
		public:
			LeftMouseBtnDownSgnl() : SignalId(101) {}
		};

		class LeftMouseBtnUpSgnl : public SignalId
		{
		public:
			LeftMouseBtnUpSgnl() : SignalId(102) {}
		};

		class MouseDragSgnl : public SignalId
		{
		public:
			MouseDragSgnl() : SignalId(103) {} // Left click is pressed and mouse is moving.
		};

		class MouseMoveSgnl : public SignalId
		{
		public:
			MouseMoveSgnl() : SignalId(104) {}
		};

		class StatePickingBase : public State
		{
		public:
			StatePickingBase(std::string name) : State(name) {}

		public:
			std::vector<EntityId> m_pickedNtties;
			std::vector<Ray> m_pickingRays;
		};

		class StateBeginPick : public StatePickingBase
		{
		public:
			StateBeginPick() : StatePickingBase("BeginPick") {}

		public:
			virtual void TransitionIn(State* prevState) override {};
			virtual void TransitionOut(State* nextState) override;
			virtual void Update(float deltaTime) override;
			virtual State* Signaled(SignalId signal) override;
		};

		class StateBeginBoxPick : public StatePickingBase
		{
		public:
			StateBeginBoxPick() : StatePickingBase("BeginBoxPick") {}

		public:
			virtual void TransitionIn(State* prevState) override {};
			virtual void TransitionOut(State* nextState) override {};
			virtual void Update(float deltaTime) override;
			virtual State* Signaled(SignalId signal) override;
		};

		class StateEndPick : public StatePickingBase
		{
		public:
			StateEndPick() : StatePickingBase("EndPick") {}

		public:
			virtual void TransitionIn(State* prevState) override {};
			virtual void TransitionOut(State* nextState) override {};
			virtual void Update(float deltaTime) override;
			virtual State* Signaled(SignalId signal) override;
		};

		class SelectMod : public BaseMod
		{
		public:
			SelectMod() : BaseMod(ModId::Select) {}

			virtual void Init() override;
			virtual void Update(float deltaTime) override;
			void ApplySelection(std::vector<EntityId>& selectedNtties);
		};
	}
}
