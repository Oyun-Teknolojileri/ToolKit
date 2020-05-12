#include "stdafx.h"
#include "Mod.h"
#include "GlobalDef.h"
#include "Viewport.h"
#include "Node.h"
#include "Primative.h"
#include "Grid.h"
#include "Directional.h"
#include "Gizmo.h"
#include "TransformMod.h"
#include "ConsoleWindow.h"
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
		}

		void ActionManager::UnInit()
		{
			for (Action* action : m_actionStack)
			{
				SafeDel(action);
			}

			m_initiated = false;
		}

		void ActionManager::AddAction(Action* action)
		{
			if (m_stackPointer > -1)
			{
				if (m_stackPointer < (int)m_actionStack.size() - 1)
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
			if (m_actionStack.size() > g_maxUndoCount)
			{
				Action* a = m_actionStack.front();
				SafeDel(a);

				pop_front(m_actionStack);
			}
			m_stackPointer = (int)m_actionStack.size() - 1;
		}

		void ActionManager::GroupLastActions(int n)
		{
			assert((int)m_actionStack.size() >= n);
			assert(m_stackPointer == (int)m_actionStack.size() - 1 && "Call grouping right after add.");

			int begIndx = (int)m_actionStack.size() - n;
			Action* root = m_actionStack[begIndx++];
			root->m_group.reserve(n - 1);
			for (int i = begIndx; i < (int)m_actionStack.size(); i++)
			{
				root->m_group.push_back(m_actionStack[i]);
			}

			m_actionStack.erase(m_actionStack.begin() + begIndx, m_actionStack.end());
			m_stackPointer = (int)m_actionStack.size() - 1;
		}

		void ActionManager::Undo()
		{
			if (!m_actionStack.empty())
			{
				if (m_stackPointer > -1)
				{
					Action* action = m_actionStack[m_stackPointer];
					action->Undo();
					for (Action* ga : action->m_group)
					{
						ga->Undo();
					}
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

			if (m_stackPointer < (int)m_actionStack.size() - 1)
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
				mod->UnInit();
				SafeDel(mod);
			}
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
							String log = "\t" + prevStateDbg->GetType() + " -> " + nextState->GetType();
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
			if (StatePickingBase* baseState = dynamic_cast<StatePickingBase*> (nextState))
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

		bool StatePickingBase::IsIgnored(EntityId id)
		{
			return std::find(m_ignoreList.begin(), m_ignoreList.end(), id) != m_ignoreList.end();
		}

		void StatePickingBase::PickDataToEntityId(EntityIdArray& ids)
		{
			for (Scene::PickData& pd : m_pickData)
			{
				if (pd.entity != nullptr)
				{
					ids.push_back(pd.entity->m_id);
				}
				else
				{
					ids.push_back(NULL_ENTITY);
				}
			}
		}

		void StateBeginPick::Update(float deltaTime)
		{
		}

		String StateBeginPick::Signaled(SignalId signal)
		{
			if (signal == BaseMod::m_leftMouseBtnDownSgnl)
			{
				Viewport* vp = g_app->GetActiveViewport();
				if (vp != nullptr)
				{
					m_mouseData[0] = vp->GetLastMousePosScreenSpace();
				}
			}

			if (signal == BaseMod::m_leftMouseBtnUpSgnl)
			{
				Viewport* vp = g_app->GetActiveViewport();
				if (vp != nullptr)
				{
					m_mouseData[0] = vp->GetLastMousePosScreenSpace();

					Ray ray = vp->RayFromMousePosition();
					Scene::PickData pd = g_app->m_scene.PickObject(ray, m_ignoreList);
					m_pickData.push_back(pd);

					if (g_app->m_showPickingDebug)
					{
						g_app->m_cursor->m_worldLocation = pd.pickPos;
						if (m_dbgArrow == nullptr)
						{
							m_dbgArrow = std::shared_ptr<Arrow2d>(new Arrow2d());
							m_ignoreList.push_back(m_dbgArrow->m_id);
							g_app->m_scene.AddEntity(m_dbgArrow.get());
						}

						m_dbgArrow->m_node->SetTranslation(ray.position);
						m_dbgArrow->m_node->SetOrientation(RotationTo(X_AXIS, ray.direction));
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

		void StateBeginBoxPick::Update(float deltaTime)
		{
		}

		String StateBeginBoxPick::Signaled(SignalId signal)
		{
			if (signal == BaseMod::m_leftMouseBtnUpSgnl)
			{
				// Frustum - AABB test.
				Viewport* vp = g_app->GetActiveViewport();
				if (vp != nullptr)
				{
					Camera* cam = vp->m_camera;

					Vec2 rect[4];
					GetMouseRect(rect[0], rect[2]);

					rect[1].x = rect[2].x;
					rect[1].y = rect[0].y;
					rect[3].x = rect[0].x;
					rect[3].y = rect[2].y;

					std::vector<Ray> rays;
					std::vector<Vec3> rect3d;

					// Front rectangle.
					Vec3 lensLoc = cam->m_node->GetTranslation(TransformationSpace::TS_WORLD);
					for (int i = 0; i < 4; i++)
					{
						Vec2 p = vp->TransformScreenToViewportSpace(rect[i]);
						Vec3 p0 = vp->TransformViewportToWorldSpace(p);
						rect3d.push_back(p0);
						rays.push_back({ lensLoc, glm::normalize(p0 - lensLoc) });
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
					planePnts = { rect3d[0], rect3d[4], rect3d[3] }; // Left plane.
					frustum.planes[0] = PlaneFrom(planePnts.data());

					planePnts = { rect3d[1], rect3d[2], rect3d[5] }; // Right plane.
					frustum.planes[1] = PlaneFrom(planePnts.data());

					planePnts = { rect3d[0], rect3d[1], rect3d[4] }; // Top plane.
					frustum.planes[2] = PlaneFrom(planePnts.data());

					planePnts = { rect3d[2], rect3d[7], rect3d[6] }; // Bottom plane.
					frustum.planes[3] = PlaneFrom(planePnts.data());

					planePnts = { rect3d[0], rect3d[2], rect3d[1] }; // Near plane.
					frustum.planes[4] = PlaneFrom(planePnts.data());

					planePnts = { rect3d[4], rect3d[5], rect3d[6] }; // Far plane.
					frustum.planes[5] = PlaneFrom(planePnts.data());

					// Perform picking.
					std::vector<Scene::PickData> ntties;
					g_app->m_scene.PickObject(frustum, ntties, m_ignoreList);
					m_pickData.insert(m_pickData.end(), ntties.begin(), ntties.end());

					// Debug draw the picking frustum.
					if (g_app->m_showPickingDebug)
					{
						std::vector<Vec3> corners =
						{
							rect3d[0], rect3d[1], rect3d[1], rect3d[2], rect3d[2], rect3d[3], rect3d[3], rect3d[0],
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
							m_dbgFrustum = std::shared_ptr<LineBatch>(new LineBatch(corners, X_AXIS, DrawType::Line));
							m_ignoreList.push_back(m_dbgFrustum->m_id);
							g_app->m_scene.AddEntity(m_dbgFrustum.get());
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
				Viewport* vp = g_app->GetActiveViewport();
				if (vp != nullptr)
				{
					m_mouseData[1] = vp->GetLastMousePosScreenSpace();

					auto drawSelectionRectangleFn = [this](ImDrawList* drawList) -> void
					{
						Vec2 min, max;
						GetMouseRect(min, max);

						ImU32 col = ImColor(GLM4IMVEC(g_selectBoxWindowColor));
						drawList->AddRectFilled(GLM2IMVEC(min), GLM2IMVEC(max), col, 5.0f);

						col = ImColor(GLM4IMVEC(g_selectBoxBorderColor));
						drawList->AddRect(GLM2IMVEC(min), GLM2IMVEC(max), col, 5.0f, 15, 2.0f);
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

		void StateEndPick::Update(float deltaTime)
		{
		}

		String StateEndPick::Signaled(SignalId signal)
		{
			return StateType::Null;
		}

		DeleteAction::DeleteAction(Entity* ntt)
		{
			m_ntt = ntt;
			g_app->m_scene.RemoveEntity(ntt->m_id);
			m_actionComitted = true;
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
			
			m_actionComitted = false;
			g_app->m_scene.AddEntity(m_ntt);
		}

		void DeleteAction::Redo()
		{
			m_actionComitted = true;
			g_app->m_scene.RemoveEntity(m_ntt->m_id);
		}

		void StateDeletePick::Update(float deltaTime)
		{
			EntityRawPtrArray deleteList;
			g_app->m_scene.GetSelectedEntities(deleteList);
			for (Entity* e : deleteList)
			{
				ActionManager::GetInstance()->AddAction(new DeleteAction(e));
			}
			ActionManager::GetInstance()->GroupLastActions((int)deleteList.size());
		}

		ToolKit::String StateDeletePick::Signaled(SignalId signal)
		{
			return StateType::Null;
		}

		// Mods
		//////////////////////////////////////////////////////////////////////////

		void SelectMod::Init()
		{
			StateBeginPick* initialState = new StateBeginPick();
			m_stateMachine->m_currentState = initialState;
			initialState->m_ignoreList = { g_app->m_grid->m_id };

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
				StateEndPick* endPick = static_cast<StateEndPick*> (m_stateMachine->m_currentState);
				EntityIdArray entities;
				endPick->PickDataToEntityId(entities);
				g_app->m_scene.AddToSelection(entities, ImGui::GetIO().KeyShift);

				ModManager::GetInstance()->DispatchSignal(BaseMod::m_backToStart);
			}

			if (m_stateMachine->m_currentState->GetType() == StateType::StateDeletePick)
			{
				ModManager::GetInstance()->DispatchSignal(BaseMod::m_backToStart);
			}
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
				StateEndPick* endPick = static_cast<StateEndPick*> (m_stateMachine->m_currentState);
				Scene::PickData& pd = endPick->m_pickData.back();
				g_app->m_cursor->m_worldLocation = pd.pickPos;

				m_stateMachine->Signal(BaseMod::m_backToStart);
			}
		}

	}
}
