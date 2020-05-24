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

		class Action
		{
		public:
			Action();
			virtual ~Action();

			virtual void Undo() = 0;
			virtual void Redo() = 0;

		public:
			std::vector<Action*> m_group;
		};

		// ActionManager
		//////////////////////////////////////////////////////////////////////////

		class ActionManager
		{
		public:
			~ActionManager();

			ActionManager(ActionManager const&) = delete;
			void operator=(ActionManager const&) = delete;

			void Init();
			void UnInit();
			void AddAction(Action* action);
			void GroupLastActions(int n);
			void Undo();
			void Redo();
			static ActionManager* GetInstance();

		private:
			ActionManager();

		private:
			static ActionManager m_instance;
			std::vector<Action*> m_actionStack;
			int m_stackPointer;
			bool m_initiated;
		};

		// ModManager
		//////////////////////////////////////////////////////////////////////////

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

		protected:
			static int GetNextSignalId();

		public:
			ModId m_id;
			StateMachine* m_stateMachine;

			// Signals.
			static SignalId m_leftMouseBtnDownSgnl;
			static SignalId m_leftMouseBtnUpSgnl;
			static SignalId m_leftMouseBtnDragSgnl;
			static SignalId m_mouseMoveSgnl;
			static SignalId m_backToStart;
			static SignalId m_delete;
			static SignalId m_duplicate;
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

		// States
		//////////////////////////////////////////////////////////////////////////

		class StateType
		{
		public:
			const static String Null;
			const static String StateBeginPick;
			const static String StateBeginBoxPick;
			const static String StateEndPick;
			const static String StateDeletePick;
			const static String StateTransformBegin;
			const static String StateTransformTo;
			const static String StateTransformEnd;
			const static String StateDuplicate;
		};

		class StatePickingBase : public State
		{
		public:
			StatePickingBase();
			virtual void TransitionIn(State* prevState) override;
			virtual void TransitionOut(State* nextState) override;
			bool IsIgnored(EntityId id);
			void PickDataToEntityId(EntityIdArray& ids);

		public:
			// Picking data.
			std::vector<Vec2> m_mouseData;
			std::vector<Scene::PickData> m_pickData;
			EntityIdArray m_ignoreList;

			// Debug models.
			static std::shared_ptr<Arrow2d> m_dbgArrow;
			static std::shared_ptr<LineBatch> m_dbgFrustum;
		};

		class StateBeginPick : public StatePickingBase
		{
		public:
			virtual void Update(float deltaTime) override;
			virtual String Signaled(SignalId signal) override;
			virtual String GetType() override { return StateType::StateBeginPick; }
		};

		class StateBeginBoxPick : public StatePickingBase
		{
		public:
			virtual void Update(float deltaTime) override;
			virtual String Signaled(SignalId signal) override;
			virtual String GetType() override { return StateType::StateBeginBoxPick; }

		private:
			void GetMouseRect(Vec2& min, Vec2& max);
		};

		class StateEndPick : public StatePickingBase
		{
		public:
			virtual void Update(float deltaTime) override;
			virtual String Signaled(SignalId signal) override;
			virtual String GetType() override { return StateType::StateEndPick; }
		};

		class DeleteAction : public Action 
		{
		public:
			DeleteAction(Entity* ntt);
			virtual ~DeleteAction();

			virtual void Undo() override;
			virtual void Redo() override;

		private:
			void HandleAnimRecords(Entity* ntt, bool remove);

		private:
			Entity* m_ntt;
			std::vector<AnimRecord> m_records;
			bool m_actionComitted;
		};

		class StateDeletePick : public StatePickingBase
		{
		public:
			virtual void Update(float deltaTime) override;
			virtual String Signaled(SignalId signal) override;
			virtual String GetType() override { return StateType::StateDeletePick; }
		};

		class StateDuplicate : public State 
		{
		public:
			virtual void TransitionIn(State* prevState) override;
			virtual void TransitionOut(State* nextState) override;
			virtual void Update(float deltaTime) override;
			virtual String Signaled(SignalId signal) override;
			virtual String GetType() { return StateType::StateDuplicate; };
		};

		// Mods
		//////////////////////////////////////////////////////////////////////////

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
