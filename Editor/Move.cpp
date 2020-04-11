#include "stdafx.h"
#include "Move.h"
#include "Gizmo.h"
#include "GlobalDef.h"
#include "Node.h"
#include "DebugNew.h"
#include "Viewport.h"
#include "ConsoleWindow.h"

namespace ToolKit
{
	namespace Editor
	{
		// StateMoveBase
		//////////////////////////////////////////////////////////////////////////
		
		StateMoveBase::StateMoveBase()
		{
			m_gizmo = nullptr;
			m_grabbedAxis = MoveGizmo::Axis::None;

			m_mouseData.resize(2);
		}

		void StateMoveBase::Update(float deltaTime)
		{
			if (m_gizmo != nullptr)
			{
				m_gizmo->Update(deltaTime);
				Entity* e = g_app->m_scene.GetCurrentSelection();
				if (e != nullptr)
				{
					glm::mat4 ts = e->m_node->GetTransform();
					DecomposeMatrix(ts, m_gizmo->m_worldLocation, m_gizmo->m_node->m_orientation);
				}
			}
		}

		void StateMoveBase::TransitionIn(State* prevState)
		{
		}

		void StateMoveBase::TransitionOut(State* nextState)
		{
			StateMoveBase* baseState = dynamic_cast<StateMoveBase*> (nextState);
			if (baseState != nullptr)
			{
				baseState->m_gizmo = m_gizmo;
				baseState->m_grabbedAxis = m_grabbedAxis;
				baseState->m_mouseData = m_mouseData;
			}
		}

		void StateMoveBase::MakeSureGizmoIsValid()
		{
			if (m_gizmo == nullptr)
			{
				m_gizmo = std::make_shared<MoveGizmo>();
			}

			if (g_app->m_scene.GetEntity(m_gizmo->m_id) == nullptr)
			{
				g_app->m_scene.AddEntity(m_gizmo.get());
			}
		}

		void StateBeginMove::TransitionIn(State* prevState)
		{
			StateMoveBase::TransitionIn(prevState);
		}

		void StateBeginMove::TransitionOut(State* nextState)
		{
			StateMoveBase::TransitionOut(nextState);

			if (nextState->ThisIsA<StateBeginPick>())
			{
				StateBeginPick* baseNext = static_cast<StateBeginPick*> (nextState);
				baseNext->m_mouseData = m_mouseData;
			}
		}

		// StateBeginMove
		//////////////////////////////////////////////////////////////////////////

		void StateBeginMove::Update(float deltaTime)
		{
			StateMoveBase::Update(deltaTime);

			if (g_app->m_scene.GetSelectedEntityCount() == 0)
			{
				if (m_gizmo != nullptr)
				{
					g_app->m_scene.RemoveEntity(m_gizmo->m_id);
				}
			}
		}

		std::string StateBeginMove::Signaled(SignalId signal)
		{
			MakeSureGizmoIsValid();

			if (signal == BaseMod::m_leftMouseBtnDownSgnl)
			{
				Viewport* vp = g_app->GetActiveViewport();
				if (vp != nullptr)
				{
					m_mouseData[0] = vp->GetLastMousePosScreenSpace();
					m_grabbedAxis = m_gizmo->HitTest(vp->RayFromMousePosition());
				}

				if (m_grabbedAxis == MoveGizmo::Axis::None)
				{
					return StateType::StateBeginPick;
				}
			}

			if (signal == BaseMod::m_leftMouseBtnDragSgnl)
			{
				if (m_grabbedAxis != MoveGizmo::Axis::None)
				{
					return StateType::StateMoveTo;
				}
			}

			return StateType::Null;
		}

		// StateMoveTo
		//////////////////////////////////////////////////////////////////////////

		void StateMoveTo::TransitionIn(State* prevState)
		{
		}

		void StateMoveTo::TransitionOut(State* nextState)
		{
		}

		void StateMoveTo::Update(float deltaTime)
		{
			StateMoveBase::Update(deltaTime);
		}

		std::string StateMoveTo::Signaled(SignalId signal)
		{		
			if (signal == BaseMod::m_leftMouseBtnDragSgnl)
			{
				Move();
				return StateType::Null;
			}

			if (signal == BaseMod::m_leftMouseBtnUpSgnl)
			{
				return StateType::StateEndMove;
			}

			return StateType::Null;
		}

		void StateMoveTo::Move()
		{
			// Move on active selection.
			Entity* e = g_app->m_scene.GetCurrentSelection();

			// Construct plane parameters. Dragging will be on intersection plane's x
			Viewport* vp = g_app->GetActiveViewport();
			Ray grabRay = vp->RayFromScreenSpacePoint(m_mouseData[0]);
			Ray deltaRay = vp->RayFromScreenSpacePoint(m_mouseData[1]);
			m_mouseData[0] = m_mouseData[1]; // This drag, becomes next grab for delta move.

			glm::mat4 ts = e->m_node->GetTransform(TransformationSpace::TS_WORLD);
			glm::vec3 x, y, z, origin;
			origin = glm::column(ts, 3);
			ExtractAxes(ts, x, y, z);

			switch (m_grabbedAxis)
			{
			case MoveGizmo::Axis::X:
				break;
			case MoveGizmo::Axis::Y:
				std::swap(x, y);
				break;
			case MoveGizmo::Axis::Z:
				std::swap(x, z);
				break;
			case MoveGizmo::Axis::None:
			case MoveGizmo::Axis::XY:
			case MoveGizmo::Axis::XZ:
			case MoveGizmo::Axis::YZ:
			default:
				break;
			}

			glm::vec3 checkDir = glm::normalize(glm::cross(x, grabRay.direction));
			float safetyAngl = glm::abs(glm::dot(checkDir, x));
			if (glm::degrees(safetyAngl) < 5.0f)
			{
				ConsoleWindow* cns = g_app->GetConsole();
				if (cns != nullptr)
				{
					cns->AddLog("Can not grab axis, angle is not safe.", ConsoleWindow::LogType::Warning);
				}
			}

			if (glm::abs(glm::dot(checkDir, z)) > glm::abs(glm::dot(checkDir, y)))
			{
				std::swap(y, z);
			}

			PlaneEquation intersectionPlane = PlaneFrom(origin, z);

			bool hit = false;
			float tGrab, tDelta;
			if (RayPlaneIntersection(grabRay, intersectionPlane, tGrab))
			{
				if (RayPlaneIntersection(deltaRay, intersectionPlane, tDelta))
				{
					hit = true;

					glm::vec3 p1 = PointOnRay(grabRay, tGrab);
					glm::vec3 p2 = PointOnRay(deltaRay, tDelta);

					glm::vec3 delta = p2 - p1;
					delta = glm::dot(x, delta) * x;

					// Move current selection.
					std::vector<Entity*> selecteds;
					g_app->m_scene.GetSelectedEntities(selecteds);
					for (Entity* e : selecteds)
					{
						e->m_node->Translate(delta, TransformationSpace::TS_LOCAL);
					}
				}
			}

			// assert(hit && "Ray-Plane intersection is expected.");
		}

		// MoveMod
		//////////////////////////////////////////////////////////////////////////

		// Signal definitions.
		SignalId MoveMod::m_linkToMoveBeginSgnl = BaseMod::GetNextSignalId();

		MoveMod::~MoveMod()
		{
			if (m_stateMachine->m_currentState != nullptr)
			{
				StateMoveBase* baseState = static_cast<StateMoveBase*> (m_stateMachine->QueryState(StateType::StateBeginMove));
				assert(baseState && "Gizmo remains in the scene as dead pointer.");

				if (baseState != nullptr && baseState->m_gizmo != nullptr)
				{
					g_app->m_scene.RemoveEntity(baseState->m_gizmo->m_id);
				}
			}
		}

		void MoveMod::Init()
		{
			State* state = new StateBeginMove();
			m_stateMachine->m_currentState = state;

			m_stateMachine->PushState(state);
			m_stateMachine->PushState(new StateBeginPick());
			m_stateMachine->PushState(new StateBeginBoxPick());

			m_stateMachine->PushState(new StateMoveTo());
			state = new StateEndPick();
			state->m_links[m_linkToMoveBeginSgnl] = StateType::StateBeginMove;
			m_stateMachine->PushState(state);
		}

		void MoveMod::Update(float deltaTime)
		{
			BaseMod::Update(deltaTime);

			if (m_stateMachine->m_currentState->GetType() == StateType::StateEndPick)
			{
				StateEndPick* endPick = static_cast<StateEndPick*> (m_stateMachine->m_currentState);
				std::vector<EntityId> entities;
				endPick->PickDataToEntityId(entities);
				g_app->m_scene.AddToSelection(entities, ImGui::GetIO().KeyShift);

				ModManager::GetInstance()->DispatchSignal(m_linkToMoveBeginSgnl);
			}
		}

	}
}