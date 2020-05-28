#include "stdafx.h"
#include "Scene.h"

namespace ToolKit
{
	namespace Editor
	{

		Scene::Scene()
		{
			m_name = "NewScene";
		}

		Scene::~Scene()
		{
			for (Entity* ntt : m_entitites)
			{
				SafeDel(ntt);
			}
			m_entitites.clear();
			m_selectedEntities.clear();
		}

		Scene::PickData Scene::PickObject(Ray ray, const EntityIdArray& ignoreList) const
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
				Mat4 ts = e->m_node->GetTransform(TransformationSpace::TS_WORLD);
				Mat4 its = glm::inverse(ts);
				rayInObjectSpace.position = its * Vec4(ray.position, 1.0f);
				rayInObjectSpace.direction = its * Vec4(ray.direction, 0.0f);

				float dist = 0;
				Drawable* dw = static_cast<Drawable*>(e);
				if (RayBoxIntersection(rayInObjectSpace, dw->GetAABB(), dist))
				{
					bool hit = true;
					if (dw->m_mesh->m_clientSideVertices.size() == dw->m_mesh->m_vertexCount)
					{
						// Per polygon check if data exist.
						hit = RayMeshIntersection(dw->m_mesh.get(), rayInObjectSpace, dist);
					}
					if (hit)
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

		void Scene::PickObject(const Frustum& frustum, std::vector<PickData>& pickedObjects, const EntityIdArray& ignoreList, bool pickPartiallyInside) const
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

		bool Scene::IsSelected(EntityId id) const
		{
			return std::find(m_selectedEntities.begin(), m_selectedEntities.end(), id) != m_selectedEntities.end();
		}

		void Scene::RemoveFromSelection(EntityId id)
		{
			auto nttIt = std::find(m_selectedEntities.begin(), m_selectedEntities.end(), id);
			if (nttIt != m_selectedEntities.end())
			{
				m_selectedEntities.erase(nttIt);
			}
		}

		void Scene::AddToSelection(EntityId id, bool additive)
		{
			assert(!IsSelected(id));
			if (!additive)
			{
				m_selectedEntities.clear();
			}
			m_selectedEntities.push_back(id);
		}

		void Scene::AddToSelection(const EntityIdArray& entities, bool additive)
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
					AddToSelection(id, true);
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
						AddToSelection(id, true);
					}
				}
			}

			MakeCurrentSelection(currentId, true);
		}

		void Scene::AddToSelection(const EntityRawPtrArray& entities, bool additive)
		{
			EntityIdArray ids;
			ToEntityIdArray(ids, entities);
			AddToSelection(ids, additive);
		}

		void Scene::ClearSelection()
		{
			m_selectedEntities.clear();
		}

		bool Scene::IsCurrentSelection(EntityId id) const
		{
			if (!m_selectedEntities.empty())
			{
				return m_selectedEntities.back() == id;
			}

			return false;
		}

		void Scene::MakeCurrentSelection(EntityId id, bool ifExist)
		{
			EntityIdArray::iterator itr = std::find(m_selectedEntities.begin(), m_selectedEntities.end(), id);
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

		uint Scene::GetSelectedEntityCount() const
		{
			return (uint)m_selectedEntities.size();
		}

		Entity* Scene::GetCurrentSelection() const
		{
			if (!m_selectedEntities.empty())
			{
				return GetEntity(m_selectedEntities.back());
			}

			return nullptr;
		}

		Entity* Scene::GetEntity(EntityId id) const
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

		void Scene::AddEntity(Entity* entity)
		{
			assert(GetEntity(entity->m_id) == nullptr && "Entity is already in the scene.");
			m_entitites.push_back(entity);
		}

		Entity* Scene::RemoveEntity(EntityId id)
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

		const EntityRawPtrArray& Scene::GetEntities() const
		{
			return m_entitites;
		}

		void Scene::GetSelectedEntities(EntityRawPtrArray& entities) const
		{
			for (EntityId id : m_selectedEntities)
			{
				Entity* e = GetEntity(id);
				assert(e);

				entities.push_back(e);
			}
		}

		void Scene::GetSelectedEntities(EntityIdArray& entities) const
		{
			entities = m_selectedEntities;
		}

		const String XmlSceneElement("S");

		void Scene::Serialize(XmlDocument* doc, XmlNode* parent)
		{
			std::ofstream file;
			String fileName = m_name + ".scene";

			file.open(fileName.c_str(), std::ios::out);
			if (file.is_open())
			{
				XmlNode* scene = doc->allocate_node(rapidxml::node_element, XmlSceneElement.c_str(), m_name.c_str());

				if (parent != nullptr)
				{
					parent->append_node(scene);
				}
				else
				{
					doc->append_node(scene);
				}

				for (Entity* ntt : m_entitites)
				{
					ntt->Serialize(doc, scene);
				}

				std::string xml;
				rapidxml::print(std::back_inserter(xml), *doc, 0);

				file << xml;
				file.close();
				doc->clear();
			}
		}

	}
}
