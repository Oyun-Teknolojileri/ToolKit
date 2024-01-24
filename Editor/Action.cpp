/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Action.h"

#include "App.h"

#include <AnimationControllerComponent.h>
#include <Prefab.h>

#include <DebugNew.h>

namespace ToolKit
{
  namespace Editor
  {

    // Action
    //////////////////////////////////////////////////////////////////////////

    Action::Action() {}

    Action::~Action()
    {
      for (Action* a : m_group)
      {
        SafeDel(a);
      }
      m_group.clear();
    }

    DeleteAction::DeleteAction(EntityPtr ntt)
    {
      m_parentId = NULL_HANDLE;
      m_ntt      = ntt;
      Redo();
    }

    DeleteAction::~DeleteAction()
    {
      if (m_actionComitted)
      {
        m_ntt = nullptr;
      }
    }

    void HandleCompSpecificOps(EntityPtr ntt, bool isActionCommitted)
    {
      if (isActionCommitted)
      {
        if (AnimControllerComponentPtr comp = ntt->GetComponent<AnimControllerComponent>())
        {
          comp->Stop();
        }
      }
    }

    void DeleteAction::Undo()
    {
      assert(m_ntt != nullptr);

      EditorScenePtr currScene = g_app->GetCurrentScene();
      if (m_ntt->IsA<Prefab>())
      {
        static_cast<Prefab*>(m_ntt.get())->Link();
      }
      currScene->AddEntity(m_ntt);

      if (m_parentId != NULL_HANDLE)
      {
        if (EntityPtr parent = currScene->GetEntity(m_parentId))
        {
          m_ntt->m_node->OrphanSelf();
          parent->m_node->AddChild(m_ntt->m_node);
        }
      }

      m_actionComitted = false;
      HandleCompSpecificOps(m_ntt, m_actionComitted);
    }

    void DeleteAction::Redo()
    {
      if (Node* pNode = m_ntt->m_node->m_parent)
      {
        if (EntityPtr ntt = pNode->OwnerEntity())
        {
          m_parentId = ntt->GetIdVal();
        }
        pNode->Orphan(m_ntt->m_node);
      }

      g_app->GetCurrentScene()->RemoveEntity(m_ntt->GetIdVal());
      if (m_ntt->IsA<Prefab>())
      {
        static_cast<Prefab*>(m_ntt.get())->Unlink();
      }
      m_actionComitted = true;
      HandleCompSpecificOps(m_ntt, m_actionComitted);
    }

    CreateAction::CreateAction(EntityPtr ntt)
    {
      m_ntt                    = ntt;
      m_actionComitted         = true;
      EditorScenePtr currScene = g_app->GetCurrentScene();
      currScene->GetSelectedEntities(m_selecteds);
      currScene->AddEntity(ntt);
    }

    CreateAction::~CreateAction()
    {
      if (!m_actionComitted)
      {
        m_ntt = nullptr;
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
      IDArray selection;
      EditorScenePtr currScene = g_app->GetCurrentScene();
      currScene->GetSelectedEntities(selection);
      currScene->AddToSelection(m_selecteds, false);
      std::swap(m_selecteds, selection);
    }

    DeleteComponentAction::DeleteComponentAction(ComponentPtr com)
    {
      m_com = com;
      if (AnimControllerComponent* ac = com->As<AnimControllerComponent>())
      {
        ac->Stop();
      }

      Redo();
    }

    void DeleteComponentAction::Undo()
    {
      if (EntityPtr owner = m_com->OwnerEntity())
      {
        owner->AddComponent(m_com);

        EditorScenePtr currScene = g_app->GetCurrentScene();
        currScene->ValidateBillboard(m_com->OwnerEntity());
      }
    }

    void DeleteComponentAction::Redo()
    {
      if (EntityPtr owner = m_com->OwnerEntity())
      {
        owner->RemoveComponent(m_com->Class());
      }
    }

    // ActionManager
    //////////////////////////////////////////////////////////////////////////

    ActionManager ActionManager::m_instance;

    ActionManager::ActionManager()
    {
      m_initiated      = false;
      m_stackPointer   = 0;
      m_actionGrouping = false;
    }

    ActionManager::~ActionManager() { assert(m_initiated == false && "Call ActionManager::UnInit."); }

    void ActionManager::Init()
    {
      m_initiated      = true;
      m_actionGrouping = false;
    }

    void ActionManager::UnInit()
    {
      ClearAllActions();
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

          m_actionStack.erase(m_actionStack.begin() + m_stackPointer + 1, m_actionStack.end());
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
      assert(m_stackPointer == static_cast<int>(m_actionStack.size()) - 1 && "Call grouping right after add.");
      if (n >= static_cast<int>(m_actionStack.size()) && !m_actionGrouping)
      {
        assert(static_cast<int>(m_actionStack.size()) >= n &&
               "We can't stack more than we have. Try using BeginActionGroup()"
               " to pass a series of action as a group");
        return;
      }

      int begIndx  = static_cast<int>(m_actionStack.size()) - n;
      Action* root = m_actionStack[begIndx++];
      root->m_group.reserve(n - 1);
      for (int i = begIndx; i < static_cast<int>(m_actionStack.size()); i++)
      {
        root->m_group.push_back(m_actionStack[i]);
      }

      m_actionStack.erase(m_actionStack.begin() + begIndx, m_actionStack.end());
      m_stackPointer   = static_cast<int>(m_actionStack.size()) - 1;
      m_actionGrouping = false;
    }

    void ActionManager::BeginActionGroup() { m_actionGrouping = true; }

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
          for (int i = static_cast<int>(action->m_group.size()) - 1; i > -1; i--)
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

    void ActionManager::ClearAllActions()
    {
      for (Action* action : m_actionStack)
      {
        SafeDel(action);
      }
      m_actionStack.clear();
      m_stackPointer = 0;
    }

    ActionManager* ActionManager::GetInstance() { return &m_instance; }

  } // namespace Editor
} // namespace ToolKit
