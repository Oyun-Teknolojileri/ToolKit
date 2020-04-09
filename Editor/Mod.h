#pragma once

#include "ToolKit.h"
#include "StateMachine.h"
#include "App.h"

namespace ToolKit
{
	class Arrow2d;
	class LineBatch;

	namespace Editor
	{
		enum class ModId
		{
			Base,
			Select,
			Cursor,
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

			void Init();
			void UnInit();
			static ModManager* GetInstance();
			void Update(float deltaTime);
			void DispatchSignal(SignalId signal);
			void SetMod(bool set, ModId mod); // If set true, sets the given mod. Else does nothing.

		private:
			ModManager();
			
		private:
			static ModManager m_instance;
			bool m_initiated;

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

		class LeftMouseBtnDragSgnl : public SignalId
		{
		public:
			LeftMouseBtnDragSgnl() : SignalId(103) {}
		};

		class MouseMoveSgnl : public SignalId
		{
		public:
			MouseMoveSgnl() : SignalId(104) {}
		};

		class StatePickingBase : public State
		{
		public:
			StatePickingBase(std::string name);
			virtual void TransitionIn(State* prevState) override;
			virtual void TransitionOut(State* nextState) override;
			bool IsIgnored(Entity* ntt);
			void PickDataToEntityId(std::vector<EntityId>& ids);

		public:
			// Picking data.
			std::vector<glm::vec2> m_mouseData;
			std::vector<Scene::PickData> m_pickData;
			std::vector<EntityId> m_ignoreList;

			// Debug models.
			static std::shared_ptr<Arrow2d> m_dbgArrow;
			static std::shared_ptr<LineBatch> m_dbgFrustum;
		};

		class StateBeginPick : public StatePickingBase
		{
		public:
			StateBeginPick() : StatePickingBase("BeginPick") {}

		public:
			virtual void Update(float deltaTime) override;
			virtual std::string Signaled(SignalId signal) override;
		};

		class StateBeginBoxPick : public StatePickingBase
		{
		public:
			StateBeginBoxPick() : StatePickingBase("BeginBoxPick") {}

		public:
			virtual void Update(float deltaTime) override;
			virtual std::string Signaled(SignalId signal) override;

		private:
			void GetMouseRect(glm::vec2& min, glm::vec2& max);
		};

		class StateEndPick : public StatePickingBase
		{
		public:
			StateEndPick() : StatePickingBase("EndPick") {}

		public:
			virtual void Update(float deltaTime) override;
			virtual std::string Signaled(SignalId signal) override;
		};

		class SelectMod : public BaseMod
		{
		public:
			SelectMod() : BaseMod(ModId::Select) { Init(); }

			virtual void Init() override;
			virtual void Update(float deltaTime) override;
		};

		class CursorMod : public BaseMod
		{
		public:
			CursorMod() : BaseMod(ModId::Cursor) { Init(); }

			virtual void Init() override;
			virtual void Update(float deltaTime) override;
		};
	}
}
