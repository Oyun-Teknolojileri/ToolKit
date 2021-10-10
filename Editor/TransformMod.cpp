#include "stdafx.h"
#include "TransformMod.h"
#include "Gizmo.h"
#include "GlobalDef.h"
#include "Node.h"
#include "EditorViewport.h"
#include "ConsoleWindow.h"
#include "Directional.h"
#include "Util.h"
#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {
    // StateMoveBase
    //////////////////////////////////////////////////////////////////////////

    StateTransformBase::StateTransformBase()
    {
      m_gizmo = nullptr;
      m_type = TransformType::Translate;

      m_mouseData.resize(2);
    }

    void StateTransformBase::Update(float deltaTime)
    {
      if (g_app->m_scene->GetSelectedEntityCount() == 0)
      {
        g_app->m_gizmo = nullptr;
        return;
      }

      Entity* e = g_app->m_scene->GetCurrentSelection();
      if (e != nullptr)
      {
        // Get world location as gizmo origin.
        m_gizmo->m_worldLocation = e->m_node->GetTranslation(TransformationSpace::TS_WORLD);
        m_gizmo->m_normalVectors = e->m_node->GetTransformAxes(g_app->m_transformSpace);
      }

      m_gizmo->Update(deltaTime);
    }

    void StateTransformBase::TransitionIn(State* prevState)
    {
    }

    void StateTransformBase::TransitionOut(State* nextState)
    {
      StateTransformBase* baseState = dynamic_cast<StateTransformBase*> (nextState);
      if (baseState != nullptr)
      {
        baseState->m_gizmo = m_gizmo;
        baseState->m_mouseData = m_mouseData;
        baseState->m_intersectionPlane = m_intersectionPlane;
        baseState->m_type = m_type;
      }
    }

    void StateTransformBase::MakeSureGizmoIsValid()
    {
      if (g_app->m_gizmo == nullptr)
      {
        Entity* e = g_app->m_scene->GetCurrentSelection();
        if (e != nullptr)
        {
          g_app->m_gizmo = m_gizmo;
        }
      }
    }

    Vec3 StateTransformBase::GetGrabbedAxis(int n)
    {
      Vec3 axes[3];
      ExtractAxes(m_gizmo->m_normalVectors, axes[0], axes[1], axes[2]);

      int first = (int)m_gizmo->GetGrabbedAxis() % 3;
      if (n == 0)
      {
        return axes[first];
      }

      int second = (first + 1) % 3;
      return axes[second];
    }

    bool StateTransformBase::IsPlaneMod()
    {
      return m_gizmo->GetGrabbedAxis() > AxisLabel::Z;
    }

    // StateTransformBegin
    //////////////////////////////////////////////////////////////////////////

    void StateTransformBegin::TransitionIn(State* prevState)
    {
      StateTransformBase::TransitionIn(prevState);
    }

    void StateTransformBegin::TransitionOut(State* nextState)
    {
      StateTransformBase::TransitionOut(nextState);

      if (nextState->ThisIsA<StateBeginPick>())
      {
        StateBeginPick* baseNext = static_cast<StateBeginPick*> (nextState);
        baseNext->m_mouseData = m_mouseData;

        if (!baseNext->IsIgnored(m_gizmo->m_id))
        {
          baseNext->m_ignoreList.push_back(m_gizmo->m_id);
        }
      }
    }

    void StateTransformBegin::Update(float deltaTime)
    {
      StateTransformBase::Update(deltaTime); // Update gizmo's loc & view.

      MakeSureGizmoIsValid();

      Entity* e = g_app->m_scene->GetCurrentSelection();
      if (e != nullptr)
      {
        EditorViewport* vp = g_app->GetActiveViewport();
        if (vp == nullptr)
        {
          return; // Console commands may put the process here whit out active viewport.
        }

        Vec3 camOrg = vp->m_camera->m_node->GetTranslation(TransformationSpace::TS_WORLD);
        Vec3 gizmOrg = m_gizmo->m_node->GetTranslation(TransformationSpace::TS_WORLD);
        Vec3 dir = glm::normalize(camOrg - gizmOrg);

        float safetyMeasure = glm::abs(glm::cos(glm::radians(5.0f)));
        AxisLabel axisLabes[3] = { AxisLabel::X, AxisLabel::Y, AxisLabel::Z };

        Vec3 axes[3];
        ExtractAxes(m_gizmo->m_normalVectors, axes[0], axes[1], axes[2]);

        if (m_type != TransformType::Rotate)
        {
          for (int i = 0; i < 3; i++)
          {
            if (safetyMeasure < glm::abs(glm::dot(dir, axes[i])))
            {
              m_gizmo->Lock(axisLabes[i]);
            }
            else
            {
              m_gizmo->UnLock(axisLabes[i]);
            }
          }
        }

        // Highlight on mouse over.
        AxisLabel axis = m_gizmo->HitTest(vp->RayFromMousePosition());
        if (axis != AxisLabel::None)
        {
          if (!m_gizmo->IsLocked(axis))
          {
            m_gizmo->m_lastHovered = axis;
          }
        }
      }
    }

    String StateTransformBegin::Signaled(SignalId signal)
    {
      if (signal == BaseMod::m_leftMouseBtnDownSgnl)
      {
        EditorViewport* vp = g_app->GetActiveViewport();
        if (vp != nullptr)
        {
          m_mouseData[0] = vp->GetLastMousePosScreenSpace();
          AxisLabel axis = m_gizmo->HitTest(vp->RayFromMousePosition());
          if (!m_gizmo->IsLocked(axis))
          {
            m_gizmo->Grab(axis);
          }
        }

        if (m_gizmo->IsGrabbed(AxisLabel::None))
        {
          return StateType::StateBeginPick;
        }
        else
        {
          CalculateIntersectionPlane();
          CalculateGrabPoint();
        }
      }

      if (signal == BaseMod::m_leftMouseBtnUpSgnl)
      {
        m_gizmo->Grab(AxisLabel::None);
        m_gizmo->m_grabPoint = Vec3();
      }

      if (signal == BaseMod::m_leftMouseBtnDragSgnl)
      {
        Entity* e = g_app->m_scene->GetCurrentSelection();
        if (e == nullptr)
        {
          return StateType::Null;
        }

        if (!m_gizmo->IsGrabbed(AxisLabel::None))
        {

          return StateType::StateTransformTo;
        }
      }

      if (signal == BaseMod::m_delete)
      {
        return StateType::StateDeletePick;
      }

      if (signal == BaseMod::m_duplicate)
      {
        return StateType::StateDuplicate;
      }

      return StateType::Null;
    }

    void StateTransformBegin::CalculateIntersectionPlane()
    {
      if (PolarGizmo* pg = dynamic_cast<PolarGizmo*> (m_gizmo))
      {
        // Polar intersection plane.
        if ((int)m_gizmo->GetGrabbedAxis() < 3)
        {
          assert(m_gizmo->GetGrabbedAxis() != AxisLabel::None);

          Vec3 p = m_gizmo->m_worldLocation;
          Vec3 axis = GetGrabbedAxis(0);

          if (PolarGizmo* pg = dynamic_cast<PolarGizmo*> (m_gizmo))
          {
            if (EditorViewport* vp = g_app->GetActiveViewport())
            {
              float t;
              PlaneEquation axisPlane = PlaneFrom(p, axis);
              Ray ray = vp->RayFromMousePosition();
              if (LinePlaneIntersection(ray, axisPlane, t))
              {
                Vec3 intersectPnt = PointOnRay(ray, t);
                intersectPnt = glm::normalize(intersectPnt - p);
                pg->m_grabPoint = intersectPnt;

                m_intersectionPlane = PlaneFrom(intersectPnt + m_gizmo->m_worldLocation, axis);
              }
            }
          }
          else
          {
            m_gizmo->m_grabPoint = axis;
          }
        }
      }
      else
      {
        // Linear intersection plane.
        EditorViewport* vp = g_app->GetActiveViewport();
        Vec3 camOrg = vp->m_camera->m_node->GetTranslation(TransformationSpace::TS_WORLD);
        Vec3 gizmOrg = m_gizmo->m_worldLocation;
        Vec3 dir = glm::normalize(camOrg - gizmOrg);

        Vec3 x, px, y, py, z, pz;
        ExtractAxes(m_gizmo->m_normalVectors, x, y, z);
        switch (m_gizmo->GetGrabbedAxis())
        {
        case AxisLabel::X:
          px = x;
          break;
        case AxisLabel::Y:
          px = y;
          break;
        case AxisLabel::Z:
          px = z;
          break;
        case AxisLabel::XY:
          m_intersectionPlane = PlaneFrom(gizmOrg, z);
          break;
        case AxisLabel::YZ:
          m_intersectionPlane = PlaneFrom(gizmOrg, x);
          break;
        case AxisLabel::ZX:
          m_intersectionPlane = PlaneFrom(gizmOrg, y);
          break;
        default:
          assert(false);
        }

        if (m_gizmo->GetGrabbedAxis() <= AxisLabel::Z)
        {
          py = glm::normalize(glm::cross(px, dir));
          pz = glm::normalize(glm::cross(py, px));
          m_intersectionPlane = PlaneFrom(gizmOrg, pz);
        }
      }
    }

    void StateTransformBegin::CalculateGrabPoint()
    {
      assert(m_gizmo->GetGrabbedAxis() != AxisLabel::None);
      m_gizmo->m_grabPoint = Vec3();

      if (EditorViewport* vp = g_app->GetActiveViewport())
      {
        float t;
        Ray ray = vp->RayFromMousePosition();
        if (LinePlaneIntersection(ray, m_intersectionPlane, t))
        {
          m_gizmo->m_grabPoint = PointOnRay(ray, t);
        }
      }
    }

    // TransformAction
    //////////////////////////////////////////////////////////////////////////

    TransformAction::TransformAction(Entity* ntt)
    {
      m_entity = ntt;
      m_transform = ntt->m_node->GetTransform(TransformationSpace::TS_WORLD);
    }

    TransformAction::~TransformAction()
    {

    }

    void TransformAction::Undo()
    {
      Swap();
    }

    void TransformAction::Redo()
    {
      Swap();
    }

    void TransformAction::Swap()
    {
      Mat4 backUp = m_entity->m_node->GetTransform(TransformationSpace::TS_WORLD);
      m_entity->m_node->SetTransform(m_transform, TransformationSpace::TS_WORLD, false);
      m_transform = backUp;
    }

    // StateTransformTo
    //////////////////////////////////////////////////////////////////////////

    void StateTransformTo::TransitionIn(State* prevState)
    {
      StateTransformBase::TransitionIn(prevState);

      EntityRawPtrArray entities, selecteds;
      g_app->m_scene->GetSelectedEntities(selecteds);
      GetRootEntities(selecteds, entities);
      if (!entities.empty())
      {
        if (entities.size() > 1)
        {
          ActionManager::GetInstance()->BeginActionGroup();
        }

        for (Entity* ntt : entities)
        {
          ActionManager::GetInstance()->AddAction(new TransformAction(ntt));
        }
        ActionManager::GetInstance()->GroupLastActions((int)entities.size());
      }

      m_delta = Vec3(0.0f);
    }

    void StateTransformTo::TransitionOut(State* prevState)
    {
      StateTransformBase::TransitionOut(prevState);
      m_gizmo->m_grabPoint = Vec3();
    }

    void StateTransformTo::Update(float deltaTime)
    {
      Transform(m_delta);
      StateTransformBase::Update(deltaTime);
    }

    String StateTransformTo::Signaled(SignalId signal)
    {
      if (signal == BaseMod::m_leftMouseBtnDragSgnl)
      {
        CalculateDelta();
      }

      if (signal == BaseMod::m_leftMouseBtnUpSgnl)
      {
        return StateType::StateTransformEnd;
      }

      return StateType::Null;
    }

    void StateTransformTo::CalculateDelta()
    {
      EditorViewport* vp = g_app->GetActiveViewport();
      m_mouseData[1] = vp->GetLastMousePosScreenSpace();

      float t;
      Ray ray = vp->RayFromMousePosition();
      if (LinePlaneIntersection(ray, m_intersectionPlane, t))
      {
        // This point.
        Vec3 p = PointOnRay(ray, t);

        // Previous. point.
        ray = vp->RayFromScreenSpacePoint(m_mouseData[0]);
        LinePlaneIntersection(ray, m_intersectionPlane, t);
        Vec3 p0 = PointOnRay(ray, t);

        static float deltaAccum = 0.0f;
        if (PolarGizmo* pg = dynamic_cast<PolarGizmo*> (m_gizmo))
        {
          int axisInd = (int)m_gizmo->GetGrabbedAxis();
          Vec3 projAxis = pg->m_handles[axisInd]->m_tangentDir;
          float delta = glm::dot(projAxis, p - p0);
          if (g_app->m_snapsEnabled)
          {
            deltaAccum += glm::abs(delta);
            if (deltaAccum > glm::radians(g_app->m_rotateDelta))
            {
              delta = glm::radians(g_app->m_rotateDelta) * glm::sign(delta);
              deltaAccum = 0.0f;
            }
            else
            {
              delta = 0.0f;
            }
          }
          m_delta = AXIS[axisInd] * delta;
        }
        else
        {
          if (IsPlaneMod())
          {
            m_delta = p - p0;
          }
          else
          {
            Vec3 projAxis = GetGrabbedAxis(0);
            Vec3 g2p = p - m_gizmo->m_worldLocation;
            Vec3 g2p0 = p0 - m_gizmo->m_worldLocation;
            float intsDst = glm::dot(projAxis, g2p0);
            float projDst = glm::dot(projAxis, g2p);
            float delta = projDst - intsDst;
            if (g_app->m_snapsEnabled || g_app->m_snapToGrid)
            {
              deltaAccum += glm::abs(delta);
              float barrier = g_app->m_moveDelta;
              if (g_app->m_snapToGrid)
              {
                barrier = glm::max(barrier, 0.25f);
              }

              if (deltaAccum > barrier)
              {
                switch (m_type)
                {
                case TransformType::Translate:
                  delta = g_app->m_moveDelta * glm::sign(delta);
                  break;
                case TransformType::Scale:
                  delta = g_app->m_scaleDelta * glm::sign(delta);
                  break;
                default:
                  assert(false);
                }
                deltaAccum = 0.0f;
              }
              else
              {
                delta = 0.0f;
              }
            }

            if (glm::isnan(deltaAccum))
            {
              assert(false && "Nan is not expected.");
              deltaAccum = 0.0f;
            }

            Vec3 moveAxis = AXIS[(int)m_gizmo->GetGrabbedAxis()];
            m_delta = moveAxis * delta;
          }
        }
      }
      else
      {
        assert(false && "Intersection expected.");
      }

      std::swap(m_mouseData[0], m_mouseData[1]);
    }

    void StateTransformTo::Transform(const Vec3& delta)
    {
      EntityRawPtrArray roots;
      g_app->m_scene->GetSelectedEntities(roots);

      Entity* e = g_app->m_scene->GetCurrentSelection();
      NodePtrArray parents;

      // Make all selecteds child of current & store their original parents.
      //if (roots.size() > 1)
      {
        for (Entity* ntt : roots)
        {
          parents.push_back(ntt->m_node->m_parent);
          ntt->m_node->OrphanSelf(true);
        }

        for (Entity* ntt : roots)
        {
          if (ntt != e)
          {
            e->m_node->AddChild(ntt->m_node, true);
          }
        }
      }

      TransformationSpace space = g_app->m_transformSpace;
      switch (m_type)
      {
      case TransformType::Translate:
      {
        if (g_app->m_snapToGrid)
        {
          const float gridMinSpace = 0.25f;
          Vec3 currPos = e->m_node->GetTranslation(TransformationSpace::TS_WORLD);
          Vec3 targetPos = currPos + delta;

          int x = (int)(targetPos.x / gridMinSpace);
          targetPos.x = x * gridMinSpace;

          int z = (int)(targetPos.z / gridMinSpace);
          targetPos.z = z * gridMinSpace;

          Vec3 delAdjst = targetPos - currPos;
          e->m_node->Translate(delAdjst, space);
        }
        else
        {
          e->m_node->Translate(delta, space);
        }
      }
      break;
      case TransformType::Rotate:
      {
        float angle = glm::length(delta);
        if (glm::equal(angle, 0.0f))
        {
          break;
        }

        Vec3 axis = delta / angle;
        Quaternion rotation = glm::angleAxis(angle, axis);
        e->m_node->Rotate(rotation, space);
      }
      break;
      case TransformType::Scale:
      {
        Vec3 scale = e->m_node->GetScale();
        e->m_node->SetScale(scale + delta);
      }
      break;
      }

      // Set original parents back.
      for (int i = 0; i < (int)roots.size(); i++)
      {
        roots[i]->m_node->OrphanSelf(true);
        Node* parent = parents[i];
        if (parent != nullptr)
        {
          parent->AddChild(roots[i]->m_node, true);
        }
      }
    }

    // StateTransformEnd
    //////////////////////////////////////////////////////////////////////////

    void StateTransformEnd::TransitionOut(State* nextState)
    {
      if (nextState->ThisIsA<StateTransformBegin>())
      {
        StateTransformBegin* baseNext = static_cast<StateTransformBegin*> (nextState);
        baseNext->m_gizmo->Grab(AxisLabel::None);
        baseNext->m_mouseData[0] = Vec2();
        baseNext->m_mouseData[1] = Vec2();
      }
    }

    void StateTransformEnd::Update(float deltaTime)
    {
      StateTransformBase::Update(deltaTime);
    }

    String StateTransformEnd::Signaled(SignalId signal)
    {
      if (signal == BaseMod::m_backToStart)
      {
        return StateType::StateTransformBegin;
      }

      return StateType::Null;
    }

    // MoveMod
    //////////////////////////////////////////////////////////////////////////

    TransformMod::TransformMod(ModId id)
      : BaseMod(id)
    {
      m_gizmo = nullptr;
    }

    TransformMod::~TransformMod()
    {
      g_app->m_gizmo = nullptr;
      SafeDel(m_gizmo);
    }

    void TransformMod::Init()
    {
      State* state = new StateTransformBegin();
      StateTransformBase* baseState = static_cast<StateTransformBase*> (state);
      switch (m_id)
      {
      case ModId::Move:
        m_gizmo = new MoveGizmo();
        baseState->m_type = StateTransformBase::TransformType::Translate;
        break;
      case ModId::Rotate:
        m_gizmo = new PolarGizmo();
        baseState->m_type = StateTransformBase::TransformType::Rotate;
        break;
      case ModId::Scale:
        m_gizmo = new ScaleGizmo();
        baseState->m_type = StateTransformBase::TransformType::Scale;
        break;
      default:
        assert(false);
        return;
      }

      baseState->m_gizmo = m_gizmo;
      m_stateMachine->m_currentState = state;

      m_stateMachine->PushState(state);
      m_stateMachine->PushState(new StateTransformTo());
      m_stateMachine->PushState(new StateTransformEnd());

      state = new StateBeginPick();
      state->m_links[m_backToStart] = StateType::StateTransformBegin;
      m_stateMachine->PushState(state);
      state = new StateEndPick();
      state->m_links[m_backToStart] = StateType::StateTransformBegin;
      m_stateMachine->PushState(state);

      state = new StateDeletePick();
      state->m_links[m_backToStart] = StateType::StateTransformBegin;
      m_stateMachine->PushState(state);

      state = new StateDuplicate();
      state->m_links[m_backToStart] = StateType::StateTransformBegin;
      m_stateMachine->PushState(state);

      m_prevTransformSpace = g_app->m_transformSpace;
      if (m_id == ModId::Scale)
      {
        g_app->m_transformSpace = TransformationSpace::TS_LOCAL;
      }
    }

    void TransformMod::UnInit()
    {
      if (m_id == ModId::Scale)
      {
        g_app->m_transformSpace = m_prevTransformSpace;
      }
    }

    void TransformMod::Update(float deltaTime)
    {
      BaseMod::Update(deltaTime);

      if (m_stateMachine->m_currentState->ThisIsA<StateEndPick>())
      {
        StateEndPick* endPick = static_cast<StateEndPick*> (m_stateMachine->m_currentState);

        EntityIdArray entities;
        endPick->PickDataToEntityId(entities);
        g_app->m_scene->AddToSelection(entities, ImGui::GetIO().KeyShift);

        ModManager::GetInstance()->DispatchSignal(m_backToStart);
      }

      if (m_stateMachine->m_currentState->ThisIsA<StateTransformEnd>())
      {
        ModManager::GetInstance()->DispatchSignal(BaseMod::m_backToStart);
      }

      if (m_stateMachine->m_currentState->ThisIsA<StateDeletePick>())
      {
        ModManager::GetInstance()->DispatchSignal(BaseMod::m_backToStart);
      }

      if (m_stateMachine->m_currentState->ThisIsA<StateDuplicate>())
      {
        ModManager::GetInstance()->DispatchSignal(BaseMod::m_backToStart);
      }
    }

  }
}