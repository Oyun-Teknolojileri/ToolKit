/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "EditorScene.h"

#include <StateMachine.h>

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
      Scale,
      Anchor
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

      ModManager(const ModManager&)     = delete;
      void operator=(const ModManager&) = delete;

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
      static const String StateAnchorBegin;
      static const String StateAnchorTo;
      static const String StateAnchorEnd;
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

  } // namespace Editor
} // namespace ToolKit
