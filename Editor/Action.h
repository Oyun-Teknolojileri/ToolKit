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

#include "Animation.h"
#include "Types.h"

namespace ToolKit
{
  namespace Editor
  {

    typedef std::vector<class Action*> ActionRawPtrArray;

    class Action
    {
     public:
      Action();
      virtual ~Action();

      virtual void Undo() = 0;
      virtual void Redo() = 0;

     public:
      ActionRawPtrArray m_group;
    };

    class DeleteAction : public Action
    {
     public:
      explicit DeleteAction(EntityPtr ntt);
      virtual ~DeleteAction();

      void Undo() override;
      void Redo() override;

     private:
      EntityPtr m_ntt;
      ULongID m_parentId;
      EntityIdArray m_children;
      bool m_actionComitted;
    };

    class CreateAction : public Action
    {
     public:
      explicit CreateAction(EntityPtr ntt);
      virtual ~CreateAction();

      void Undo() override;
      void Redo() override;

     private:
      void SwapSelection();

     private:
      EntityPtr m_ntt;
      bool m_actionComitted;
      EntityIdArray m_selecteds;
    };

    class DeleteComponentAction : public Action
    {
     public:
      explicit DeleteComponentAction(ComponentPtr com);

      void Undo() override;
      void Redo() override;

     private:
      ComponentPtr m_com;
    };

    // ActionManager
    //////////////////////////////////////////////////////////////////////////

    class ActionManager
    {
     public:
      ~ActionManager();

      ActionManager(const ActionManager&)  = delete;
      void operator=(const ActionManager&) = delete;

      void Init();
      void UnInit();
      void AddAction(Action* action);
      void GroupLastActions(int n);
      void BeginActionGroup();
      void RemoveLastAction();
      void Undo();
      void Redo();
      void ClearAllActions();
      static ActionManager* GetInstance();

     private:
      ActionManager();

     private:
      static ActionManager m_instance;
      ActionRawPtrArray m_actionStack;

      int m_stackPointer;
      bool m_initiated;
      bool m_actionGrouping;
    };

  } // namespace Editor
} // namespace ToolKit
