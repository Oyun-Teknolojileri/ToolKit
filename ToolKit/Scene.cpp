#include "Scene.h"

#include <string>
#include <vector>
#include <algorithm>

#include "ToolKit.h"
#include "Util.h"
#include "Component.h"
#include "ResourceComponent.h"
#include "DebugNew.h"

namespace ToolKit
{

  Scene::Scene()
  {
    m_name = "New Scene";
  }

  Scene::Scene(String file)
    : Scene()
  {
    SetFile(file);
  }

  Scene::~Scene()
  {
    Destroy(false);
  }

  void Scene::Load()
  {
    if (m_loaded)
    {
      return;
    }

    String path = GetFile();
    NormalizePath(path);
    XmlFile sceneFile = GetFileManager()->GetXmlFile(path);
    XmlDocument sceneDoc;
    sceneDoc.parse<0>(sceneFile.data());

    DeSerialize(&sceneDoc, nullptr);

    // Update parent - child relation for entities.
    for (Entity* e : m_entities)
    {
      if (e->_parentId != 0)
      {
        Entity* parent = GetEntity(e->_parentId);
        if (parent)
        {
          parent->m_node->AddChild(e->m_node);
        }
      }
    }

    m_loaded = true;
  }

  void Scene::Save(bool onlyIfDirty)
  {
    String fullPath = GetFile();
    if (fullPath.empty())
    {
      fullPath = ScenePath(m_name + SCENE);
    }

    std::ofstream file;
    file.open(fullPath.c_str(), std::ios::out);
    if (file.is_open())
    {
      XmlDocument doc;
      Serialize(&doc, nullptr);

      std::string xml;
      rapidxml::print(std::back_inserter(xml), doc, 0);

      file << xml;
      file.close();
      doc.clear();
    }
  }

  void Scene::Init(bool flushClientSideArray)
  {
    if (m_initiated)
    {
      return;
    }

    const EntityRawPtrArray& ntties = GetEntities();
    for (Entity* ntt : ntties)
    {
      if (ntt->IsDrawable())
      {
        MeshComponentPtrArray meshes;
        ntt->GetComponent<MeshComponent>(meshes);
        for (MeshComponentPtr& mesh : meshes)
        {
          mesh->Init(flushClientSideArray);
        }
      }
    }

    m_initiated = true;
  }

  void Scene::UnInit()
  {
    Destroy(false);
  }

  void Scene::Merge(ScenePtr other)
  {
    ULongID lastID = GetHandleManager()->GetNextHandle(), biggestID = 0;
    const EntityRawPtrArray& entities = other->GetEntities();
    for (Entity* ntt : entities)
    {
      AddEntity(ntt);  // Insert into this scene.
    }
    GetHandleManager()->SetMaxHandle(biggestID);

    other->RemoveAllEntities();
    GetSceneManager()->Remove(other->GetFile());
  }

  Scene::PickData Scene::PickObject
  (
    Ray ray,
    const EntityIdArray& ignoreList
  ) const
  {
    PickData pd;
    pd.pickPos = ray.position + ray.direction * 5.0f;

    float closestPickedDistance = FLT_MAX;
    for (Entity* ntt : m_entities)
    {
      if (!ntt->IsDrawable())
      {
        continue;
      }

      if
      (
        std::find(ignoreList.begin(), ignoreList.end(), ntt->GetIdVal())
        != ignoreList.end()
      )
      {
        continue;
      }

      Ray rayInObjectSpace = ray;
      Mat4 ts = ntt->m_node->GetTransform(TransformationSpace::TS_WORLD);
      Mat4 its = glm::inverse(ts);
      rayInObjectSpace.position = its * Vec4(ray.position, 1.0f);
      rayInObjectSpace.direction = its * Vec4(ray.direction, 0.0f);

      float dist = 0;
      if (RayBoxIntersection(rayInObjectSpace, ntt->GetAABB(), dist))
      {
        bool hit = true;

        // Collect meshes.
        MeshComponentPtrArray meshes;
        ntt->GetComponent<MeshComponent>(meshes);

        for (MeshComponentPtr& meshCmp : meshes)
        {
          MeshPtr mesh = meshCmp->GetMeshVal();
          if (mesh->m_clientSideVertices.size() == mesh->m_vertexCount)
          {
            // Per polygon check if data exist.
            float meshDist = 0.0f;
            hit = RayMeshIntersection(mesh.get(), rayInObjectSpace, meshDist);
            if (hit)
            {
              dist = meshDist;
            }
          }

          if (hit)
          {
            if (dist < closestPickedDistance && dist > 0.0f)
            {
              pd.entity = ntt;
              pd.pickPos = ray.position + ray.direction * dist;
              closestPickedDistance = dist;
            }
          }
        }
      }
    }

    return pd;
  }

  void Scene::PickObject
  (
    const Frustum& frustum,
    std::vector<PickData>& pickedObjects,
    const EntityIdArray& ignoreList,
    bool pickPartiallyInside
  ) const
  {
    for (Entity* e : m_entities)
    {
      if (!e->IsDrawable())
      {
        continue;
      }

      if
      (
        std::find
        (
          ignoreList.begin(),
          ignoreList.end(),
          e->GetIdVal()
        ) != ignoreList.end()
      )
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

  Entity* Scene::GetEntity(ULongID id) const
  {
    for (Entity* e : m_entities)
    {
      if (e->GetIdVal() == id)
      {
        return e;
      }
    }

    return nullptr;
  }

  void Scene::AddEntity(Entity* entity)
  {
    ULongID nttyID = entity->GetIdVal();
    assert
    (
      GetEntity(nttyID) == nullptr &&
      "Entity is already in the scene."
    );
    m_entities.push_back(entity);
  }

  Entity* Scene::RemoveEntity(ULongID id)
  {
    Entity* removed = nullptr;
    for (int i = static_cast<int>(m_entities.size()) - 1; i >= 0; i--)
    {
      if (m_entities[i]->GetIdVal() == id)
      {
        removed = m_entities[i];
        m_entities.erase(m_entities.begin() + i);
        break;
      }
    }

    return removed;
  }

  void Scene::RemoveEntity(const EntityRawPtrArray& entities)
  {
    for (Entity* ntt : entities)
    {
      RemoveEntity(ntt->GetIdVal());
    }
  }

  void Scene::RemoveAllEntities()
  {
    m_entities.clear();
  }

  const EntityRawPtrArray& Scene::GetEntities() const
  {
    return m_entities;
  }

  const LightRawPtrArray Scene::GetLights() const
  {
    LightRawPtrArray lights;
    for (Entity* ntt : m_entities)
    {
      if (ntt->GetType() == EntityType::Entity_Light)
      {
        lights.push_back(static_cast<Light*>(ntt));
      }
    }

    return lights;
  }

  Entity* Scene::GetFirstEntityByName(const String& name)
  {
    for (Entity* e : m_entities)
    {
      if (e->GetNameVal() == name)
      {
        return e;
      }
    }
    return nullptr;
  }

  EntityRawPtrArray Scene::GetByTag(const String& tag)
  {
    EntityRawPtrArray arrayByTag;
    for (Entity* e : m_entities)
    {
      StringArray tokens;
      Split(e->GetTagVal(), ".", tokens);

      for (const String& token : tokens)
      {
        if (token == tag)
        {
          arrayByTag.push_back(e);
        }
      }
    }

    return arrayByTag;
  }

  Entity* Scene::GetFirstByTag(const String& tag)
  {
    EntityRawPtrArray res = GetByTag(tag);
    return res.empty() ? nullptr : res.front();
  }

  EntityRawPtrArray Scene::Filter(std::function<bool(Entity*)> filter)
  {
    EntityRawPtrArray filtered;
    std::copy_if
    (
      m_entities.begin(),
      m_entities.end(),
      std::back_inserter(filtered),
      filter
    );
    return filtered;
  }

  void Scene::Destroy(bool removeResources)
  {
    for (Entity* ntt : m_entities)
    {
      if (removeResources)
      {
        ntt->RemoveResources();
      }

      SafeDel(ntt);
    }
    m_entities.clear();

    m_loaded = false;
    m_initiated = false;
  }

  void Scene::SavePrefab(Entity* entity)
  {
    // Assign a default node.
    Node* prevNode = entity->m_node;
    entity->m_node = new Node();
    entity->m_node->m_children = prevNode->m_children;

    // Construct prefab.
    Scene prefab;
    prefab.AddEntity(entity);
    GetChildren(entity, prefab.m_entities);
    String name = entity->GetNameVal() + SCENE;
    prefab.SetFile(PrefabPath(name));
    prefab.m_name = name;
    prefab.Save(false);
    prefab.m_entities.clear();

    // Restore the old node.
    entity->m_node->m_children.clear();
    SafeDel(entity->m_node);
    entity->m_node = prevNode;
  }

  void Scene::ClearEntities()
  {
    m_entities.clear();
  }

  void Scene::CopyTo(Resource* other)
  {
    Resource::CopyTo(other);
    Scene* cpy = static_cast<Scene*> (other);
    cpy->m_name = m_name + "_cpy";

    cpy->m_entities.reserve(m_entities.size());
    EntityRawPtrArray roots;
    GetRootEntities(m_entities, roots);

    for (Entity* ntt : roots)
    {
      DeepCopy(ntt, cpy->m_entities);
    }
  }

  void Scene::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* scene = CreateXmlNode(doc, XmlSceneElement, parent);

    // Match scene name with saved file.
    String name;
    DecomposePath(GetFile(), nullptr, &name, nullptr);
    // Always write the current version.
    WriteAttr(scene, doc, "version", TKVersionStr);
    WriteAttr(scene, doc, "name", name.c_str());

    for (Entity* ntt : m_entities)
    {
      ntt->Serialize(doc, scene);
    }
  }

  void Scene::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    XmlNode* root = nullptr;
    if (parent != nullptr)
    {
      root = parent->first_node(XmlSceneElement.c_str());
    }
    else
    {
      root = doc->first_node(XmlSceneElement.c_str());
    }

    // Match scene name with file name.
    String path = GetFile();
    NormalizePath(path);

    DecomposePath(path, nullptr, &m_name, nullptr);
    ReadAttr(root, "version", m_version);

    ULongID lastID = GetHandleManager()->GetNextHandle();
    ULongID biggestID = 0;
    XmlNode* node = nullptr;
    for
    (
      node = root->first_node(XmlEntityElement.c_str());
      node;
      node = node->next_sibling(XmlEntityElement.c_str())
    )
    {
      XmlAttribute* typeAttr = node->first_attribute
      (
        XmlEntityTypeAttr.c_str()
      );
      EntityType t = (EntityType)std::atoi(typeAttr->value());
      Entity* ntt = Entity::CreateByType(t);

      ntt->DeSerialize(doc, node);
      // Incrementing the incoming ntt ids with current max id value...
      //   to prevent id collisions.
      ULongID currentID = ntt->GetIdVal() + lastID;
      biggestID = glm::max(biggestID, currentID);
      ntt->SetIdVal(currentID);
      ntt->_parentId = ntt->_parentId + lastID;
      m_entities.push_back(ntt);
    }
    GetHandleManager()->SetMaxHandle(biggestID);
  }

  ULongID Scene::GetBiggestEntityId()
  {
    ULongID lastId = 0;
    for (Entity* ntt : m_entities)
    {
      lastId = glm::max(lastId, ntt->GetIdVal());
    }

    return lastId;
  }

  SceneManager::SceneManager()
  {
    m_type = ResourceType::Scene;
  }

  SceneManager::~SceneManager()
  {
  }

  void SceneManager::Init()
  {
    m_currentScene = nullptr;
    ResourceManager::Init();
  }

  void SceneManager::Uninit()
  {
    m_currentScene = nullptr;
    ResourceManager::Uninit();
  }

  bool SceneManager::CanStore(ResourceType t)
  {
    return t == ResourceType::Scene;
  }

  ResourcePtr SceneManager::CreateLocal(ResourceType type)
  {
    return ResourcePtr(new Scene());
  }

  String SceneManager::GetDefaultResource(ResourceType type)
  {
    return ScenePath("Sample.scene", true);
  }

  ScenePtr SceneManager::GetCurrentScene()
  {
    return m_currentScene;
  }

  void SceneManager::SetCurrentScene(const ScenePtr& scene)
  {
    m_currentScene = scene;
  }

}  // namespace ToolKit
