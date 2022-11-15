#include "AnchorMod.h"

#include "App.h"
#include "Camera.h"
#include "ConsoleWindow.h"
#include "EditorViewport.h"
#include "Gizmo.h"
#include "Global.h"
#include "Node.h"
#include "Util.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    // StateMoveBase
    //////////////////////////////////////////////////////////////////////////

    StateAnchorBase::StateAnchorBase()
    {
      m_anchor = nullptr;
      m_type   = TransformType::Translate;

      m_mouseData.resize(2);
    }

    SignalId StateAnchorBase::Update(float deltaTime)
    {
      m_signalConsumed = !m_anchor->IsGrabbed(DirectionLabel::None);

      MakeSureAnchorIsValid();
      m_anchor->Update(deltaTime);
      return NullSignal;
    }

    void StateAnchorBase::TransitionIn(State* prevState)
    {
      m_anchorDeltaTransform = ZERO;
    }

    void StateAnchorBase::TransitionOut(State* nextState)
    {
      StateAnchorBase* baseState = dynamic_cast<StateAnchorBase*>(nextState);

      if (baseState != nullptr)
      {
        baseState->m_anchor            = m_anchor;
        baseState->m_mouseData         = m_mouseData;
        baseState->m_intersectionPlane = m_intersectionPlane;
        baseState->m_type              = m_type;
      }
    }

    void StateAnchorBase::MakeSureAnchorIsValid()
    {
      g_app->m_anchor = nullptr;

      EditorScenePtr currScene = g_app->GetCurrentScene();
      if (currScene->GetSelectedEntityCount() == 0)
      {
        m_anchor->m_entity = nullptr;
        return;
      }

      if (Entity* e = currScene->GetCurrentSelection())
      {
        if (e->IsSurfaceInstance())
        {
          Surface* surface = static_cast<Surface*>(e);
          if (Node* parentNode = surface->m_node->m_parent)
          {
            if (parentNode->m_entity->GetType() == EntityType::Entity_Canvas)
            {
              m_anchor->m_entity  = surface;
              Canvas* canvasPanel = static_cast<Canvas*>(parentNode->m_entity);
              m_anchor->m_worldLocation = canvasPanel->m_node->GetTranslation(
                  TransformationSpace::TS_WORLD);

              g_app->m_anchor = m_anchor;
            }
          }
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
        baseNext->m_mouseData    = m_mouseData;

        if (!baseNext->IsIgnored(m_anchor->GetIdVal()))
        {
          baseNext->m_ignoreList.push_back(m_anchor->GetIdVal());
        }
      }
    }

    SignalId StateAnchorBegin::Update(float deltaTime)
    {
      StateAnchorBase::Update(deltaTime);

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
      }

      ReflectAnchorTransform(m_anchor->m_entity);

      return NullSignal;
    }

    String StateAnchorBegin::Signaled(SignalId signal)
    {
      if (signal == BaseMod::m_leftMouseBtnDownSgnl)
      {
        if (EditorViewport* vp = g_app->GetActiveViewport())
        {
          m_mouseData[0]      = vp->GetLastMousePosScreenSpace();
          DirectionLabel axis = m_anchor->HitTest(vp->RayFromMousePosition());
          m_anchor->Grab(axis);
        }

        if (!m_anchor->IsGrabbed(DirectionLabel::None) &&
            g_app->GetCurrentScene()->GetCurrentSelection() != nullptr)
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

    String StateAnchorBegin::GetType()
    {
      return StateType::StateAnchorBegin;
    }

    void StateAnchorBegin::CalculateIntersectionPlane()
    {
      if (EditorViewport* vp = g_app->GetActiveViewport())
      {
        const Vec3 camOrg = vp->GetCamera()->m_node->GetTranslation(
            TransformationSpace::TS_WORLD);
        const Vec3 anchorOrg = m_anchor->m_worldLocation;
        const Vec3 dir       = glm::normalize(camOrg - anchorOrg);
        m_intersectionPlane  = PlaneFrom(anchorOrg, Vec3{0, 0, 1});
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
      m_entity    = ntt;
      m_transform = ntt->m_node->GetTransform(TransformationSpace::TS_WORLD);
    }

    AnchorAction::~AnchorAction()
    {
    }

    void AnchorAction::Undo()
    {
      Swap();
    }

    void AnchorAction::Redo()
    {
      Swap();
    }

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

      m_anchorDeltaTransform = ZERO;
      m_deltaAccum           = ZERO;
      m_initialLoc = currScene->GetCurrentSelection()->m_node->GetTranslation(
          TransformationSpace::TS_WORLD);
      SDL_GetGlobalMouseState(&m_mouseInitialLoc.x, &m_mouseInitialLoc.y);
    }

    void StateAnchorTo::TransitionOut(State* prevState)
    {
      StateAnchorBase::TransitionOut(prevState);
      m_anchor->m_grabPoint = ZERO;

      // Set the mouse position roughly.
      SDL_WarpMouseGlobal(static_cast<int>(m_mouseData[1].x),
                          static_cast<int>(m_mouseData[1].y));
    }

    SignalId StateAnchorTo::Update(float deltaTime)
    {
      Transform(m_anchorDeltaTransform);
      StateAnchorBase::Update(deltaTime);
      ImGui::SetMouseCursor(ImGuiMouseCursor_None);
      if (EditorViewport* vp = g_app->GetActiveViewport())
      {
        Vec2 contentMin, contentMax;
        vp->GetContentAreaScreenCoordinates(&contentMin, &contentMax);

        auto drawMoveCursorFn =
            [this, contentMin, contentMax](ImDrawList* drawList) -> void {
          // Clamp the mouse pos.
          Vec2 pos = m_mouseData[1];
          pos      = glm::clamp(pos, contentMin, contentMax);

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

    String StateAnchorTo::GetType()
    {
      return StateType::StateAnchorTo;
    }

    void StateAnchorTo::CalculateDelta()
    {
      IVec2 mouseLoc;
      SDL_GetGlobalMouseState(&mouseLoc.x, &mouseLoc.y);
      m_mouseData[1] = m_mouseData[0] + Vec2(mouseLoc - m_mouseInitialLoc);

      SDL_WarpMouseGlobal(m_mouseInitialLoc.x, m_mouseInitialLoc.y);

      float t;
      EditorViewport* vp = g_app->GetActiveViewport();
      Ray ray            = vp->RayFromScreenSpacePoint(m_mouseData[1]);
      if (LinePlaneIntersection(ray, m_intersectionPlane, t))
      {
        Vec3 p = PointOnRay(ray, t);

        Canvas* canvasPanel = static_cast<Canvas*>(
            m_anchor->m_entity->m_node->m_parent->m_entity);

        ray = vp->RayFromScreenSpacePoint(m_mouseData[0]);
        LinePlaneIntersection(ray, m_intersectionPlane, t);
        Vec3 p0                = PointOnRay(ray, t);
        m_anchorDeltaTransform = p - p0;
      }
      else
      {
        assert(false && "Intersection expected.");
        m_anchorDeltaTransform = ZERO;
      }

      std::swap(m_mouseData[0], m_mouseData[1]);
    }

    void StateAnchorTo::Transform(const Vec3& delta)
    {
      EntityRawPtrArray roots;
      EditorScenePtr currScene = g_app->GetCurrentScene();
      currScene->GetSelectedEntities(roots);

      Entity* e = currScene->GetCurrentSelection();

      ReflectAnchorTransform(e);
    }

    void StateAnchorBase::ReflectAnchorTransform(Entity* ntt)
    {
      if (m_anchor == nullptr || ntt == nullptr || !ntt->IsSurfaceInstance() ||
          !ntt->m_node->m_parent || !ntt->m_node->m_parent->m_entity ||
          ntt->m_node->m_parent->m_entity->GetType() !=
              EntityType::Entity_Canvas)
        return;

      Surface* surface = static_cast<Surface*>(ntt);

      const DirectionLabel direction = m_anchor->GetGrabbedDirection();
      const bool hasXDirection =
          (direction != DirectionLabel::N && direction != DirectionLabel::S) ||
          (direction == DirectionLabel::CENTER);
      const bool hasYDirection =
          (direction != DirectionLabel::E && direction != DirectionLabel::W) ||
          (direction == DirectionLabel::CENTER);

      Vec3 deltaX, deltaY;

      m_deltaAccum += m_anchorDeltaTransform;
      m_anchorDeltaTransform = ZERO;

      if (g_app->m_snapsEnabled)
      {
        float spacing = g_app->m_moveDelta;
        for (uint i = 0; i < 2; i++)
        {
          if (abs(m_deltaAccum[i]) > spacing)
          {
            m_anchorDeltaTransform[i] =
                glm::round(m_deltaAccum[i] / spacing) * spacing;
            m_deltaAccum[i] = 0.0f;
          }
        }
      }

      if (hasXDirection)
      {
        Vec3 dir{1.f, 0.f, 0.f};
        dir = glm::normalize(dir);
        deltaX += glm::dot(dir, m_anchorDeltaTransform) * dir;
      }

      if (hasYDirection)
      {
        Vec3 dir{0.f, 1.f, 0.f};
        dir = glm::normalize(dir);
        deltaY += glm::dot(dir, m_anchorDeltaTransform) * dir;
      }

      GetLogger()->WriteConsole(
          LogType::Warning,
          "DeltaTransform: %f, %f & DeltaAcum: %f, %f & Spacing",
          m_anchorDeltaTransform.x,
          m_anchorDeltaTransform.y,
          m_deltaAccum.x,
          m_deltaAccum.y,
          g_app->m_moveDelta);
      float w = 0, h = 0;

      if (Entity* parent = surface->m_node->m_parent->m_entity)
      {
        if (parent->GetType() == EntityType::Entity_Canvas)
        {
          Canvas* canvasPanel  = static_cast<Canvas*>(parent);
          const BoundingBox bb = canvasPanel->GetAABB(true);
          w                    = bb.GetWidth();
          h                    = bb.GetHeight();
        }
      }

      float* anchorRatios = surface->m_anchorParams.m_anchorRatios;

      if (direction == DirectionLabel::CENTER)
      {
        const float dX = m_anchorDeltaTransform.x / w;
        const float dY = m_anchorDeltaTransform.y / h;

        anchorRatios[0] += std::min(1.f, dX);
        anchorRatios[0] = std::max(0.f, anchorRatios[0]);
        anchorRatios[0] = std::min(1.f, anchorRatios[0]);

        anchorRatios[1] = 1.f - anchorRatios[0];

        anchorRatios[2] -= std::min(1.f, dY);
        anchorRatios[2] = std::max(0.f, anchorRatios[2]);
        anchorRatios[2] = std::min(1.f, anchorRatios[2]);

        anchorRatios[3] = 1.f - anchorRatios[2];
      }

      if (direction == DirectionLabel::W || direction == DirectionLabel::NW ||
          direction == DirectionLabel::SW)
      {
        const float d = m_anchorDeltaTransform.x / w;
        anchorRatios[0] += std::min(1.f, d);
        anchorRatios[0] = std::max(0.f, anchorRatios[0]);
        anchorRatios[0] = std::min(1.f, anchorRatios[0]);

        if ((anchorRatios[0] + anchorRatios[1]) > 1.f)
        {
          anchorRatios[0] = 1.f - anchorRatios[1];
        }
      }
      if (direction == DirectionLabel::E || direction == DirectionLabel::NE ||
          direction == DirectionLabel::SE)
      {
        const float d = m_anchorDeltaTransform.x / w;
        anchorRatios[1] -= std::min(1.f, d);

        anchorRatios[1] = std::max(0.f, anchorRatios[1]);
        anchorRatios[1] = std::min(1.f, anchorRatios[1]);

        if ((anchorRatios[1] + anchorRatios[0]) > 1.f)
        {
          anchorRatios[1] = 1.f - anchorRatios[0];
        }
      }
      if (direction == DirectionLabel::N || direction == DirectionLabel::NW ||
          direction == DirectionLabel::NE)
      {
        const float d = m_anchorDeltaTransform.y / h;
        anchorRatios[2] -= std::min(1.f, d);

        anchorRatios[2] = std::max(0.f, anchorRatios[2]);
        anchorRatios[2] = std::min(1.f, anchorRatios[2]);

        if ((anchorRatios[2] + anchorRatios[3]) > 1.f)
        {
          anchorRatios[2] = 1.f - anchorRatios[3];
        }
      }
      if (direction == DirectionLabel::S || direction == DirectionLabel::SW ||
          direction == DirectionLabel::SE)
      {
        const float d = m_anchorDeltaTransform.y / h;
        anchorRatios[3] += std::min(1.f, d);

        anchorRatios[3] = std::max(0.f, anchorRatios[3]);
        anchorRatios[3] = std::min(1.f, anchorRatios[3]);

        if ((anchorRatios[3] + anchorRatios[2]) > 1.f)
        {
          anchorRatios[3] = 1.f - anchorRatios[2];
        }
      }

      Vec3 canvasPoints[4], surfacePoints[4];
      surface->CalculateAnchorOffsets(canvasPoints, surfacePoints);

      surface->m_anchorParams.m_offsets[2] =
          surfacePoints[0].x - canvasPoints[0].x;
      surface->m_anchorParams.m_offsets[3] =
          canvasPoints[1].x - surfacePoints[1].x;

      surface->m_anchorParams.m_offsets[0] =
          canvasPoints[0].y - surfacePoints[0].y;
      surface->m_anchorParams.m_offsets[1] =
          surfacePoints[2].y - canvasPoints[2].y;

      m_anchorDeltaTransform = ZERO; // Consume the delta.
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

    String StateAnchorEnd::GetType()
    {
      return StateType::StateAnchorEnd;
    }

    // MoveMod
    //////////////////////////////////////////////////////////////////////////

    AnchorMod::AnchorMod(ModId id) : BaseMod(id)
    {
    }

    AnchorMod::~AnchorMod()
    {
      g_app->m_anchor = nullptr;
    }

    void AnchorMod::Init()
    {
      State* state               = new StateAnchorBegin();
      StateAnchorBase* baseState = static_cast<StateAnchorBase*>(state);
      switch (m_id)
      {
      case ModId::Anchor:
        m_anchor =
            std::make_shared<Anchor>(Billboard::Settings{false, 6.0f, 60.0f});
        baseState->m_type = StateAnchorBase::TransformType::Translate;
        break;
      default:
        assert(false);
        return;
      }

      baseState->m_anchor            = m_anchor;
      m_stateMachine->m_currentState = state;

      m_stateMachine->PushState(state);
      m_stateMachine->PushState(new StateAnchorTo());
      m_stateMachine->PushState(new StateAnchorEnd());

      m_prevTransformSpace = g_app->m_transformSpace;
    }

    void AnchorMod::UnInit()
    {
    }

    void AnchorMod::Update(float deltaTime)
    {
      BaseMod::Update(deltaTime);

      if (m_stateMachine->m_currentState->ThisIsA<StateAnchorEnd>())
      {
        m_stateMachine->Signal(BaseMod::m_backToStart);
      }
    }

  } // namespace Editor
} // namespace ToolKit
