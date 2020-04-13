#include "stdafx.h"
#include "Move.h"
#include "Gizmo.h"
#include "GlobalDef.h"
#include "Node.h"
#include "DebugNew.h"
#include "Viewport.h"
#include "ConsoleWindow.h"
#include <Directional.h>

namespace ToolKit
{
	namespace Editor
	{
		// StateMoveBase
		//////////////////////////////////////////////////////////////////////////
		
		StateMoveBase::StateMoveBase()
		{
			m_gizmo = nullptr;
			m_grabbedAxis = AxisLabel::None;
			m_intersectDist = 0.0f;

			m_mouseData.resize(2);
		}

		void StateMoveBase::Update(float deltaTime)
		{
			if (m_gizmo != nullptr)
			{
				if (g_app->m_scene.GetSelectedEntityCount() == 0)
				{
					g_app->m_scene.RemoveEntity(m_gizmo->m_id);
					return;
				}

				Entity* e = g_app->m_scene.GetCurrentSelection();
				if (e != nullptr)
				{
					glm::mat4 ts = e->m_node->GetTransform();
					DecomposeMatrix(ts, m_gizmo->m_worldLocation, m_gizmo->m_node->m_orientation);
				}

				m_gizmo->Update(deltaTime);
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
				baseState->m_mouseData = m_mouseData;
				baseState->m_gizmo = m_gizmo;
				baseState->m_grabbedAxis = m_grabbedAxis;
				baseState->m_intersectionPlane = m_intersectionPlane;
				baseState->m_moveAxis = m_moveAxis;
				baseState->m_intersectDist = m_intersectDist;
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
				Entity* e = g_app->m_scene.GetCurrentSelection();
				if (e != nullptr)
				{
					g_app->m_scene.AddEntity(m_gizmo.get());
				}
			}
		}

		// StateBeginMove
		//////////////////////////////////////////////////////////////////////////

		void StateBeginMove::TransitionOut(State* nextState)
		{
			StateMoveBase::TransitionOut(nextState);

			if (nextState->ThisIsA<StateBeginPick>())
			{
				StateBeginPick* baseNext = static_cast<StateBeginPick*> (nextState);
				baseNext->m_mouseData = m_mouseData;

				if (m_gizmo != nullptr)
				{
					if (!baseNext->IsIgnored(m_gizmo->m_id))
					{
						baseNext->m_ignoreList.push_back(m_gizmo->m_id);
					}
				}
			}
		}

		void StateBeginMove::Update(float deltaTime)
		{
			StateMoveBase::Update(deltaTime); // Update gizmo's loc & view.

			MakeSureGizmoIsValid();

			m_gizmo->m_inAccessable = AxisLabel::None;
			m_gizmo->m_grabbed = m_grabbedAxis;
			Entity* e = g_app->m_scene.GetCurrentSelection();
			if (e != nullptr)
			{
				Vec3 x, y, z;
				glm::mat4 ts = e->m_node->GetTransform();
				ExtractAxes(ts, x, y, z);

				Viewport* vp = g_app->GetActiveViewport();
				Vec3 camOrg = vp->m_camera->m_node->GetTranslation(TransformationSpace::TS_WORLD);
				Vec3 gizmOrg = m_gizmo->m_node->GetTranslation(TransformationSpace::TS_WORLD);
				Vec3 dir = glm::normalize(camOrg - gizmOrg);

				float safetyMeasure = glm::abs(glm::cos(glm::radians(15.0f)));
				AxisLabel axisLabes[3] = { AxisLabel::X, AxisLabel::Y, AxisLabel::Z };
				Vec3 axes[3] = { x, y, z };

				for (int i = 0; i < 3; i++)
				{
					if (safetyMeasure < glm::abs(glm::dot(dir, axes[i])))
					{
						m_gizmo->m_inAccessable = axisLabes[i];
					}
				}
			}
		}

		String StateBeginMove::Signaled(SignalId signal)
		{
			if (signal == BaseMod::m_leftMouseBtnDownSgnl)
			{
				Viewport* vp = g_app->GetActiveViewport();
				if (vp != nullptr)
				{
					m_mouseData[0] = vp->GetLastMousePosScreenSpace();
					m_grabbedAxis = m_gizmo->HitTest(vp->RayFromMousePosition());
				}

				if (m_grabbedAxis == AxisLabel::None)
				{
					return StateType::StateBeginPick;
				}
			}

			if (signal == BaseMod::m_leftMouseBtnDragSgnl)
			{
				Entity* e = g_app->m_scene.GetCurrentSelection();
				if (e == nullptr)
				{
					return StateType::Null;
				}

				if (m_grabbedAxis != AxisLabel::None)
				{
					if (m_grabbedAxis != m_gizmo->m_inAccessable)
					{
						CalculateIntersectionPlane();
						return StateType::StateMoveTo;
					}
				}
			}

			return StateType::Null;
		}

		void StateBeginMove::CalculateIntersectionPlane()
		{
			Entity* e = g_app->m_scene.GetCurrentSelection();
			Vec3 x, y, z;
			glm::mat4 ts = e->m_node->GetTransform();
			ExtractAxes(ts, x, y, z);

			Viewport* vp = g_app->GetActiveViewport();
			Vec3 camOrg = vp->m_camera->m_node->GetTranslation(TransformationSpace::TS_WORLD);
			Vec3 gizmOrg = m_gizmo->m_worldLocation;
			Vec3 dir = glm::normalize(camOrg - gizmOrg);

			Vec3 px, py, pz;
			switch (m_grabbedAxis)
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
			}

			m_moveAxis = px;
			py = glm::normalize(glm::cross(px, dir));
			pz = glm::normalize(glm::cross(py, px));
			m_intersectionPlane = PlaneFrom(gizmOrg, pz);

			float t;
			Ray ray = vp->RayFromMousePosition();
			if (LinePlaneIntersection(ray, m_intersectionPlane, t))
			{
				Vec3 p = PointOnRay(ray, t);
				Vec3 go2p = p - gizmOrg;
				m_intersectDist = glm::dot(px, go2p);
			}
			else
			{
				assert(false && "Intersection expected.");
			}
		}

		// StateMoveTo
		//////////////////////////////////////////////////////////////////////////

		void StateMoveTo::TransitionIn(State* prevState)
		{
			StateMoveBase::TransitionIn(prevState);

			// Create guide line.
			if ((int)m_grabbedAxis < 3)
			{
				assert(m_grabbedAxis != AxisLabel::None && "{0, 1, 2} expected.");

				Vec3 p = m_gizmo->m_worldLocation;
				Vec3 color = g_gizmoColor[(int)m_grabbedAxis];
				std::vector<Vec3> points
				{
					p + m_moveAxis * 100.0f,
					p - m_moveAxis * 100.0f
				};

				assert(m_activeAxis == nullptr && "Expected to be nulled on TransitionOut.");
				m_activeAxis = std::make_shared<LineBatch>(points, color, DrawType::Line, 1.0f);
				g_app->m_scene.AddEntity(m_activeAxis.get());
			}
		}

		void StateMoveTo::TransitionOut(State* prevState)
		{
			StateMoveBase::TransitionOut(prevState);

			g_app->m_scene.RemoveEntity(m_activeAxis->m_id);
			assert(m_activeAxis.use_count() == 1 && "There must be single instance.");
			m_activeAxis = nullptr;
		}

		void StateMoveTo::Update(float deltaTime)
		{
			StateMoveBase::Update(deltaTime);
		}

		String StateMoveTo::Signaled(SignalId signal)
		{		
			if (signal == BaseMod::m_leftMouseBtnDragSgnl)
			{
				Move();
			}

			if (signal == BaseMod::m_leftMouseBtnUpSgnl)
			{
				return StateType::StateEndMove;
			}

			return StateType::Null;
		}

		void StateMoveTo::Move()
		{
			Viewport* vp = g_app->GetActiveViewport();
			m_mouseData[1] = vp->GetLastMousePosScreenSpace();

			float t;
			Ray ray = vp->RayFromMousePosition();
			if (LinePlaneIntersection(ray, m_intersectionPlane, t))
			{
				Vec3 p = PointOnRay(ray, t);
				Vec3 go2p = p - m_gizmo->m_worldLocation;
				
				float projDst = glm::dot(m_moveAxis, go2p);
				Vec3 delta = m_moveAxis * (projDst - m_intersectDist);

				std::vector<Entity*> selecteds;
				g_app->m_scene.GetSelectedEntities(selecteds);

				for (Entity* e : selecteds)
				{
					e->m_node->Translate(delta);
				}
			}
			else
			{
				assert(false && "Intersection expected.");
			}
		}

		// StateEndMove
		//////////////////////////////////////////////////////////////////////////

		void StateEndMove::TransitionOut(State* nextState)
		{
			if (nextState->ThisIsA<StateBeginMove>())
			{
				StateBeginMove* baseNext = static_cast<StateBeginMove*> (nextState);
				baseNext->m_grabbedAxis = AxisLabel::None;
				baseNext->m_mouseData[0] = glm::vec2();
				baseNext->m_mouseData[1] = glm::vec2();
				baseNext->m_intersectDist = 0.0f;
			}
		}

		String StateEndMove::Signaled(SignalId signal)
		{
			if (signal == BaseMod::m_backToStart)
			{
				return StateType::StateBeginMove;
			}

			return StateType::Null;
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
			m_stateMachine->PushState(new StateMoveTo());
			m_stateMachine->PushState(new StateEndMove());

			m_stateMachine->PushState(new StateBeginPick());
			// m_stateMachine->PushState(new StateBeginBoxPick());
			state = new StateEndPick();
			state->m_links[m_linkToMoveBeginSgnl] = StateType::StateBeginMove;
			m_stateMachine->PushState(state);
		}

		void MoveMod::Update(float deltaTime)
		{
			BaseMod::Update(deltaTime);

			if (m_stateMachine->m_currentState->ThisIsA<StateEndPick>())
			{
				StateEndPick* endPick = static_cast<StateEndPick*> (m_stateMachine->m_currentState);
				
				std::vector<EntityId> entities;
				endPick->PickDataToEntityId(entities);
				g_app->m_scene.AddToSelection(entities, ImGui::GetIO().KeyShift);

				ModManager::GetInstance()->DispatchSignal(m_linkToMoveBeginSgnl);
			}

			if (m_stateMachine->m_currentState->ThisIsA<StateEndMove>())
			{
				ModManager::GetInstance()->DispatchSignal(BaseMod::m_backToStart);
			}
		}

	}
}