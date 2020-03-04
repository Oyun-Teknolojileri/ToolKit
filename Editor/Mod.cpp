#include "stdafx.h"
#include "Mod.h"
#include "GlobalDef.h"
#include "Viewport.h"

ToolKit::Editor::ModManager ToolKit::Editor::ModManager::m_instance;

ToolKit::Editor::ModManager::~ModManager()
{
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

ToolKit::Editor::ModManager::ModManager()
{
}

ToolKit::Editor::BaseMod::BaseMod()
{
	m_stateMachine = new StateMachine();
	m_terminate = false;
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
	m_stateMachine->Signal(signal);
}

void ToolKit::Editor::StateBeginPick::TransitionOut(State* nextState)
{
	if (StatePickingBase* baseState = dynamic_cast<StatePickingBase*> (nextState))
	{
		baseState->m_pickedNtties = m_pickedNtties;
		baseState->m_pickingRays = m_pickingRays;
	}
}

void ToolKit::Editor::StateBeginPick::Update(float deltaTime)
{
}

ToolKit::State* ToolKit::Editor::StateBeginPick::Signaled(SignalId signal)
{
	if (signal == LeftMouseBtnUpSgnl())
	{
		Viewport* vp = g_app->GetActiveViewport();
		if (vp != nullptr)
		{
			Ray ray = vp->RayFromMousePosition();
			m_pickingRays.push_back(ray);

			Scene::PickData pd = g_app->m_scene.PickObject(ray);
			if (pd.entity != nullptr)
			{
				m_pickedNtties.push_back(pd.entity->m_id);
				return new StateEndPick();
			}
		}
	}

	return nullptr;
}

void ToolKit::Editor::StateBeginBoxPick::Update(float deltaTime)
{
}

ToolKit::State* ToolKit::Editor::StateBeginBoxPick::Signaled(SignalId signal)
{
	return nullptr;
}

void ToolKit::Editor::StateEndPick::Update(float deltaTime)
{
}

ToolKit::State* ToolKit::Editor::StateEndPick::Signaled(SignalId signal)
{
	return nullptr;
}

void ToolKit::Editor::SelectMod::Init()
{
	m_stateMachine->PushState(new StateBeginPick());
	m_stateMachine->PushState(new StateBeginBoxPick());
	m_stateMachine->PushState(new StateEndPick());
}

void ToolKit::Editor::SelectMod::Update(float deltaTime)
{
	BaseMod::Update(deltaTime);

	// Final state.
	if (m_stateMachine->m_currentState->m_name == StateEndPick().m_name)
	{
		StateEndPick* endPick = static_cast<StateEndPick*> (m_stateMachine->m_currentState);

	}
}
