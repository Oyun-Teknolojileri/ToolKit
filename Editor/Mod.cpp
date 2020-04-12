#include "stdafx.h"
#include "Mod.h"
#include "GlobalDef.h"
#include "Viewport.h"
#include "Node.h"
#include "Primative.h"
#include "Grid.h"
#include "Directional.h"
#include "Gizmo.h"
#include "Move.h"
#include "DebugNew.h"
#include "ConsoleWindow.h"

ToolKit::Editor::ModManager ToolKit::Editor::ModManager::m_instance;

ToolKit::Editor::ModManager::~ModManager()
{
	assert(m_initiated == false && "Call UnInit.");
}

ToolKit::Editor::ModManager* ToolKit::Editor::ModManager::GetInstance()
{
	return &m_instance;
}

void ToolKit::Editor::ModManager::Update(float deltaTime)
{
	if (m_modStack.empty())
	{
		return;
	}

	BaseMod* currentMod = m_modStack.back();
	currentMod->Update(deltaTime);
}

void ToolKit::Editor::ModManager::DispatchSignal(SignalId signal)
{
	if (m_modStack.empty())
	{
		return;
	}

	m_modStack.back()->Signal(signal);
}

void ToolKit::Editor::ModManager::SetMod(bool set, ModId mod)
{
	if (set)
	{
		if (m_modStack.back()->m_id != ModId::Base)
		{
			BaseMod* prevMod = m_modStack.back();
			SafeDel(prevMod);
			m_modStack.pop_back();
		}

		static std::string modNameDbg;
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
			nextMod = new MoveMod();
			modNameDbg = "Mod: Move";
			break;
		case ModId::Rotate:
			break;
		case ModId::Scale:
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

ToolKit::Editor::ModManager::ModManager()
{
	m_initiated = false;
}

void ToolKit::Editor::ModManager::Init()
{
	m_modStack.push_back(new BaseMod(ModId::Base));
	m_initiated = true;
}

void ToolKit::Editor::ModManager::UnInit()
{
	for (BaseMod* mod : m_modStack)
	{
		mod->UnInit();
		SafeDel(mod);
	}
	m_initiated = false;
}

// Signal definitions.
namespace ToolKit
{
	namespace Editor
	{
		SignalId BaseMod::m_leftMouseBtnDownSgnl = BaseMod::GetNextSignalId();
		SignalId BaseMod::m_leftMouseBtnUpSgnl = BaseMod::GetNextSignalId();
		SignalId BaseMod::m_leftMouseBtnDragSgnl = BaseMod::GetNextSignalId();
		SignalId BaseMod::m_mouseMoveSgnl = BaseMod::GetNextSignalId();
		SignalId BaseMod::m_backToStart = BaseMod::GetNextSignalId();
	}
}

ToolKit::Editor::BaseMod::BaseMod(ModId id)
{
	m_id = id;
	m_stateMachine = new StateMachine();
}

ToolKit::Editor::BaseMod::~BaseMod()
{
	SafeDel(m_stateMachine);
}

void ToolKit::Editor::BaseMod::Init()
{
}

void ToolKit::Editor::BaseMod::UnInit()
{
}

void ToolKit::Editor::BaseMod::Update(float deltaTime)
{
	m_stateMachine->Update(deltaTime);
}

void ToolKit::Editor::BaseMod::Signal(SignalId signal)
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
					std::string log = "\t" + prevStateDbg->GetType() + " -> " + nextState->GetType();
					consol->AddLog(log, "ModDbg");
				}
			}
		}
	}
}

int ToolKit::Editor::BaseMod::GetNextSignalId()
{
	static int signalCounter = 100;
	return ++signalCounter;
}


// State definitions.
namespace ToolKit
{
	namespace Editor
	{
		const std::string StateType::Null = "";
		const std::string StateType::StateBeginPick = "StateBeginPick";
		const std::string StateType::StateBeginBoxPick = "StateBeginBoxPick";
		const std::string StateType::StateEndPick = "StateEndPick";
		const std::string StateType::StateBeginMove = "StateBeginMove";
		const std::string StateType::StateMoveTo = "StateMoveTo";
		const std::string StateType::StateEndMove = "StateEndMove";

		std::shared_ptr<Arrow2d> StatePickingBase::m_dbgArrow = nullptr;
		std::shared_ptr<LineBatch> StatePickingBase::m_dbgFrustum = nullptr;
	}
}

ToolKit::Editor::StatePickingBase::StatePickingBase()
{ 
	m_mouseData.resize(2); 
}

void ToolKit::Editor::StatePickingBase::TransitionIn(State* prevState)
{
}

void ToolKit::Editor::StatePickingBase::TransitionOut(State* nextState)
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

bool ToolKit::Editor::StatePickingBase::IsIgnored(EntityId id)
{
	return std::find(m_ignoreList.begin(), m_ignoreList.end(), id) != m_ignoreList.end();
}

void ToolKit::Editor::StatePickingBase::PickDataToEntityId(std::vector<EntityId>& ids)
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

void ToolKit::Editor::StateBeginPick::Update(float deltaTime)
{
}

std::string ToolKit::Editor::StateBeginPick::Signaled(SignalId signal)
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

				m_dbgArrow->m_node->m_translation = ray.position;
				m_dbgArrow->m_node->m_orientation = ToolKit::RotationTo(ToolKit::X_AXIS, ray.direction);
			}

			return StateType::StateEndPick;
		}
	}

	if (signal == BaseMod::m_leftMouseBtnDragSgnl)
	{
		return StateType::StateBeginBoxPick;
	}

	return StateType::Null;
}

void ToolKit::Editor::StateBeginBoxPick::Update(float deltaTime)
{
}

std::string ToolKit::Editor::StateBeginBoxPick::Signaled(SignalId signal)
{
	if (signal == BaseMod::m_leftMouseBtnUpSgnl)
	{
		// Frustum - AABB test.
		Viewport* vp = g_app->GetActiveViewport();
		if (vp != nullptr)
		{
			Camera* cam = vp->m_camera;
			
			glm::vec2 rect[4];
			GetMouseRect(rect[0], rect[2]);

			rect[1].x = rect[2].x;
			rect[1].y = rect[0].y;
			rect[3].x = rect[0].x;
			rect[3].y = rect[2].y;

			std::vector<Ray> rays;
			std::vector<glm::vec3> rect3d;

			// Front rectangle.
			glm::vec3 lensLoc = cam->m_node->GetTranslation(TransformationSpace::TS_WORLD);
			for (int i = 0; i < 4; i++)
			{
				glm::vec2 p = vp->TransformScreenToViewportSpace(rect[i]);
				glm::vec3 p0 = vp->TransformViewportToWorldSpace(p);
				rect3d.push_back(p0);
				rays.push_back({ lensLoc, glm::normalize(p0 - lensLoc) });
			}

			// Back rectangle.
			float depth = 1000.0f;
			for (int i = 0; i < 4; i++)
			{
				glm::vec3 p = rect3d[i] + rays[i].direction * depth;
				rect3d.push_back(p);
			}

			// Frustum from 8 points.
			Frustum frustum;
			std::vector<glm::vec3> planePnts;			
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
				std::vector<glm::vec3> corners =
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
					m_dbgFrustum = std::shared_ptr<LineBatch>(new LineBatch(corners, ToolKit::X_AXIS, DrawType::Line));
					m_ignoreList.push_back(m_dbgFrustum->m_id);
					g_app->m_scene.AddEntity(m_dbgFrustum.get());
				}
				else
				{
					m_dbgFrustum->Generate(corners, ToolKit::X_AXIS, DrawType::Line);
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
				glm::vec2 min, max;
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

void ToolKit::Editor::StateBeginBoxPick::GetMouseRect(glm::vec2& min, glm::vec2& max)
{
	min = glm::vec2(FLT_MAX, FLT_MAX);
	max = glm::vec2(-FLT_MAX, -FLT_MAX);

	for (int i = 0; i < 2; i++)
	{
		min = glm::min(min, m_mouseData[i]);
		max = glm::max(max, m_mouseData[i]);
	}
}

void ToolKit::Editor::StateEndPick::Update(float deltaTime)
{
}

std::string ToolKit::Editor::StateEndPick::Signaled(SignalId signal)
{
	// Keep picking.
	if (signal == BaseMod::m_leftMouseBtnDownSgnl)
	{
		Viewport* vp = g_app->GetActiveViewport();
		if (vp != nullptr && vp->IsViewportQueriable())
		{
			m_mouseData[0] = vp->GetLastMousePosScreenSpace();
			return StateType::StateBeginPick;
		}
	}

	return StateType::Null;
}

void ToolKit::Editor::SelectMod::Init()
{
	StateBeginPick* initialState = new StateBeginPick();
	m_stateMachine->m_currentState = initialState;
	initialState->m_ignoreList = { g_app->m_grid->m_id };

	m_stateMachine->PushState(initialState);
	m_stateMachine->PushState(new StateBeginBoxPick());
	m_stateMachine->PushState(new StateEndPick());
}

void ToolKit::Editor::SelectMod::Update(float deltaTime)
{
	BaseMod::Update(deltaTime);
	static bool stateTransition = false;

	if (m_stateMachine->m_currentState->GetType() == StateType::StateBeginPick)
	{
		stateTransition = true;
	}

	if (m_stateMachine->m_currentState->GetType() == StateType::StateEndPick && stateTransition)
	{
		stateTransition = false;

		StateEndPick* endPick = static_cast<StateEndPick*> (m_stateMachine->m_currentState);
		std::vector<EntityId> entities;
		endPick->PickDataToEntityId(entities);
		g_app->m_scene.AddToSelection(entities, ImGui::GetIO().KeyShift);
	}
}

void ToolKit::Editor::CursorMod::Init()
{
	State* initialState = new StateBeginPick();
	m_stateMachine->m_currentState = initialState;

	m_stateMachine->PushState(initialState);
	m_stateMachine->PushState(new StateEndPick());
}

void ToolKit::Editor::CursorMod::Update(float deltaTime)
{
	BaseMod::Update(deltaTime);
	static bool stateTransition = false;

	if (m_stateMachine->m_currentState->GetType() == StateType::StateBeginPick)
	{
		stateTransition = true;
	}

	if (m_stateMachine->m_currentState->GetType() == StateType::StateEndPick && stateTransition)
	{
		stateTransition = false;

		StateEndPick* endPick = static_cast<StateEndPick*> (m_stateMachine->m_currentState);
		Scene::PickData& pd = endPick->m_pickData.back();
		g_app->m_cursor->m_worldLocation = pd.pickPos;
	}
}
