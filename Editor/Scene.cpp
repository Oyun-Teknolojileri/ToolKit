#include "stdafx.h"
#include "Scene.h"

ToolKit::Editor::Scene::PickData ToolKit::Editor::Scene::PickObject(Ray ray, const std::vector<EntityId>& ignoreList)
{
	PickData pd;
	pd.pickPos = ray.position + ray.direction * 5.0f;

	float closestPickedDistance = FLT_MAX;
	for (Entity* e : m_entitites)
	{
		if (!e->IsDrawable())
		{
			continue;
		}

		if (std::find(ignoreList.begin(), ignoreList.end(), e->m_id) != ignoreList.end())
		{
			continue;
		}

		Ray rayInObjectSpace = ray;
		glm::mat4 modelTs = e->m_node->GetTransform();
		glm::mat4 InvModelTs = glm::inverse(modelTs);
		rayInObjectSpace.position = InvModelTs * glm::vec4(ray.position, 1.0f);
		rayInObjectSpace.direction = glm::transpose(modelTs) * glm::vec4(ray.direction, 1.0f);

		float dist = 0;
		Drawable* dw = static_cast<Drawable*>(e);
		if (RayBoxIntersection(rayInObjectSpace, dw->GetAABB(), dist))
		{
			if (RayMeshIntersection(dw->m_mesh.get(), rayInObjectSpace, dist))
			{
				if (dist < closestPickedDistance && dist > 0.0f)
				{
					pd.entity = e;
					pd.pickPos = ray.position + ray.direction * dist;
					closestPickedDistance = dist;
				}
			}
		}
	}

	return pd;
}

void ToolKit::Editor::Scene::PickObject(const Frustum& frustum, std::vector<PickData>& pickedObjects, const std::vector<EntityId>& ignoreList, bool pickPartiallyInside)
{
	for (Entity* e : m_entitites)
	{
		if (!e->IsDrawable())
		{
			continue;
		}

		if (std::find(ignoreList.begin(), ignoreList.end(), e->m_id) != ignoreList.end())
		{
			continue;
		}

		BoundingBox bb = e->GetAABB(true);
		IntersectResult res = FrustumBoxIntersection(frustum, bb);
		if (res != IntersectResult::Outside)
		{
			PickData pd;
			pd.pickPos = (bb.max + bb.min) * 0.5f;
			pd.entity = e;

			if (res == IntersectResult::Inside)
			{
				pickedObjects.push_back(pd);
			}
			else if (pickPartiallyInside)
			{
				pickedObjects.push_back(pd);
			}
		}
	}
}

bool ToolKit::Editor::Scene::IsSelected(EntityId id)
{
	return std::find(m_selectedEntities.begin(), m_selectedEntities.end(), id) != m_selectedEntities.end();
}

void ToolKit::Editor::Scene::RemoveFromSelection(EntityId id)
{
	auto nttIt = std::find(m_selectedEntities.begin(), m_selectedEntities.end(), id);
	if (nttIt != m_selectedEntities.end())
	{
		m_selectedEntities.erase(nttIt);
	}
}

void ToolKit::Editor::Scene::AddToSelection(EntityId id)
{
	assert(!IsSelected(id));
	m_selectedEntities.push_back(id);
}

void ToolKit::Editor::Scene::AddToSelection(const std::vector<EntityId>& entities, bool additive)
{
	EntityId currentId = NULL_ENTITY;
	if (entities.size() > 1)
	{
		for (const EntityId& id : entities)
		{
			if (id != NULL_ENTITY)
			{
				if (IsCurrentSelection(id))
				{
					currentId = id;
				}
			}
		}
	}

	// Start with a clear selection list.
	if (!additive)
	{
		ClearSelection();
	}

	for (const EntityId& id : entities)
	{
		if (id == NULL_ENTITY)
		{
			continue;
		}

		// Add to selection.
		if (!additive)
		{
			AddToSelection(id);
		}
		else // Add, make current or toggle selection.
		{
			if (IsSelected(id))
			{
				if (GetSelectedEntityCount() > 1)
				{
					if (entities.size() == 1)
					{
						if (IsCurrentSelection(id))
						{
							RemoveFromSelection(id);
						}
						else
						{
							MakeCurrentSelection(id, false);
						}
					}
				}
				else
				{
					RemoveFromSelection(id);
				}
			}
			else
			{
				AddToSelection(id);
			}
		}
	}

	MakeCurrentSelection(currentId, true);
}

void ToolKit::Editor::Scene::ClearSelection()
{
	m_selectedEntities.clear();
}

bool ToolKit::Editor::Scene::IsCurrentSelection(EntityId id)
{
	if (!m_selectedEntities.empty())
	{
		return m_selectedEntities.back() == id;
	}

	return false;
}

void ToolKit::Editor::Scene::MakeCurrentSelection(EntityId id, bool ifExist)
{
	std::vector<EntityId>::iterator itr = std::find(m_selectedEntities.begin(), m_selectedEntities.end(), id);
	if (itr != m_selectedEntities.end())
	{
		std::iter_swap(itr, m_selectedEntities.end() - 1);
	}
	else
	{
		if (!ifExist)
		{
			m_selectedEntities.push_back(id);
		}
	}
}

ToolKit::uint ToolKit::Editor::Scene::GetSelectedEntityCount()
{
	return (uint)m_selectedEntities.size();
}

ToolKit::Entity* ToolKit::Editor::Scene::GetCurrentSelection()
{
	if (!m_selectedEntities.empty())
	{
		return GetEntity(m_selectedEntities.back());
	}

	return nullptr;
}

ToolKit::Entity* ToolKit::Editor::Scene::GetEntity(EntityId id)
{
	for (Entity* e : m_entitites)
	{
		if (e->m_id == id)
		{
			return e;
		}
	}

	return nullptr;
}

void ToolKit::Editor::Scene::AddEntity(Entity* entity)
{
	m_entitites.push_back(entity);
}

ToolKit::Entity* ToolKit::Editor::Scene::RemoveEntity(EntityId id)
{
	Entity* removed = nullptr;
	for (int i = (int)m_entitites.size() - 1; i >= 0; i--)
	{
		if (m_entitites[i]->m_id == id)
		{
			removed = m_entitites[i];
			m_entitites.erase(m_entitites.begin() + i);
			RemoveFromSelection(id);
			break;
		}
	}

	return removed;
}

const std::vector<ToolKit::Entity*>& ToolKit::Editor::Scene::GetEntities()
{
	return m_entitites;
}
