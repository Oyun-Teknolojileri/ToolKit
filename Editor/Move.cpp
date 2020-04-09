#include "stdafx.h"
#include "Move.h"
#include "Gizmo.h"
#include "GlobalDef.h"
#include "Node.h"
#include "DebugNew.h"
#include "Viewport.h"

ToolKit::Editor::StateMoveBase::StateMoveBase(std::string name)
	: State(name)
{
	m_mouseData.resize(2);
}

void ToolKit::Editor::StateMoveBase::TransitionIn(State* prevState)
{
}

void ToolKit::Editor::StateMoveBase::TransitionOut(State* nextState)
{
	StateMoveBase* baseState = dynamic_cast<StateMoveBase*> (nextState);
	if (baseState != nullptr)
	{
		baseState->m_gizmo = m_gizmo;
	}
}

void ToolKit::Editor::StateBeginMove::TransitionIn(State* prevState)
{
}

void ToolKit::Editor::StateBeginMove::TransitionOut(State* nextState)
{
	if (nextState->m_name == StateBeginPick().m_name)
	{
		StateBeginPick* baseNext = static_cast<StateBeginPick*> (nextState);
		baseNext->m_mouseData = m_mouseData;
	}
}

void ToolKit::Editor::StateBeginMove::Update(float deltaTime)
{
	if (g_app->m_scene.GetSelectedEntityCount() > 0)
	{
		if (m_gizmo == nullptr)
		{
			m_gizmo = std::make_shared<MoveGizmo>();
		}

		if (g_app->m_scene.GetEntity(m_gizmo->m_id) == nullptr)
		{
			g_app->m_scene.AddEntity(m_gizmo.get());
		}
		
		Entity* e = g_app->m_scene.GetCurrentSelection();
		m_gizmo->m_worldLocation = e->m_node->GetTranslation(TransformationSpace::TS_WORLD);
		m_gizmo->m_node->m_orientation = e->m_node->GetOrientation(TransformationSpace::TS_WORLD);
		m_gizmo->Update(deltaTime);
	}
	else
	{
		if (m_gizmo != nullptr)
		{
			g_app->m_scene.RemoveEntity(m_gizmo->m_id);
		}
	}
}

std::string ToolKit::Editor::StateBeginMove::Signaled(SignalId signal)
{
	if (signal == BaseMod::m_leftMouseBtnDownSgnl)
	{
		Viewport* vp = g_app->GetActiveViewport();
		if (vp != nullptr)
		{
			m_mouseData[0] = vp->GetLastMousePosScreenSpace();
		}

		return StateBeginPick().m_name;
	}

	return "";
}

// Signal definitions.
namespace ToolKit
{
	namespace Editor
	{
		SignalId MoveMod::m_linkToMoveBeginSgnl = BaseMod::GetNextSignalId();
	}
}

ToolKit::Editor::MoveMod::~MoveMod()
{
	if (m_stateMachine->m_currentState != nullptr)
	{
		StateMoveBase* baseState = static_cast<StateMoveBase*> (m_stateMachine->QueryState(StateBeginMove().m_name));
		assert(baseState && "Gizmo remains in the scene as dead pointer.");

		if (baseState != nullptr && baseState->m_gizmo != nullptr)
		{
			g_app->m_scene.RemoveEntity(baseState->m_gizmo->m_id);
		}
	}
}

void ToolKit::Editor::MoveMod::Init()
{
	State* state = new StateBeginMove();
	m_stateMachine->m_currentState = state;

	m_stateMachine->PushState(state);
	m_stateMachine->PushState(new StateBeginPick());
	m_stateMachine->PushState(new StateBeginBoxPick());

	state = new StateEndPick();
	state->m_links[m_linkToMoveBeginSgnl] = StateBeginMove().m_name;
	m_stateMachine->PushState(state);
}

void ToolKit::Editor::MoveMod::Update(float deltaTime)
{
	BaseMod::Update(deltaTime);

	if (m_stateMachine->m_currentState->m_name == StateEndPick().m_name)
	{
		StateEndPick* endPick = static_cast<StateEndPick*> (m_stateMachine->m_currentState);
		std::vector<EntityId> entities;
		endPick->PickDataToEntityId(entities);
		g_app->m_scene.AddToSelection(entities, ImGui::GetIO().KeyShift);

		ModManager::GetInstance()->DispatchSignal(m_linkToMoveBeginSgnl);
	}
}
