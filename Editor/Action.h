#pragma once

#include <vector>

#include "Types.h"
#include "Animation.h"

namespace ToolKit
{
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

    class DeleteAction : public Action
    {
     public:
      explicit DeleteAction(Entity* ntt);
      virtual ~DeleteAction();

      void Undo() override;
      void Redo() override;

     private:
      void HandleAnimRecords(Entity* ntt);

     private:
      Entity* m_ntt;
      ULongID m_parentId;
      EntityIdArray m_children;
      AnimRecordRawPtrArray m_records;
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

      ActionManager(ActionManager const&) = delete;
      void operator=(ActionManager const&) = delete;

      void Init();
      void UnInit();
      void AddAction(Action* action);
      void GroupLastActions(int n);
      void BeginActionGroup();
      void RemoveLastAction();
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
      bool m_actionGrouping;
    };

  }  // namespace Editor
}  // namespace ToolKit
