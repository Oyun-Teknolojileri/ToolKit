#pragma once

#include <memory>
#include <vector>

#include "ToolKit.h"
#include "StateMachine.h"
#include "App.h"

namespace ToolKit
{

  namespace Editor
  {

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
      explicit BaseMod(ModId id);
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
      // If set true, sets the given mod. Else does nothing.
      void SetMod(bool set, ModId mod);

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
      static const String Null;
      static const String StateBeginPick;
      static const String StateBeginBoxPick;
      static const String StateEndPick;
      static const String StateDeletePick;
      static const String StateTransformBegin;
      static const String StateTransformTo;
      static const String StateTransformEnd;
      static const String StateDuplicate;
    };

    class StatePickingBase : public State
    {
     public:
      StatePickingBase();
      void TransitionIn(State* prevState) override;
      void TransitionOut(State* nextState) override;
      bool IsIgnored(ULongID id);
      void PickDataToEntityId(EntityIdArray& ids);

     public:
      // Picking data.
      std::vector<Vec2> m_mouseData;
      std::vector<EditorScene::PickData> m_pickData;
      EntityIdArray m_ignoreList;

      // Debug models.
      static std::shared_ptr<Arrow2d> m_dbgArrow;
      static std::shared_ptr<LineBatch> m_dbgFrustum;
    };

    class StateBeginPick : public StatePickingBase
    {
     public:
      void TransitionIn(State* prevState) override;
      SignalId Update(float deltaTime) override;
      String Signaled(SignalId signal) override;
      String GetType() override { return StateType::StateBeginPick; }
    };

    class StateBeginBoxPick : public StatePickingBase
    {
     public:
      SignalId Update(float deltaTime) override;
      String Signaled(SignalId signal) override;
      String GetType() override { return StateType::StateBeginBoxPick; }

     private:
      void GetMouseRect(Vec2& min, Vec2& max);
    };

    class StateEndPick : public StatePickingBase
    {
     public:
      SignalId Update(float deltaTime) override;
      String Signaled(SignalId signal) override;
      String GetType() override { return StateType::StateEndPick; }
    };

    class StateDeletePick : public StatePickingBase
    {
     public:
      SignalId Update(float deltaTime) override;
      String Signaled(SignalId signal) override;
      String GetType() override { return StateType::StateDeletePick; }
    };

    class StateDuplicate : public State
    {
     public:
      void TransitionIn(State* prevState) override;
      void TransitionOut(State* nextState) override;
      SignalId Update(float deltaTime) override;
      String Signaled(SignalId signal) override;
      String GetType() override { return StateType::StateDuplicate; };
    };

    // Mods
    //////////////////////////////////////////////////////////////////////////

    class SelectMod : public BaseMod
    {
     public:
      SelectMod();

      void Init() override;
      void Update(float deltaTime) override;
    };

    class CursorMod : public BaseMod
    {
     public:
      CursorMod();

      void Init() override;
      void Update(float deltaTime) override;
    };

  }  // namespace Editor
}  // namespace ToolKit
