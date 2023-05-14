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
      explicit DeleteAction(Entity* ntt);
      virtual ~DeleteAction();

      void Undo() override;
      void Redo() override;

     private:
      Entity* m_ntt;
      ULongID m_parentId;
      EntityIdArray m_children;
      bool m_actionComitted;
    };

    class CreateAction : public Action
    {
     public:
      explicit CreateAction(Entity* ntt);
      virtual ~CreateAction();

      void Undo() override;
      void Redo() override;

     private:
      void SwapSelection();

     private:
      Entity* m_ntt;
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
