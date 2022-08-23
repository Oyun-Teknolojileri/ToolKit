#include "Action.h"

#include <utility>

#include "App.h"
#include "EditorScene.h"
#include "GlobalDef.h"

namespace ToolKit
{
  namespace Editor
  {

    // Action
    //////////////////////////////////////////////////////////////////////////

    Action::Action()
    {
    }

    Action::~Action()
    {
      for (Action* a : m_group)
      {
        SafeDel(a);
      }
      m_group.clear();
    }

    DeleteAction::DeleteAction(Entity* ntt)
    {
      m_ntt = ntt;
      Redo();
    }

    DeleteAction::~DeleteAction()
    {
      if (m_actionComitted)
      {
        SafeDel(m_ntt);
      }
    }

    void HandleCompSpecificOps(Entity* ntt, bool isActionCommitted)
    {
      if (isActionCommitted)
      {
        AnimControllerComponentPtr comp =
          ntt->GetComponent<AnimControllerComponent>();
        if (comp)
        {
          comp->Stop();
        }
      }
      else
      {
      }
    }

    void DeleteAction::Undo()
    {
      assert(m_ntt != nullptr);

      EditorScenePtr currScene = g_app->GetCurrentScene();
      currScene->AddEntity(m_ntt);
      if (m_parentId != NULL_HANDLE)
      {
        if (Entity* pEntt = currScene->GetEntity(m_parentId))
        {
          m_ntt->m_node->OrphanSelf();
          pEntt->m_node->AddChild(m_ntt->m_node);
        }
      }

      m_actionComitted = false;
      HandleCompSpecificOps(m_ntt, m_actionComitted);
    }

    void DeleteAction::Redo()
    {
      if (Node* pNode = m_ntt->m_node->m_parent)
      {
        if (pNode->m_entity)
        {
          m_parentId = pNode->m_entity->GetIdVal();
        }
        pNode->Orphan(m_ntt->m_node);
      }

      g_app->GetCurrentScene()->RemoveEntity(m_ntt->GetIdVal());
      m_actionComitted = true;
      HandleCompSpecificOps(m_ntt, m_actionComitted);
    }

    CreateAction::CreateAction(Entity* ntt)
    {
      m_ntt = ntt;
      m_actionComitted = true;
      EditorScenePtr currScene = g_app->GetCurrentScene();
      currScene->GetSelectedEntities(m_selecteds);
      currScene->AddEntity(ntt);
    }

    CreateAction::~CreateAction()
    {
      if (!m_actionComitted)
      {
        SafeDel(m_ntt);
      }
    }

    void CreateAction::Undo()
    {
      SwapSelection();
      g_app->GetCurrentScene()->RemoveEntity(m_ntt->GetIdVal());
      m_actionComitted = false;
    }

    void CreateAction::Redo()
    {
      SwapSelection();
      g_app->GetCurrentScene()->AddEntity(m_ntt);
      m_actionComitted = true;
    }

    void CreateAction::SwapSelection()
    {
      EntityIdArray selection;
      EditorScenePtr currScene = g_app->GetCurrentScene();
      currScene->GetSelectedEntities(selection);
      currScene->AddToSelection(m_selecteds, false);
      std::swap(m_selecteds, selection);
    }

    DeleteComponentAction::DeleteComponentAction(ComponentPtr com)
    {
      m_com = com;
      switch (com->GetType())
      {
        case ComponentType::AnimControllerComponent:
        {
          reinterpret_cast<AnimControllerComponent*>(com.get())->Stop();
        }
        break;
        default:
        break;
      };
      Redo();
    }

    void DeleteComponentAction::Undo()
    {
      if (m_com->m_entity)
      {
        m_com->m_entity->AddComponent(m_com);
      }
    }

    void DeleteComponentAction::Redo()
    {
      if (m_com->m_entity)
      {
        m_com->m_entity->RemoveComponent(m_com->m_id);
      }
    }

    // ActionManager
    //////////////////////////////////////////////////////////////////////////

    ActionManager ActionManager::m_instance;

    ActionManager::ActionManager()
    {
      m_initiated = false;
      m_stackPointer = 0;
    }

    ActionManager::~ActionManager()
    {
      assert(m_initiated == false && "Call ActionManager::UnInit.");
    }

    void ActionManager::Init()
    {
      m_initiated = true;
      m_actionGrouping = false;
    }

    void ActionManager::UnInit()
    {
      for (Action* action : m_actionStack)
      {
        SafeDel(action);
      }
      m_actionStack.clear();

      m_initiated = false;
    }

    void ActionManager::AddAction(Action* action)
    {
      if (m_stackPointer > -1)
      {
        if (m_stackPointer < static_cast<int>(m_actionStack.size()) - 1)
        {
          // All actions above stack pointer are invalidated.
          for (size_t i = m_stackPointer + 1; i < m_actionStack.size(); i++)
          {
            Action* a = m_actionStack[i];
            SafeDel(a);
          }

          m_actionStack.erase
          (
            m_actionStack.begin() + m_stackPointer + 1,
            m_actionStack.end()
          );
        }
      }
      else
      {
        for (size_t i = 0; i < m_actionStack.size(); i++)
        {
          Action* a = m_actionStack[i];
          SafeDel(a);
        }

        m_actionStack.clear();
      }

      m_actionStack.push_back(action);
      if (m_actionStack.size() > g_maxUndoCount && !m_actionGrouping)
      {
        Action* a = m_actionStack.front();
        SafeDel(a);

        pop_front(m_actionStack);
      }
      m_stackPointer = static_cast<int>(m_actionStack.size()) - 1;
    }

    void ActionManager::GroupLastActions(int n)
    {
      if (n == 0)
      {
        return;
      }

      // Sanity Checks.
      assert(m_stackPointer == static_cast<int>(m_actionStack.size()) - 1
      && "Call grouping right after add.");
      if (n >= static_cast<int>(m_actionStack.size()) && !m_actionGrouping)
      {
        assert(static_cast<int>(m_actionStack.size()) >=
        n
        && "We can't stack more than we have. Try using BeginActionGroup()"
        " to pass a series of action as a group");
        return;
      }

      int begIndx = static_cast<int>(m_actionStack.size()) - n;
      Action* root = m_actionStack[begIndx++];
      root->m_group.reserve(n - 1);
      for (int i = begIndx; i < static_cast<int>(m_actionStack.size()); i++)
      {
        root->m_group.push_back(m_actionStack[i]);
      }

      m_actionStack.erase
      (
        m_actionStack.begin() + begIndx,
        m_actionStack.end()
      );
      m_stackPointer = static_cast<int>(m_actionStack.size()) - 1;
      m_actionGrouping = false;
    }

    void ActionManager::BeginActionGroup()
    {
      m_actionGrouping = true;
    }

    void ActionManager::RemoveLastAction()
    {
      if (!m_actionStack.empty())
      {
        if (m_stackPointer > -1)
        {
          Action* action = m_actionStack[m_stackPointer];
          SafeDel(action);
          m_actionStack.erase(m_actionStack.begin() + m_stackPointer);
          m_stackPointer--;
        }
      }
    }

    void ActionManager::Undo()
    {
      if (!m_actionStack.empty())
      {
        if (m_stackPointer > -1)
        {
          Action* action = m_actionStack[m_stackPointer];

          // Undo in reverse order.
          for
          (
            int i = static_cast<int>(action->m_group.size()) - 1;
            i > -1;
            i--
          )
          {
            action->m_group[i]->Undo();
          }
          action->Undo();
          m_stackPointer--;
        }
      }
    }

    void ActionManager::Redo()
    {
      if (m_actionStack.empty())
      {
        return;
      }

      if (m_stackPointer < static_cast<int>(m_actionStack.size()) - 1)
      {
        Action* action = m_actionStack[m_stackPointer + 1];
        action->Redo();
        for (Action* ga : action->m_group)
        {
          ga->Redo();
        }

        m_stackPointer++;
      }
    }

    ActionManager* ActionManager::GetInstance()
    {
      return &m_instance;
    }

  }  // namespace Editor
}  // namespace ToolKit
