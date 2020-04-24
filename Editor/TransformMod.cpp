#include "stdafx.h"
#include "TransformMod.h"
#include "Gizmo.h"
#include "GlobalDef.h"
#include "Node.h"
#include "Viewport.h"
#include "ConsoleWindow.h"
#include "Directional.h"
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
			m_mouseData.resize(2);
		}

		void StateTransformBase::Update(float deltaTime)
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
					// Get world location as gizmo origin.
					m_gizmo->m_worldLocation = e->m_node->GetTranslation(TransformationSpace::TS_WORLD);
					
					// Get transform orientation.
					Quaternion orientation;
					switch (g_app->m_transformOrientation)
					{
					case TransformationSpace::TS_WORLD:
						break;
					case TransformationSpace::TS_PARENT:
						if (e->m_node->m_parent != nullptr)
						{
							orientation = e->m_node->m_parent->GetOrientation(TransformationSpace::TS_WORLD);
						}
						break;
					case TransformationSpace::TS_LOCAL:
						orientation = e->m_node->GetOrientation(TransformationSpace::TS_WORLD);
					default:
						break;
					}

					m_axisOrientation = orientation;
					m_gizmo->m_node->SetOrientation(orientation, TransformationSpace::TS_WORLD);
				}

				m_gizmo->Update(deltaTime);
			}
		}

		void StateTransformBase::TransitionIn(State* prevState)
		{
		}

		void StateTransformBase::TransitionOut(State* nextState)
		{
			StateTransformBase* baseState = dynamic_cast<StateTransformBase*> (nextState);
			if (baseState != nullptr)
			{
				baseState->m_mouseData = m_mouseData;
				baseState->m_gizmo = m_gizmo;
				baseState->m_intersectionPlane = m_intersectionPlane;
			}
		}

		void StateTransformBase::MakeSureGizmoIsValid()
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

		ToolKit::Vec3 StateTransformBase::GetGrabbedAxis(int n)
		{
			Vec3 axes[3];
			ExtractAxes(glm::toMat4(m_axisOrientation), axes[0], axes[1], axes[2]);

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

				if (m_gizmo != nullptr)
				{
					if (!baseNext->IsIgnored(m_gizmo->m_id))
					{
						baseNext->m_ignoreList.push_back(m_gizmo->m_id);
					}
				}
			}
		}

		void StateTransformBegin::Update(float deltaTime)
		{
			StateTransformBase::Update(deltaTime); // Update gizmo's loc & view.

			MakeSureGizmoIsValid();

			Entity* e = g_app->m_scene.GetCurrentSelection();
			if (e != nullptr)
			{
				Vec3 x, y, z;
				ExtractAxes(glm::toMat4(m_axisOrientation), x, y, z);

				Viewport* vp = g_app->GetActiveViewport();
				Vec3 camOrg = vp->m_camera->m_node->GetTranslation(TransformationSpace::TS_WORLD);
				Vec3 gizmOrg = m_gizmo->m_node->GetTranslation(TransformationSpace::TS_WORLD);
				Vec3 dir = glm::normalize(camOrg - gizmOrg);

				float safetyMeasure = glm::abs(glm::cos(glm::radians(5.0f)));
				AxisLabel axisLabes[3] = { AxisLabel::X, AxisLabel::Y, AxisLabel::Z };
				Vec3 axes[3] = { x, y, z };

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
		}

		String StateTransformBegin::Signaled(SignalId signal)
		{
			if (signal == BaseMod::m_leftMouseBtnDownSgnl)
			{
				Viewport* vp = g_app->GetActiveViewport();
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
			}

			if (signal == BaseMod::m_leftMouseBtnUpSgnl)
			{
				if (m_gizmo != nullptr)
				{
					m_gizmo->Grab(AxisLabel::None);
				}
			}

			if (signal == BaseMod::m_leftMouseBtnDragSgnl)
			{
				Entity* e = g_app->m_scene.GetCurrentSelection();
				if (e == nullptr)
				{
					return StateType::Null;
				}

				if (!m_gizmo->IsGrabbed(AxisLabel::None))
				{
					CalculateIntersectionPlane();
					return StateType::StateTransformTo;
				}
			}

			return StateType::Null;
		}

		void StateTransformBegin::CalculateIntersectionPlane()
		{
			Entity* e = g_app->m_scene.GetCurrentSelection();
			
			Vec3 x, y, z;
			ExtractAxes(glm::toMat4(m_axisOrientation), x, y, z);

			Viewport* vp = g_app->GetActiveViewport();
			Vec3 camOrg = vp->m_camera->m_node->GetTranslation(TransformationSpace::TS_WORLD);
			Vec3 gizmOrg = m_gizmo->m_worldLocation;
			Vec3 dir = glm::normalize(camOrg - gizmOrg);

			Vec3 px, py, pz;
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

		// StateTransformTo
		//////////////////////////////////////////////////////////////////////////

		void StateTransformTo::TransitionIn(State* prevState)
		{
			StateTransformBase::TransitionIn(prevState);

			// Create guide line.
			if ((int)m_gizmo->GetGrabbedAxis() < 3)
			{
				assert(m_gizmo->GetGrabbedAxis() != AxisLabel::None);

				Vec3 p = m_gizmo->m_worldLocation;
				Vec3 color = g_gizmoColor[(int)m_gizmo->GetGrabbedAxis()];
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

		void StateTransformTo::TransitionOut(State* prevState)
		{
			StateTransformBase::TransitionOut(prevState);

			if (m_guideLine != nullptr)
			{
				g_app->m_scene.RemoveEntity(m_guideLine->m_id);
				assert(m_guideLine.use_count() == 1 && "There must be single instance.");
				m_guideLine = nullptr;
			}
		}

		void StateTransformTo::Update(float deltaTime)
		{
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

				if (IsPlaneMod())
				{
					m_delta = p - p0;
				}
				else
				{
					Vec3 projAxis = GetGrabbedAxis(0);
					Vec3 g2p0 = p0 - m_gizmo->m_worldLocation;
					float intsDst = glm::dot(projAxis, g2p0);
					float projDst = glm::dot(projAxis, g2p);

					Vec3 moveAxis = AXIS[(int)m_gizmo->GetGrabbedAxis()];
					m_delta = moveAxis * (projDst - intsDst);
				}
			}
			else
			{
				assert(false && "Intersection expected.");
			}

			std::swap(m_mouseData[0], m_mouseData[1]);
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

		// Signal definitions.
		SignalId TransformMod::m_linkToTransformBeginSgnl = BaseMod::GetNextSignalId();

		TransformMod::TransformMod(ModId id) 
			: BaseMod(id)
		{
			Init();
		}

		TransformMod::~TransformMod()
		{
			if (m_stateMachine->m_currentState != nullptr)
			{
				StateTransformBase* baseState = static_cast<StateTransformBase*> (m_stateMachine->QueryState(StateType::StateTransformBegin));
				assert(baseState && "Gizmo remains in the scene as dead pointer.");

				if (baseState != nullptr && baseState->m_gizmo != nullptr)
				{
					g_app->m_scene.RemoveEntity(baseState->m_gizmo->m_id);
				}
			}
		}

		void TransformMod::Init()
		{
			State* state = new StateTransformBegin();
			m_stateMachine->m_currentState = state;

			m_stateMachine->PushState(state);
			m_stateMachine->PushState(new StateTransformTo());
			m_stateMachine->PushState(new StateTransformEnd());

			m_stateMachine->PushState(new StateBeginPick());
			m_stateMachine->PushState(new StateBeginBoxPick());
			state = new StateEndPick();
			state->m_links[m_linkToTransformBeginSgnl] = StateType::StateTransformBegin;
			m_stateMachine->PushState(state);
		}

		void TransformMod::Update(float deltaTime)
		{
			BaseMod::Update(deltaTime);

			if (m_stateMachine->m_currentState->ThisIsA<StateEndPick>())
			{
				StateEndPick* endPick = static_cast<StateEndPick*> (m_stateMachine->m_currentState);
				
				std::vector<EntityId> entities;
				endPick->PickDataToEntityId(entities);
				g_app->m_scene.AddToSelection(entities, ImGui::GetIO().KeyShift);

				ModManager::GetInstance()->DispatchSignal(m_linkToTransformBeginSgnl);
			}

			if (m_stateMachine->m_currentState->ThisIsA<StateTransformTo>())
			{
				StateTransformTo* t = static_cast<StateTransformTo*> (m_stateMachine->m_currentState);
				Transform(t->m_delta);
				t->m_delta = Vec3(0.0f);
				t->Update(deltaTime); // Update gizmo in this frame.
			}

			if (m_stateMachine->m_currentState->ThisIsA<StateTransformEnd>())
			{
				ModManager::GetInstance()->DispatchSignal(BaseMod::m_backToStart);
			}
		}

		void TransformMod::Transform(const Vec3& delta) const
		{
			std::vector<Entity*> selecteds;
			g_app->m_scene.GetSelectedEntities(selecteds);

			for (Entity* e : selecteds)
			{
				switch (m_id)
				{
				case ModId::Move:
					e->m_node->Translate(delta, g_app->m_transformOrientation);
					break;
				case ModId::Rotate:
					assert(false && "Not implemented.");
					break;
				case ModId::Scale:
					e->m_node->Scale(Vec3(1.0f) + delta, g_app->m_transformOrientation);
					break;
				}
			}
		}

	}
}