#include "AnchorMod.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "Camera.h"
#include "ConsoleWindow.h"
#include "DebugNew.h"
#include "EditorViewport.h"
#include "Gizmo.h"
#include "GlobalDef.h"
#include "Node.h"
#include "Util.h"

namespace ToolKit
{
  namespace Editor
  {

    // StateMoveBase
    //////////////////////////////////////////////////////////////////////////

    StateAnchorBase::StateAnchorBase()
    {
      m_anchor = nullptr;
      m_type = TransformType::Translate;

      m_mouseData.resize(2);
    }

    SignalId StateAnchorBase::Update(float deltaTime)
    {
      EditorScenePtr currScene = g_app->GetCurrentScene();
      if (currScene->GetSelectedEntityCount() == 0)
      {
        g_app->m_anchor = nullptr;
        return NullSignal;
      }

      if (Entity* e = currScene->GetCurrentSelection())
      {
        if (e->IsSurfaceInstance())
        {
          Surface* surface = static_cast<Surface*>(e);
          if (Node* parentNode = surface->m_node->m_parent)
          {
            if (parentNode->m_entity->GetType()
              == EntityType::Entity_CanvasPanel)
            {
              m_anchor->m_entity = surface;
              CanvasPanel* canvasPanel =
                static_cast<CanvasPanel*>(parentNode->m_entity);
              m_anchor->m_worldLocation = canvasPanel->m_node->GetTranslation(
                TransformationSpace::TS_WORLD);
              for (int i = 0; i < 4; i++)
              {
                m_anchor->m_anchorRatios[i] = surface->m_anchorRatios[i];
              }
            }
          }
        }
      }

      m_anchor->Update(deltaTime);
      return NullSignal;
    }

    void StateAnchorBase::TransitionIn(State* prevState) { }

    void StateAnchorBase::TransitionOut(State* nextState)
    {
      StateAnchorBase* baseState = dynamic_cast<StateAnchorBase*>(nextState);

      if (baseState != nullptr)
      {
        baseState->m_anchor = m_anchor;
        baseState->m_mouseData = m_mouseData;
        baseState->m_intersectionPlane = m_intersectionPlane;
        baseState->m_type = m_type;
      }
    }

    void StateAnchorBase::MakeSureAnchorIsValid()
    {
      if (g_app->m_anchor == nullptr)
      {
        if (g_app->GetCurrentScene()->GetCurrentSelection() != nullptr)
        {
          g_app->m_anchor = m_anchor;
        }
      }
    }

    // StateAnchorBegin
    //////////////////////////////////////////////////////////////////////////

    void StateAnchorBegin::TransitionIn(State* prevState)
    {
      StateAnchorBase::TransitionIn(prevState);
    }

    void StateAnchorBegin::TransitionOut(State* nextState)
    {
      StateAnchorBase::TransitionOut(nextState);

      if (nextState->ThisIsA<StateBeginPick>())
      {
        StateBeginPick* baseNext = static_cast<StateBeginPick*>(nextState);
        baseNext->m_mouseData = m_mouseData;

        if (!baseNext->IsIgnored(m_anchor->GetIdVal()))
        {
          baseNext->m_ignoreList.push_back(m_anchor->GetIdVal());
        }
      }
    }

    SignalId StateAnchorBegin::Update(float deltaTime)
    {
      StateAnchorBase::Update(deltaTime);

      MakeSureAnchorIsValid();

      if (g_app->GetCurrentScene()->GetCurrentSelection() != nullptr)
      {
        if (EditorViewport* vp = g_app->GetActiveViewport())
        {
          const DirectionLabel axis =
            m_anchor->HitTest(vp->RayFromMousePosition());
          if (axis != DirectionLabel::None)
          {
            m_anchor->m_lastHovered = axis;
          }
        }
        else
          return NullSignal;
      }

      return NullSignal;
    }

    String StateAnchorBegin::Signaled(SignalId signal)
    {
      if (signal == BaseMod::m_leftMouseBtnDownSgnl)
      {
        if (EditorViewport* vp = g_app->GetActiveViewport())
        {
          m_mouseData[0] = vp->GetLastMousePosScreenSpace();
          DirectionLabel axis = m_anchor->HitTest(vp->RayFromMousePosition());
          m_anchor->Grab(axis);
        }

        if (m_anchor->IsGrabbed(DirectionLabel::None)
          || g_app->GetCurrentScene()->GetCurrentSelection() == nullptr)
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
        m_anchor->Grab(DirectionLabel::None);
        m_anchor->m_grabPoint = ZERO;
      }

      if (signal == BaseMod::m_leftMouseBtnDragSgnl)
      {
        if (g_app->GetCurrentScene()->GetCurrentSelection() == nullptr)
        {
          return StateType::Null;
        }

        if (!m_anchor->IsGrabbed(DirectionLabel::None))
        {
          return StateType::StateAnchorTo;
        }
      }

      return StateType::Null;
    }

    String StateAnchorBegin::GetType() { return StateType::StateAnchorBegin; }

    void StateAnchorBegin::CalculateIntersectionPlane()
    {
      if (EditorViewport* vp = g_app->GetActiveViewport())
      {
        const Vec3 camOrg = vp->GetCamera()->m_node->GetTranslation(
          TransformationSpace::TS_WORLD);
        const Vec3 anchorOrg = m_anchor->m_worldLocation;
        const Vec3 dir = glm::normalize(camOrg - anchorOrg);
        m_intersectionPlane = PlaneFrom(anchorOrg, Vec3 { 0, 0, 1 });
      }
    }

    void StateAnchorBegin::CalculateGrabPoint()
    {
      m_anchor->m_grabPoint = ZERO;

      if (EditorViewport* vp = g_app->GetActiveViewport())
      {
        float t;
        Ray ray = vp->RayFromMousePosition();
        if (LinePlaneIntersection(ray, m_intersectionPlane, t))
        {
          m_anchor->m_grabPoint = PointOnRay(ray, t);
        }
      }
    }

    // AnchorAction
    //////////////////////////////////////////////////////////////////////////

    AnchorAction::AnchorAction(Entity* ntt)
    {
      m_entity = ntt;
      m_transform = ntt->m_node->GetTransform(TransformationSpace::TS_WORLD);
    }

    AnchorAction::~AnchorAction() { }

    void AnchorAction::Undo() { Swap(); }

    void AnchorAction::Redo() { Swap(); }

    void AnchorAction::Swap()
    {
      const Mat4 backUp =
        m_entity->m_node->GetTransform(TransformationSpace::TS_WORLD);
      m_entity->m_node->SetTransform(
        m_transform, TransformationSpace::TS_WORLD, false);
      m_transform = backUp;
    }

    // StateAnchorTo
    //////////////////////////////////////////////////////////////////////////

    void StateAnchorTo::TransitionIn(State* prevState)
    {
      StateAnchorBase::TransitionIn(prevState);

      EntityRawPtrArray entities, selecteds;
      EditorScenePtr currScene = g_app->GetCurrentScene();
      currScene->GetSelectedEntities(selecteds);
      GetRootEntities(selecteds, entities);
      if (!entities.empty())
      {
        if (entities.size() > 1)
        {
          ActionManager::GetInstance()->BeginActionGroup();
        }

        int actionEntityCount = 0;
        for (Entity* ntt : entities)
        {
          if (ntt->GetTransformLockVal())
          {
            continue;
          }

          actionEntityCount++;
          ActionManager::GetInstance()->AddAction(new AnchorAction(ntt));
        }
        ActionManager::GetInstance()->GroupLastActions(actionEntityCount);
      }

      m_delta = ZERO;
      m_deltaAccum = ZERO;
      m_initialLoc = currScene->GetCurrentSelection()->m_node->GetTranslation(
        TransformationSpace::TS_WORLD);
      SDL_GetGlobalMouseState(&m_mouseInitialLoc.x, &m_mouseInitialLoc.y);
    }

    void StateAnchorTo::TransitionOut(State* prevState)
    {
      StateAnchorBase::TransitionOut(prevState);
      m_anchor->m_grabPoint = ZERO;

      // Set the mouse position roughly.
      SDL_WarpMouseGlobal(
        static_cast<int>(m_mouseData[1].x), static_cast<int>(m_mouseData[1].y));
    }

    SignalId StateAnchorTo::Update(float deltaTime)
    {
      Transform(m_delta);
      StateAnchorBase::Update(deltaTime);
      ImGui::SetMouseCursor(ImGuiMouseCursor_None);
      if (EditorViewport* vp = g_app->GetActiveViewport())
      {
        Vec2 contentMin, contentMax;
        vp->GetContentAreaScreenCoordinates(&contentMin, &contentMax);

        auto drawMoveCursorFn = [this, contentMin, contentMax](
                                  ImDrawList* drawList) -> void
        {
          // Clamp the mouse pos.
          Vec2 pos = m_mouseData[1];
          pos = glm::clamp(pos, contentMin, contentMax);

          // Draw cursor.
          Vec2 size(28.0f);
          drawList->AddImage(Convert2ImGuiTexture(UI::m_moveIcn),
            pos - size * 0.5f,
            pos + size * 0.5f);
        };

        vp->m_drawCommands.push_back(drawMoveCursorFn);
      }
      return NullSignal;
    }

    String StateAnchorTo::Signaled(SignalId signal)
    {
      if (signal == BaseMod::m_leftMouseBtnDragSgnl)
      {
        CalculateDelta();
      }

      if (signal == BaseMod::m_leftMouseBtnUpSgnl)
      {
        return StateType::StateAnchorEnd;
      }

      return StateType::Null;
    }

    String StateAnchorTo::GetType() { return StateType::StateAnchorTo; }

    void StateAnchorTo::CalculateDelta()
    {
      IVec2 mouseLoc;
      SDL_GetGlobalMouseState(&mouseLoc.x, &mouseLoc.y);
      m_mouseData[1] = m_mouseData[0] + Vec2(mouseLoc - m_mouseInitialLoc);

      SDL_WarpMouseGlobal(m_mouseInitialLoc.x, m_mouseInitialLoc.y);

      m_delta = Vec3(m_mouseData[1] - m_mouseData[0], 0);

      std::swap(m_mouseData[0], m_mouseData[1]);
    }

    void StateAnchorTo::Transform(const Vec3& delta)
    {
      EntityRawPtrArray roots;
      EditorScenePtr currScene = g_app->GetCurrentScene();
      currScene->GetSelectedEntities(roots);

      Entity* e = currScene->GetCurrentSelection();

      Translate(e);
    }

    void StateAnchorTo::Translate(Entity* ntt)
    {
      const DirectionLabel direction = m_anchor->GetGrabbedDirection();
      const bool hasXDirection =
        (direction != DirectionLabel::N && direction != DirectionLabel::S)
        || (direction == DirectionLabel::CENTER);
      const bool hasYDirection =
        (direction != DirectionLabel::E && direction != DirectionLabel::W)
        || (direction == DirectionLabel::CENTER);

      Vec3 deltaX, deltaY;

      if (hasXDirection)
      {
        Vec3 dir { 1.f, 0.f, 0.f };
        dir = glm::normalize(dir);
        deltaX += glm::dot(dir, m_delta) * dir;
      }

      if (hasYDirection)
      {
        Vec3 dir { 0.f, 1.f, 0.f };
        dir = glm::normalize(dir);
        deltaY += glm::dot(dir, m_delta) * dir;
      }

      if (ntt->IsSurfaceInstance()
        && ntt->GetType() != EntityType::Entity_CanvasPanel)
      {
        Surface* surface = static_cast<Surface*>(ntt);

        float w = 0, h = 0;

        if (Entity* parent = surface->m_node->m_parent->m_entity)
        {
          if (parent->GetType() == EntityType::Entity_CanvasPanel)
          {
            CanvasPanel* canvasPanel = static_cast<CanvasPanel*>(parent);
            w = canvasPanel->GetSizeVal().x;
            h = canvasPanel->GetSizeVal().y;
          }
        }

        if (direction == DirectionLabel::CENTER)
        {
          const float dX = m_delta.x / w;
          const float dY = m_delta.y / h;

          surface->m_anchorRatios[0] += std::min(1.f, dX);
          surface->m_anchorRatios[0] =
            std::max(0.f, surface->m_anchorRatios[0]);
          surface->m_anchorRatios[0] =
            std::min(1.f, surface->m_anchorRatios[0]);

          surface->m_anchorRatios[1] = 1.f - surface->m_anchorRatios[0];

          surface->m_anchorRatios[2] += std::min(1.f, dY);
          surface->m_anchorRatios[2] =
            std::max(0.f, surface->m_anchorRatios[2]);
          surface->m_anchorRatios[2] =
            std::min(1.f, surface->m_anchorRatios[2]);

          surface->m_anchorRatios[3] = 1.f - surface->m_anchorRatios[2];
        }

        if (direction == DirectionLabel::W || direction == DirectionLabel::NW
          || direction == DirectionLabel::SW)
        {
          const float d = m_delta.x / w;
          surface->m_anchorRatios[0] += std::min(1.f, d);
          surface->m_anchorRatios[0] =
            std::max(0.f, surface->m_anchorRatios[0]);
          surface->m_anchorRatios[0] =
            std::min(1.f, surface->m_anchorRatios[0]);

          if ((surface->m_anchorRatios[0] + surface->m_anchorRatios[1]) > 1.f)
          {
            surface->m_anchorRatios[0] = 1.f - surface->m_anchorRatios[1];
          }
        }
        if (direction == DirectionLabel::E || direction == DirectionLabel::NE
          || direction == DirectionLabel::SE)
        {
          const float d = m_delta.x / w;
          surface->m_anchorRatios[1] -= std::min(1.f, d);

          surface->m_anchorRatios[1] =
            std::max(0.f, surface->m_anchorRatios[1]);
          surface->m_anchorRatios[1] =
            std::min(1.f, surface->m_anchorRatios[1]);

          if ((surface->m_anchorRatios[1] + surface->m_anchorRatios[0]) > 1.f)
          {
            surface->m_anchorRatios[1] = 1.f - surface->m_anchorRatios[0];
          }
        }
        if (direction == DirectionLabel::N || direction == DirectionLabel::NW
          || direction == DirectionLabel::NE)
        {
          const float d = m_delta.y / h;
          surface->m_anchorRatios[2] += std::min(1.f, d);

          surface->m_anchorRatios[2] =
            std::max(0.f, surface->m_anchorRatios[2]);
          surface->m_anchorRatios[2] =
            std::min(1.f, surface->m_anchorRatios[2]);

          if ((surface->m_anchorRatios[2] + surface->m_anchorRatios[3]) > 1.f)
          {
            surface->m_anchorRatios[2] = 1.f - surface->m_anchorRatios[3];
          }
        }
        if (direction == DirectionLabel::S || direction == DirectionLabel::SW
          || direction == DirectionLabel::SE)
        {
          const float d = m_delta.y / h;
          surface->m_anchorRatios[3] -= std::min(1.f, d);

          surface->m_anchorRatios[3] =
            std::max(0.f, surface->m_anchorRatios[3]);
          surface->m_anchorRatios[3] =
            std::min(1.f, surface->m_anchorRatios[3]);

          if ((surface->m_anchorRatios[3] + surface->m_anchorRatios[2]) > 1.f)
          {
            surface->m_anchorRatios[3] = 1.f - surface->m_anchorRatios[2];
          }
        }
      }
    }

    // StateAnchorEnd
    //////////////////////////////////////////////////////////////////////////

    void StateAnchorEnd::TransitionOut(State* nextState)
    {
      if (nextState->ThisIsA<StateAnchorBegin>())
      {
        StateAnchorBegin* baseNext = static_cast<StateAnchorBegin*>(nextState);

        baseNext->m_anchor->Grab(DirectionLabel::None);
        baseNext->m_mouseData[0] = Vec2();
        baseNext->m_mouseData[1] = Vec2();
      }
    }

    SignalId StateAnchorEnd::Update(float deltaTime)
    {
      StateAnchorBase::Update(deltaTime);
      return NullSignal;
    }

    String StateAnchorEnd::Signaled(SignalId signal)
    {
      if (signal == BaseMod::m_backToStart)
      {
        return StateType::StateAnchorBegin;
      }

      return StateType::Null;
    }

    String StateAnchorEnd::GetType() { return StateType::StateAnchorEnd; }

    // MoveMod
    //////////////////////////////////////////////////////////////////////////

    AnchorMod::AnchorMod(ModId id)
      : BaseMod(id)
    {
    }

    AnchorMod::~AnchorMod() { g_app->m_anchor = nullptr; }

    void AnchorMod::Init()
    {
      State* state = new StateAnchorBegin();
      StateAnchorBase* baseState = static_cast<StateAnchorBase*>(state);
      switch (m_id)
      {
        case ModId::Anchor:
          m_anchor = std::make_shared<Anchor>(
            Billboard::Settings { false, 6.0f, 60.0f });
          baseState->m_type = StateAnchorBase::TransformType::Translate;
          break;
        default:
          assert(false);
          return;
      }

      baseState->m_anchor = m_anchor;
      m_stateMachine->m_currentState = state;

      m_stateMachine->PushState(state);
      m_stateMachine->PushState(new StateAnchorTo());
      m_stateMachine->PushState(new StateAnchorEnd());

      state = new StateBeginPick();
      state->m_links[m_backToStart] = StateType::StateAnchorBegin;
      m_stateMachine->PushState(state);
      state = new StateEndPick();
      state->m_links[m_backToStart] = StateType::StateAnchorBegin;
      m_stateMachine->PushState(state);

      m_prevTransformSpace = g_app->m_transformSpace;
    }

    void AnchorMod::UnInit() { }

    void AnchorMod::Update(float deltaTime)
    {
      BaseMod::Update(deltaTime);

      if (m_stateMachine->m_currentState->ThisIsA<StateEndPick>())
      {
        StateEndPick* endPick =
          static_cast<StateEndPick*>(m_stateMachine->m_currentState);

        EntityIdArray entities;
        endPick->PickDataToEntityId(entities);
        g_app->GetCurrentScene()->AddToSelection(
          entities, ImGui::GetIO().KeyShift);

        ModManager::GetInstance()->DispatchSignal(m_backToStart);
      }

      if (m_stateMachine->m_currentState->ThisIsA<StateAnchorEnd>())
      {
        ModManager::GetInstance()->DispatchSignal(BaseMod::m_backToStart);
      }
    }

  }  // namespace Editor
}  // namespace ToolKit
