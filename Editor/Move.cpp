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
		
		StateGizmoBase::StateGizmoBase()
		{
			m_gizmo = nullptr;
			m_grabbedAxis = AxisLabel::None;
			m_mouseData.resize(2);
		}

		void StateGizmoBase::Update(float deltaTime)
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
					Mat4 ts = e->m_node->GetTransform();
					DecomposeMatrix(ts, m_gizmo->m_worldLocation, m_gizmo->m_node->m_orientation);
				}

				m_gizmo->Update(deltaTime);
			}
		}

		void StateGizmoBase::TransitionIn(State* prevState)
		{
		}

		void StateGizmoBase::TransitionOut(State* nextState)
		{
			StateGizmoBase* baseState = dynamic_cast<StateGizmoBase*> (nextState);
			if (baseState != nullptr)
			{
				baseState->m_mouseData = m_mouseData;
				baseState->m_gizmo = m_gizmo;
				baseState->m_grabbedAxis = m_grabbedAxis;
				baseState->m_intersectionPlane = m_intersectionPlane;
			}
		}

		void StateGizmoBase::MakeSureGizmoIsValid()
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

		ToolKit::Vec3 StateGizmoBase::GetGrabbedAxis(int n)
		{
			Vec3 axes[3];
			Entity* e = g_app->m_scene.GetCurrentSelection();
			ExtractAxes(e->m_node->GetTransform(), axes[0], axes[1], axes[2]);

			int first = (int)m_grabbedAxis % 3;
			if (n == 0)
			{
				return axes[first];
			}

			int second = (first + 1) % 3;
			return axes[second];
		}

		bool StateGizmoBase::IsPlaneMod()
		{
			return m_grabbedAxis > AxisLabel::Z;
		}

		// StateBeginMove
		//////////////////////////////////////////////////////////////////////////

		void StateGizmoBegin::TransitionOut(State* nextState)
		{
			StateGizmoBase::TransitionOut(nextState);

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

		void StateGizmoBegin::Update(float deltaTime)
		{
			StateGizmoBase::Update(deltaTime); // Update gizmo's loc & view.

			MakeSureGizmoIsValid();

			m_gizmo->Grab(m_grabbedAxis);
			Entity* e = g_app->m_scene.GetCurrentSelection();
			if (e != nullptr)
			{
				Vec3 x, y, z;
				Mat4 ts = e->m_node->GetTransform();
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
						m_gizmo->Lock(axisLabes[i]);
					}
				}
			}
		}

		String StateGizmoBegin::Signaled(SignalId signal)
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
					if (!m_gizmo->IsLocked(m_grabbedAxis))
					{
						CalculateIntersectionPlane();
						return StateType::StateMoveTo;
					}
				}
			}

			return StateType::Null;
		}

		void StateGizmoBegin::CalculateIntersectionPlane()
		{
			Entity* e = g_app->m_scene.GetCurrentSelection();
			
			Vec3 x, y, z;
			Mat4 ts = e->m_node->GetTransform();
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

			if (m_grabbedAxis <= AxisLabel::Z)
			{
				py = glm::normalize(glm::cross(px, dir));
				pz = glm::normalize(glm::cross(py, px));
				m_intersectionPlane = PlaneFrom(gizmOrg, pz);
			}
		}

		// StateMoveTo
		//////////////////////////////////////////////////////////////////////////

		void StateMoveTo::TransitionIn(State* prevState)
		{
			StateGizmoBase::TransitionIn(prevState);

			// Create guide line.
			if ((int)m_grabbedAxis < 3)
			{
				assert(m_grabbedAxis != AxisLabel::None);

				Vec3 p = m_gizmo->m_worldLocation;
				Vec3 color = g_gizmoColor[(int)m_grabbedAxis];
				Vec3 axis = GetGrabbedAxis(0);
				std::vector<Vec3> points
				{
					p + axis * 100.0f,
					p - axis * 100.0f
				};

				assert(m_guideLine == nullptr && "Expected to be nulled on TransitionOut.");
				m_guideLine = std::make_shared<LineBatch>(points, color, DrawType::Line, 1.0f);
				g_app->m_scene.AddEntity(m_guideLine.get());
			}
		}

		void StateMoveTo::TransitionOut(State* prevState)
		{
			StateGizmoBase::TransitionOut(prevState);

			if (m_guideLine != nullptr)
			{
				g_app->m_scene.RemoveEntity(m_guideLine->m_id);
				assert(m_guideLine.use_count() == 1 && "There must be single instance.");
				m_guideLine = nullptr;
			}
		}

		void StateMoveTo::Update(float deltaTime)
		{
			StateGizmoBase::Update(deltaTime);
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
				Vec3 g2p = p - m_gizmo->m_worldLocation;

				ray = vp->RayFromScreenSpacePoint(m_mouseData[0]);
				LinePlaneIntersection(ray, m_intersectionPlane, t);
				Vec3 p0 = PointOnRay(ray, t);

				Vec3 delta;
				if (IsPlaneMod())
				{
					delta = p - p0;
				}
				else
				{
					Vec3 movAxis = GetGrabbedAxis(0);
					Vec3 g2p0 = p0 - m_gizmo->m_worldLocation;
					float intsDst = glm::dot(movAxis, g2p0);
					float projDst = glm::dot(movAxis, g2p);

					delta = movAxis * (projDst - intsDst);
				}

				std::vector<Entity*> selecteds;
				g_app->m_scene.GetSelectedEntities(selecteds);

				for (Entity* e : selecteds)
				{
					e->m_node->Translate(delta);
					std::swap(m_mouseData[0], m_mouseData[1]);
				}
			}
			else
			{
				assert(false && "Intersection expected.");
			}
		}

		// StateEndMove
		//////////////////////////////////////////////////////////////////////////

		void StateGizmoEnd::TransitionOut(State* nextState)
		{
			if (nextState->ThisIsA<StateGizmoBegin>())
			{
				StateGizmoBegin* baseNext = static_cast<StateGizmoBegin*> (nextState);
				baseNext->m_grabbedAxis = AxisLabel::None;
				baseNext->m_mouseData[0] = Vec2();
				baseNext->m_mouseData[1] = Vec2();
			}
		}

		String StateGizmoEnd::Signaled(SignalId signal)
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
				StateGizmoBase* baseState = static_cast<StateGizmoBase*> (m_stateMachine->QueryState(StateType::StateBeginMove));
				assert(baseState && "Gizmo remains in the scene as dead pointer.");

				if (baseState != nullptr && baseState->m_gizmo != nullptr)
				{
					g_app->m_scene.RemoveEntity(baseState->m_gizmo->m_id);
				}
			}
		}

		void MoveMod::Init()
		{
			State* state = new StateGizmoBegin();
			m_stateMachine->m_currentState = state;

			m_stateMachine->PushState(state);
			m_stateMachine->PushState(new StateMoveTo());
			m_stateMachine->PushState(new StateGizmoEnd());

			m_stateMachine->PushState(new StateBeginPick());
			m_stateMachine->PushState(new StateBeginBoxPick());
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

			if (m_stateMachine->m_currentState->ThisIsA<StateGizmoEnd>())
			{
				ModManager::GetInstance()->DispatchSignal(BaseMod::m_backToStart);
			}
		}

	}
}