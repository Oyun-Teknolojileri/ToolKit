#include "Mod.h"

#include <utility>
#include <vector>
#include <memory>
#include <algorithm>

#include "GlobalDef.h"
#include "EditorViewport.h"
#include "Node.h"
#include "Primative.h"
#include "Grid.h"
#include "Camera.h"
#include "Gizmo.h"
#include "TransformMod.h"
#include "ConsoleWindow.h"
#include "Util.h"
#include "DirectionComponent.h"
#include "DebugNew.h"

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
      HandleAnimRecords(m_ntt);
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
    }

    void DeleteAction::HandleAnimRecords(Entity* ntt)
    {
      if (m_actionComitted)
      {
        m_records.clear();
        auto removeItr = std::remove_if
        (
          GetAnimationPlayer()->m_records.begin(),
          GetAnimationPlayer()->m_records.end(),
          [this, ntt](AnimRecord* record)
          {
            if (ntt == record->m_entity)
            {
              m_records.push_back(record);
              return true;
            }

            return false;
          }
        );

        if (GetAnimationPlayer()->m_records.end() != removeItr)
        {
          GetAnimationPlayer()->m_records.erase(removeItr);
        }
      }
      else
      {
        GetAnimationPlayer()->m_records.insert
        (
          GetAnimationPlayer()->m_records.end(),
          m_records.begin(),
          m_records.end()
        );
      }
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

    // ModManager
    //////////////////////////////////////////////////////////////////////////

    ModManager ModManager::m_instance;

    ModManager::~ModManager()
    {
      assert(m_initiated == false && "Call UnInit.");
    }

    ModManager* ModManager::GetInstance()
    {
      return &m_instance;
    }

    void ModManager::Update(float deltaTime)
    {
      if (m_modStack.empty())
      {
        return;
      }

      BaseMod* currentMod = m_modStack.back();
      currentMod->Update(deltaTime);
    }

    void ModManager::DispatchSignal(SignalId signal)
    {
      if (m_modStack.empty())
      {
        return;
      }

      m_modStack.back()->Signal(signal);
    }

    void ModManager::SetMod(bool set, ModId mod)
    {
      if (set)
      {
        if (m_modStack.back()->m_id != ModId::Base)
        {
          BaseMod* prevMod = m_modStack.back();
          prevMod->UnInit();
          SafeDel(prevMod);
          m_modStack.pop_back();
        }

        static String modNameDbg;
        BaseMod* nextMod = nullptr;
        switch (mod)
        {
        case ModId::Select:
          nextMod = new SelectMod();
          modNameDbg = "Mod: Select";
          break;
        case ModId::Cursor:
          nextMod = new CursorMod();
          modNameDbg = "Mod: Cursor";
          break;
        case ModId::Move:
          nextMod = new TransformMod(mod);
          modNameDbg = "Mod: Move";
          break;
        case ModId::Rotate:
          nextMod = new TransformMod(mod);
          modNameDbg = "Mod: Rotate";
          break;
        case ModId::Scale:
          nextMod = new TransformMod(mod);
          modNameDbg = "Mod: Scale";
          break;
        case ModId::Base:
        default:
          break;
        }

        assert(nextMod);
        if (nextMod != nullptr)
        {
          nextMod->Init();
          m_modStack.push_back(nextMod);

          // #ConsoleDebug_Mod
          if (g_app->m_showStateTransitionsDebug)
          {
            ConsoleWindow* console = g_app->GetConsole();
            if (console != nullptr)
            {
              console->AddLog(modNameDbg, "ModDbg");
            }
          }
        }

        /*
        * If the state is changed while the previous state is being actively
        *  used (in StateTransitionTo state), delete the last function
        * pointers from the array, since the function
        * parameters are not valid anymore.
        */
        EditorViewport* vp = g_app->GetActiveViewport();
        if (vp == nullptr)
        {
          return;
        }
        vp->m_drawCommands.clear();
      }
    }

    ModManager::ModManager()
    {
      m_initiated = false;
    }

    void ModManager::Init()
    {
      m_modStack.push_back(new BaseMod(ModId::Base));
      m_initiated = true;
    }

    void ModManager::UnInit()
    {
      for (BaseMod* mod : m_modStack)
      {
        SafeDel(mod);
      }
      m_modStack.clear();

      m_initiated = false;
    }

    // ModManager
    //////////////////////////////////////////////////////////////////////////

    // Signal definitions.
    SignalId BaseMod::m_leftMouseBtnDownSgnl = BaseMod::GetNextSignalId();
    SignalId BaseMod::m_leftMouseBtnUpSgnl = BaseMod::GetNextSignalId();
    SignalId BaseMod::m_leftMouseBtnDragSgnl = BaseMod::GetNextSignalId();
    SignalId BaseMod::m_mouseMoveSgnl = BaseMod::GetNextSignalId();
    SignalId BaseMod::m_backToStart = BaseMod::GetNextSignalId();
    SignalId BaseMod::m_delete = BaseMod::GetNextSignalId();
    SignalId BaseMod::m_duplicate = BaseMod::GetNextSignalId();

    BaseMod::BaseMod(ModId id)
    {
      m_id = id;
      m_stateMachine = new StateMachine();
    }

    BaseMod::~BaseMod()
    {
      SafeDel(m_stateMachine);
    }

    void BaseMod::Init()
    {
    }

    void BaseMod::UnInit()
    {
    }

    void BaseMod::Update(float deltaTime)
    {
      m_stateMachine->Update(deltaTime);
    }

    void BaseMod::Signal(SignalId signal)
    {
      State* prevStateDbg = m_stateMachine->m_currentState;

      m_stateMachine->Signal(signal);

      // #ConsoleDebug_Mod
      if (g_app->m_showStateTransitionsDebug)
      {
        State* nextState = m_stateMachine->m_currentState;
        if (prevStateDbg != nextState)
        {
          if (prevStateDbg && nextState)
          {
            if (ConsoleWindow* consol = g_app->GetConsole())
            {
              String log = "\t" + prevStateDbg->GetType() + " -> "
              + nextState->GetType();
              consol->AddLog(log, "ModDbg");
            }
          }
        }
      }
    }

    int BaseMod::GetNextSignalId()
    {
      static int signalCounter = 100;
      return ++signalCounter;
    }

    // States
    //////////////////////////////////////////////////////////////////////////

    const String StateType::Null = "";
    const String StateType::StateBeginPick = "StateBeginPick";
    const String StateType::StateBeginBoxPick = "StateBeginBoxPick";
    const String StateType::StateEndPick = "StateEndPick";
    const String StateType::StateDeletePick = "StateDeletePick";
    const String StateType::StateTransformBegin = "StateTransformBegin";
    const String StateType::StateTransformTo = "StateTransformTo";
    const String StateType::StateTransformEnd = "StateTransformEnd";
    const String StateType::StateDuplicate = "StateDuplicate";

    std::shared_ptr<Arrow2d> StatePickingBase::m_dbgArrow = nullptr;
    std::shared_ptr<LineBatch> StatePickingBase::m_dbgFrustum = nullptr;

    StatePickingBase::StatePickingBase()
    {
      m_mouseData.resize(2);
    }

    void StatePickingBase::TransitionIn(State* prevState)
    {
    }

    void StatePickingBase::TransitionOut(State* nextState)
    {
      if
      (
        StatePickingBase* baseState =
        dynamic_cast<StatePickingBase*> (nextState)
      )
      {
        baseState->m_ignoreList = m_ignoreList;
        baseState->m_mouseData = m_mouseData;

        if (nextState->GetType() != StateType::StateBeginPick)
        {
          baseState->m_pickData = m_pickData;
        }
      }

      m_pickData.clear();
    }

    bool StatePickingBase::IsIgnored(ULongID id)
    {
      return std::find
      (
        m_ignoreList.begin(),
        m_ignoreList.end(),
        id
      ) != m_ignoreList.end();
    }

    void StatePickingBase::PickDataToEntityId(EntityIdArray& ids)
    {
      for (EditorScene::PickData& pd : m_pickData)
      {
        if (pd.entity != nullptr)
        {
          ids.push_back(pd.entity->GetIdVal());
        }
        else
        {
          ids.push_back(NULL_HANDLE);
        }
      }
    }

    void StateBeginPick::TransitionIn(State* prevState)
    {
      // Construct the ignore list.
      m_ignoreList.clear();
      EntityRawPtrArray ignores;
      if (EditorViewport* vp = g_app->GetActiveViewport())
      {
        if (vp->GetType() == Window::Type::Viewport)
        {
          ignores = g_app->GetCurrentScene()->Filter
          (
            [](Entity* ntt) -> bool
            {
              return ntt->IsSurfaceInstance();
            }
          );
        }

        if (vp->GetType() == Window::Type::Viewport2d)
        {
          ignores = g_app->GetCurrentScene()->Filter
          (
            [](Entity* ntt) -> bool
            {
              return !ntt->IsSurfaceInstance();
            }
          );
        }
      }

      ToEntityIdArray(m_ignoreList, ignores);
      m_ignoreList.push_back(g_app->m_grid->GetIdVal());
    }

    SignalId StateBeginPick::Update(float deltaTime)
    {
      return NullSignal;
    }

    String StateBeginPick::Signaled(SignalId signal)
    {
      if (signal == BaseMod::m_leftMouseBtnDownSgnl)
      {
        EditorViewport* vp = g_app->GetActiveViewport();
        if (vp != nullptr)
        {
          m_mouseData[0] = vp->GetLastMousePosScreenSpace();
        }
      }

      if (signal == BaseMod::m_leftMouseBtnUpSgnl)
      {
        EditorViewport* vp = g_app->GetActiveViewport();
        if (vp != nullptr)
        {
          m_mouseData[0] = vp->GetLastMousePosScreenSpace();

          Ray ray = vp->RayFromMousePosition();
          EditorScenePtr currScene = g_app->GetCurrentScene();
          EditorScene::PickData pd = currScene->PickObject(ray, m_ignoreList);
          m_pickData.push_back(pd);

          if (g_app->m_showPickingDebug)
          {
            g_app->m_cursor->m_worldLocation = pd.pickPos;

            if (m_dbgArrow == nullptr)
            {
              m_dbgArrow = std::shared_ptr<Arrow2d>(new Arrow2d(AxisLabel::X));
              m_ignoreList.push_back(m_dbgArrow->GetIdVal());
              currScene->AddEntity(m_dbgArrow.get());
            }

            m_dbgArrow->m_node->SetTranslation(ray.position);
            m_dbgArrow->m_node->SetOrientation
            (
              RotationTo(X_AXIS, ray.direction)
            );
          }

          return StateType::StateEndPick;
        }
      }

      if (signal == BaseMod::m_leftMouseBtnDragSgnl)
      {
        return StateType::StateBeginBoxPick;
      }

      if (signal == BaseMod::m_delete)
      {
        return StateType::StateDeletePick;
      }

      return StateType::Null;
    }

    SignalId StateBeginBoxPick::Update(float deltaTime)
    {
      return NullSignal;
    }

    String StateBeginBoxPick::Signaled(SignalId signal)
    {
      if (signal == BaseMod::m_leftMouseBtnUpSgnl)
      {
        // Frustum - AABB test.
        EditorViewport* vp = g_app->GetActiveViewport();
        if (vp != nullptr)
        {
          Camera* cam = vp->GetCamera();

          Vec2 rect[4];
          GetMouseRect(rect[0], rect[2]);

          rect[1].x = rect[2].x;
          rect[1].y = rect[0].y;
          rect[3].x = rect[0].x;
          rect[3].y = rect[2].y;

          std::vector<Ray> rays;
          std::vector<Vec3> rect3d;

          // Front rectangle.
          Vec3 lensLoc = cam->m_node->GetTranslation
          (
            TransformationSpace::TS_WORLD
          );
          for (int i = 0; i < 4; i++)
          {
            Vec2 p = vp->TransformScreenToViewportSpace(rect[i]);
            Vec3 p0 = vp->TransformViewportToWorldSpace(p);
            rect3d.push_back(p0);
            if (cam->IsOrtographic())
            {
              rays.push_back
              (
                {
                  lensLoc,
                  cam->GetComponent<DirectionComponent>()->GetDirection()
                }
              );
            }
            else
            {
              rays.push_back({ lensLoc, glm::normalize(p0 - lensLoc) });
            }
          }

          // Back rectangle.
          float depth = 1000.0f;
          for (int i = 0; i < 4; i++)
          {
            Vec3 p = rect3d[i] + rays[i].direction * depth;
            rect3d.push_back(p);
          }

          // Frustum from 8 points.
          Frustum frustum;
          std::vector<Vec3> planePnts;
          planePnts = { rect3d[0], rect3d[7], rect3d[4] };  // Left plane.
          frustum.planes[0] = PlaneFrom(planePnts.data());

          planePnts = { rect3d[5], rect3d[6], rect3d[1] };  // Right plane.
          frustum.planes[1] = PlaneFrom(planePnts.data());

          planePnts = { rect3d[4], rect3d[5], rect3d[0] };  // Top plane.
          frustum.planes[2] = PlaneFrom(planePnts.data());

          planePnts = { rect3d[3], rect3d[6], rect3d[7] };  // Bottom plane.
          frustum.planes[3] = PlaneFrom(planePnts.data());

          planePnts = { rect3d[0], rect3d[1], rect3d[3] };  // Near plane.
          frustum.planes[4] = PlaneFrom(planePnts.data());

          planePnts = { rect3d[7], rect3d[5], rect3d[4] };  // Far plane.
          frustum.planes[5] = PlaneFrom(planePnts.data());

          // Perform picking.
          std::vector<EditorScene::PickData> ntties;
          EditorScenePtr currScene = g_app->GetCurrentScene();
          currScene->PickObject(frustum, ntties, m_ignoreList);
          m_pickData.insert(m_pickData.end(), ntties.begin(), ntties.end());

          // Debug draw the picking frustum.
          if (g_app->m_showPickingDebug)
          {
            std::vector<Vec3> corners =
            {
              rect3d[0], rect3d[1], rect3d[1], rect3d[2], rect3d[2],
              rect3d[3], rect3d[3], rect3d[0],
              rect3d[0], rect3d[0] + rays[0].direction * depth,
              rect3d[1], rect3d[1] + rays[1].direction * depth,
              rect3d[2], rect3d[2] + rays[2].direction * depth,
              rect3d[3], rect3d[3] + rays[3].direction * depth,
              rect3d[0] + rays[0].direction * depth,
              rect3d[1] + rays[1].direction * depth,
              rect3d[1] + rays[1].direction * depth,
              rect3d[2] + rays[2].direction * depth,
              rect3d[2] + rays[2].direction * depth,
              rect3d[3] + rays[3].direction * depth,
              rect3d[3] + rays[3].direction * depth,
              rect3d[0] + rays[0].direction * depth
            };

            if (m_dbgFrustum == nullptr)
            {
              m_dbgFrustum = std::shared_ptr<LineBatch>
              (
                new LineBatch(corners, X_AXIS, DrawType::Line)
              );
              m_ignoreList.push_back(m_dbgFrustum->GetIdVal());
              currScene->AddEntity(m_dbgFrustum.get());
            }
            else
            {
              m_dbgFrustum->Generate(corners, X_AXIS, DrawType::Line);
            }
          }
        }

        return StateType::StateEndPick;
      }

      if (signal == BaseMod::m_leftMouseBtnDragSgnl)
      {
        EditorViewport* vp = g_app->GetActiveViewport();
        if (vp != nullptr)
        {
          m_mouseData[1] = vp->GetLastMousePosScreenSpace();

          auto drawSelectionRectangleFn = [this](ImDrawList* drawList) -> void
          {
            Vec2 min, max;
            GetMouseRect(min, max);

            ImU32 col = ImColor(g_selectBoxWindowColor);
            drawList->AddRectFilled(min, max, col, 5.0f);

            col = ImColor(g_selectBoxBorderColor);
            drawList->AddRect(min, max, col, 5.0f, 15, 2.0f);
          };

          vp->m_drawCommands.push_back(drawSelectionRectangleFn);
        }
      }

      return StateType::Null;
    }

    void StateBeginBoxPick::GetMouseRect(Vec2& min, Vec2& max)
    {
      min = Vec2(TK_FLT_MAX);
      max = Vec2(-TK_FLT_MAX);

      for (int i = 0; i < 2; i++)
      {
        min = glm::min(min, m_mouseData[i]);
        max = glm::max(max, m_mouseData[i]);
      }
    }

    SignalId StateEndPick::Update(float deltaTime)
    {
      return NullSignal;
    }

    String StateEndPick::Signaled(SignalId signal)
    {
      return StateType::Null;
    }

    SignalId StateDeletePick::Update(float deltaTime)
    {
      Window::Type activeType = g_app->GetActiveWindow()->GetType();
      if  // Stop text edit deletes to remove entities.
        (
          activeType != Window::Type::Viewport
          && activeType != Window::Type::Viewport2d
          && activeType != Window::Type::Outliner
        )
      {
        return NullSignal;
      }

      // Gather the selection hierarchy.
      EntityRawPtrArray deleteList;
      g_app->GetCurrentScene()->GetSelectedEntities(deleteList);

      EntityRawPtrArray roots;
      GetRootEntities(deleteList, roots);

      deleteList.clear();
      for (Entity* e : roots)
      {
        // Gather hierarchy from parent to child.
        deleteList.push_back(e);
        GetChildren(e, deleteList);
      }

      // Revert to recover hierarchies.
      std::reverse(deleteList.begin(), deleteList.end());

      if (!deleteList.empty())
      {
        if (deleteList.size() > 1)
        {
          ActionManager::GetInstance()->BeginActionGroup();
        }

        for (Entity* e : deleteList)
        {
          ActionManager::GetInstance()->AddAction(new DeleteAction(e));
        }
        ActionManager::GetInstance()->GroupLastActions
        (
          static_cast<int>(deleteList.size())
        );
      }

      return NullSignal;
    }

    String StateDeletePick::Signaled(SignalId signal)
    {
      return StateType::Null;
    }

    void StateDuplicate::TransitionIn(State* prevState)
    {
      EntityRawPtrArray selecteds;
      EditorScenePtr currScene = g_app->GetCurrentScene();
      currScene->GetSelectedEntities(selecteds);
      if (!selecteds.empty())
      {
        currScene->ClearSelection();
        if (selecteds.size() > 1)
        {
          ActionManager::GetInstance()->BeginActionGroup();
        }

        EntityRawPtrArray selectedRoots;
        GetRootEntities(selecteds, selectedRoots);

        int cpyCount = 0;
        bool copy = ImGui::GetIO().KeyShift;
        for (Entity* ntt : selectedRoots)
        {
          EntityRawPtrArray copies;
          if (copy)
          {
            DeepCopy(ntt, copies);
          }
          else
          {
            DeepInstantiate(ntt, copies);
          }

          for (Entity* cpy : copies)
          {
            ActionManager::GetInstance()->AddAction(new CreateAction(cpy));
          }

          currScene->AddToSelection(copies.front()->GetIdVal(), true);
          cpyCount += static_cast<int>(copies.size());
        }

        // Status info
        g_app->m_statusMsg = std::to_string(cpyCount)
        + " entities are copied.";

        ActionManager::GetInstance()->GroupLastActions(cpyCount);
      }
    }

    void StateDuplicate::TransitionOut(State* nextState)
    {
    }

    SignalId StateDuplicate::Update(float deltaTime)
    {
      return NullSignal;
    }

    String StateDuplicate::Signaled(SignalId signal)
    {
      return StateType::Null;
    }

    // Mods
    //////////////////////////////////////////////////////////////////////////

    SelectMod::SelectMod()
      : BaseMod(ModId::Select)
    {
    }

    void SelectMod::Init()
    {
      StateBeginPick* initialState = new StateBeginPick();
      m_stateMachine->m_currentState = initialState;

      m_stateMachine->PushState(initialState);
      m_stateMachine->PushState(new StateBeginBoxPick());

      State* state = new StateEndPick();
      state->m_links[m_backToStart] = StateType::StateBeginPick;
      m_stateMachine->PushState(state);

      state = new StateDeletePick();
      state->m_links[m_backToStart] = StateType::StateBeginPick;
      m_stateMachine->PushState(state);
    }

    void SelectMod::Update(float deltaTime)
    {
      BaseMod::Update(deltaTime);

      if (m_stateMachine->m_currentState->GetType() == StateType::StateEndPick)
      {
        StateEndPick* endPick =
        static_cast<StateEndPick*>(m_stateMachine->m_currentState);
        EntityIdArray entities;
        endPick->PickDataToEntityId(entities);
        g_app->GetCurrentScene()->AddToSelection
        (
          entities,
          ImGui::GetIO().KeyShift
        );

        ModManager::GetInstance()->DispatchSignal(BaseMod::m_backToStart);
      }

      if
      (
        m_stateMachine->m_currentState->GetType() == StateType::StateDeletePick
      )
      {
        ModManager::GetInstance()->DispatchSignal(BaseMod::m_backToStart);
      }
    }

    CursorMod::CursorMod()
      : BaseMod(ModId::Cursor)
    {
    }

    void CursorMod::Init()
    {
      State* state = new StateBeginPick();
      m_stateMachine->m_currentState = state;
      m_stateMachine->PushState(state);

      state = new StateEndPick();
      state->m_links[BaseMod::m_backToStart] = StateType::StateBeginPick;
      m_stateMachine->PushState(state);
    }

    void CursorMod::Update(float deltaTime)
    {
      BaseMod::Update(deltaTime);

      if (m_stateMachine->m_currentState->GetType() == StateType::StateEndPick)
      {
        StateEndPick* endPick =
        static_cast<StateEndPick*>(m_stateMachine->m_currentState);
        EditorScene::PickData& pd = endPick->m_pickData.back();
        g_app->m_cursor->m_worldLocation = pd.pickPos;

        m_stateMachine->Signal(BaseMod::m_backToStart);
      }
    }
  }  // namespace Editor
}  // namespace ToolKit
