#include "stdafx.h"
#include "Mod.h"
#include "GlobalDef.h"
#include "Viewport.h"
#include "Node.h"
#include "Primative.h"
#include "DebugNew.h"

ToolKit::Editor::ModManager ToolKit::Editor::ModManager::m_instance;

ToolKit::Editor::ModManager::~ModManager()
{
	for (BaseMod* mod : m_modStack)
	{
		mod->UnInit();
		SafeDel(mod);
	}
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

		BaseMod* nextMod = nullptr;
		switch (mod)
		{
		case ToolKit::Editor::ModId::Select:
			nextMod = new SelectMod();
			break;
		case ToolKit::Editor::ModId::Move:
			break;
		case ToolKit::Editor::ModId::Rotate:
			break;
		case ToolKit::Editor::ModId::Scale:
			break;
		case ToolKit::Editor::ModId::Base:
		default:
			break;
		}

		assert(nextMod);
		if (nextMod != nullptr)
		{
			m_modStack.push_back(nextMod);
		}
	}
}

ToolKit::Editor::ModManager::ModManager()
{
	m_modStack.push_back(new BaseMod(ModId::Base));
}

ToolKit::Editor::BaseMod::BaseMod(ModId id)
{
	m_id = id;
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

void ToolKit::Editor::StateBeginPick::TransitionIn(State* prevState) 
{
}

void ToolKit::Editor::StateBeginPick::TransitionOut(State* nextState)
{
	if (StatePickingBase* baseState = dynamic_cast<StatePickingBase*> (nextState))
	{
		baseState->m_pickedNtties = m_pickedNtties;
		baseState->m_pickingRays = m_pickingRays;
	}

	m_pickedNtties.clear();
	m_pickingRays.clear();
}

void ToolKit::Editor::StateBeginPick::Update(float deltaTime)
{
}

std::string ToolKit::Editor::StateBeginPick::Signaled(SignalId signal)
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
			}

			// Test Shoot rays. For debug visualisation purpose.
			if (g_app->m_pickingDebug)
			{
				if (pd.entity != nullptr)
				{
					g_app->m_hitMarker->m_node->m_translation = pd.pickPos;
				}

				static std::shared_ptr<Arrow2d> rayMdl = nullptr;
				if (rayMdl == nullptr)
				{
					rayMdl = std::shared_ptr<Arrow2d>(new Arrow2d());
					g_app->m_scene.m_entitites.push_back(rayMdl.get());
				}

				rayMdl->m_node->m_translation = ray.position;
				rayMdl->m_node->m_orientation = ToolKit::RotationTo(ToolKit::X_AXIS, ray.direction);
			}

			return StateEndPick().m_name;
		}
	}

	return std::string();
}

void ToolKit::Editor::StateBeginBoxPick::Update(float deltaTime)
{
}

std::string ToolKit::Editor::StateBeginBoxPick::Signaled(SignalId signal)
{
	return std::string();
}

void ToolKit::Editor::StateEndPick::TransitionIn(State* prevState) 
{
	ApplySelection(m_pickedNtties);
}

void ToolKit::Editor::StateEndPick::TransitionOut(State* nextState) 
{
	m_pickedNtties.clear();
	m_pickingRays.clear();
}

void ToolKit::Editor::StateEndPick::Update(float deltaTime)
{
}

std::string ToolKit::Editor::StateEndPick::Signaled(SignalId signal)
{
	// Keep picking.
	if (signal == LeftMouseBtnDownSgnl())
	{
		return StateBeginPick().m_name;
	}

	return std::string();
}

void ToolKit::Editor::SelectMod::Init()
{
	State* initialState = new StateBeginPick();
	m_stateMachine->m_currentState = initialState;

	m_stateMachine->PushState(initialState);
	m_stateMachine->PushState(new StateBeginBoxPick());
	m_stateMachine->PushState(new StateEndPick());
}

void ToolKit::Editor::SelectMod::Update(float deltaTime)
{
	BaseMod::Update(deltaTime);
}

void ToolKit::Editor::StateEndPick::ApplySelection(std::vector<EntityId>& selectedNtties)
{
	bool shiftClick = ImGui::GetIO().KeyShift;

	if (selectedNtties.empty())
	{
		if (!shiftClick)
		{
			g_app->m_scene.ClearSelection();
		}
	}

	for (EntityId id : selectedNtties)
	{
		Entity* e = g_app->m_scene.GetEntity(id);		

		if (e && !shiftClick)
		{
			g_app->m_scene.ClearSelection();
			g_app->m_scene.AddToSelection(e->m_id);
		}

		if (e && shiftClick)
		{
			if (g_app->m_scene.IsSelected(e->m_id))
			{
				g_app->m_scene.RemoveFromSelection(e->m_id);
			}
			else
			{
				g_app->m_scene.AddToSelection(e->m_id);
			}
		}
	}
}
