#include "Scene.h"

#include "Component.h"
#include "EnvironmentComponent.h"
#include "Prefab.h"
#include "ResourceComponent.h"
#include "ToolKit.h"
#include "Util.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "DebugNew.h"

namespace ToolKit
{

  Scene::Scene() { m_name = "New Scene"; }

  Scene::Scene(String file) : Scene() { SetFile(file); }

  Scene::~Scene() { Destroy(false); }

  void Scene::Load()
  {
    if (m_loaded)
    {
      return;
    }

    String path = GetFile();
    NormalizePath(path);
    XmlFilePtr sceneFile = GetFileManager()->GetXmlFile(path);
    XmlDocument sceneDoc;
    sceneDoc.parse<0>(sceneFile->data());
    m_isPrefab = path.find("Prefabs") != String::npos;

    DeSerialize(&sceneDoc, nullptr);
    PostDeSerialize();

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
      if (ntt->GetType() == EntityType::Entity_Sky)
      {
        static_cast<Sky*>(ntt)->Init();
      }
      else if (ntt->IsDrawable())
      {
        // Mesh component
        MeshComponentPtrArray meshes;
        ntt->GetComponent<MeshComponent>(meshes);
        for (MeshComponentPtr& mesh : meshes)
        {
          mesh->Init(flushClientSideArray);
        }

        // Environment component
        EnvironmentComponentPtr envCom =
            ntt->GetComponent<EnvironmentComponent>();
        if (envCom != nullptr)
        {
          envCom->Init(true);
        }
      }
    }

    m_initiated = true;
  }

  void Scene::UnInit() { Destroy(false); }

  void Scene::Update(float deltaTime) {}

  void Scene::Merge(ScenePtr other)
  {
    ULongID lastID = GetHandleManager()->GetNextHandle(), biggestID = 0;
    const EntityRawPtrArray& entities = other->GetEntities();
    for (Entity* ntt : entities)
    {
      AddEntity(ntt); // Insert into this scene.
    }
    GetHandleManager()->SetMaxHandle(biggestID);

    other->RemoveAllEntities();
    GetSceneManager()->Remove(other->GetFile());
  }

  Scene::PickData Scene::PickObject(Ray ray,
                                    const EntityIdArray& ignoreList,
                                    const EntityRawPtrArray& extraList)
  {
    PickData pd;
    pd.pickPos                  = ray.position + ray.direction * 5.0f;

    float closestPickedDistance = FLT_MAX;

    auto pickFn = [&ignoreList, &ray, &pd, &closestPickedDistance](
                      const EntityRawPtrArray& entities) -> void
    {
      for (Entity* ntt : entities)
      {
        if (!ntt->IsDrawable())
        {
          continue;
        }

        if (std::find(ignoreList.begin(), ignoreList.end(), ntt->GetIdVal()) !=
            ignoreList.end())
        {
          continue;
        }

        Ray rayInObjectSpace = ray;
        Mat4 ts  = ntt->m_node->GetTransform(TransformationSpace::TS_WORLD);
        Mat4 its = glm::inverse(ts);
        rayInObjectSpace.position  = its * Vec4(ray.position, 1.0f);
        rayInObjectSpace.direction = its * Vec4(ray.direction, 0.0f);

        float dist                 = 0;
        if (RayBoxIntersection(rayInObjectSpace, ntt->GetAABB(), dist))
        {
          bool hit         = false;

          float t          = TK_FLT_MAX;
          uint submeshIndx = FindMeshIntersection(ntt, ray, t);

          // There was no tracing possible object, so hit should be true
          if (t == 0.0f && submeshIndx == TK_UINT_MAX)
          {
            hit = true;
          }
          else if (t != TK_FLT_MAX && submeshIndx != TK_UINT_MAX)
          {
            hit = true;
          }

          if (hit)
          {
            if (dist < closestPickedDistance && dist > 0.0f)
            {
              pd.entity             = ntt;
              pd.pickPos            = ray.position + ray.direction * dist;
              closestPickedDistance = dist;
            }
          }
        }
      }
    };

    pickFn(m_entities);
    pickFn(extraList);

    return pd;
  }

  void Scene::PickObject(const Frustum& frustum,
                         std::vector<PickData>& pickedObjects,
                         const EntityIdArray& ignoreList,
                         const EntityRawPtrArray& extraList,
                         bool pickPartiallyInside)
  {
    auto pickFn = [&frustum, &pickedObjects, &ignoreList, &pickPartiallyInside](
                      const EntityRawPtrArray& entities) -> void
    {
      for (Entity* e : entities)
      {
        if (!e->IsDrawable())
        {
          continue;
        }

        if (std::find(ignoreList.begin(), ignoreList.end(), e->GetIdVal()) !=
            ignoreList.end())
        {
          continue;
        }

        BoundingBox bb      = e->GetAABB(true);
        IntersectResult res = FrustumBoxIntersection(frustum, bb);
        if (res != IntersectResult::Outside)
        {
          PickData pd;
          pd.pickPos = (bb.max + bb.min) * 0.5f;
          pd.entity  = e;

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
    };

    pickFn(m_entities);
    pickFn(extraList);
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
    if (entity)
    {
      bool isUnique = GetEntity(entity->GetIdVal()) == nullptr;
      assert(isUnique);
      if (isUnique)
      {
        m_entities.push_back(entity);
      }
    }
  }

  EntityRawPtrArray& Scene::AccessEntityArray() { return m_entities; }

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
    m_entities.erase(std::remove_if(m_entities.begin(),
                                    m_entities.end(),
                                    [&entities](const Entity* ntt) -> bool
                                    {
                                      for (const Entity* removeNtt : entities)
                                      {
                                        if (removeNtt->GetIdVal() ==
                                            ntt->GetIdVal())
                                        {
                                          return true;
                                        }
                                      }

                                      return false;
                                    }),
                     m_entities.end());
  }

  void Scene::RemoveAllEntities() { m_entities.clear(); }

  const EntityRawPtrArray& Scene::GetEntities() const { return m_entities; }

  LightRawPtrArray Scene::GetLights() const
  {
    LightRawPtrArray lights;
    for (Entity* ntt : m_entities)
    {
      if (ntt->IsLightInstance())
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
    std::copy_if(m_entities.begin(),
                 m_entities.end(),
                 std::back_inserter(filtered),
                 filter);
    return filtered;
  }

  // Returns the last sky added
  SkyBase* Scene::GetSky()
  {
    for (int i = static_cast<int>(m_entities.size()) - 1; i >= 0; --i)
    {
      if (m_entities[i]->IsSkyInstance())
      {
        return static_cast<SkyBase*>(m_entities[i]);
      }
    }

    return nullptr;
  }

  void Scene::LinkPrefab(const String& fullPath)
  {
    if (fullPath == GetFile())
    {
      GetLogger()->WriteConsole(LogType::Error, "You can't prefab same scene!");
      return;
    }

    String path = GetRelativeResourcePath(fullPath);
    // Check if file is from Prefab folder
    {
      String folder     = fullPath.substr(0, fullPath.length() - path.length());
      String prefabPath = PrefabPath("");
      if (folder != PrefabPath(""))
      {
        GetLogger()->WriteConsole(
            LogType::Error,
            "You can't use a prefab outside of Prefab folder!");
        return;
      }
      String folderName =
          folder.substr(folder.find_last_of(GetPathSeparator()));
      // if (folderName != GetResourcePath())
    }
    Prefab* prefab = new Prefab();
    AddEntity(prefab);
    prefab->SetPrefabPathVal(path);
    prefab->Init(this);
  }

  EntityRawPtrArray Scene::GetEnvironmentLightEntities()
  {
    EntityRawPtrArray envLightNtties;
    envLightNtties.clear();
    for (Entity* ntt : envLightNtties)
    {
      if (ntt->GetType() == EntityType::Entity_Sky)
      {
        continue;
      }

      if (EnvironmentComponentPtr envCom =
              ntt->GetComponent<EnvironmentComponent>())
      {
        if (envCom->GetIlluminateVal())
        {
          if (HdriPtr hdr = envCom->GetHdriVal())
          {
            if (hdr->IsTextureAssigned())
            {
              envCom->Init(true);
              envLightNtties.push_back(ntt);
            }
          }
        }
      }
    }

    return envLightNtties;
  }

  void Scene::Destroy(bool removeResources)
  {
    EntityRawPtrArray prefabs;
    for (Entity* ntt : m_entities)
    {
      if (ntt->GetType() == EntityType::Entity_Prefab)
      {
        prefabs.push_back(ntt);
      }
    }

    for (Entity* ntt : prefabs)
    {
      Prefab* prefab = static_cast<Prefab*>(ntt);
      prefab->UnInit();
    }

    int maxCnt = (int) m_entities.size() - 1;

    for (int i = maxCnt; i >= 0; i--)
    {
      Entity* ntt = m_entities[i];
      if (removeResources)
      {
        ntt->RemoveResources();
      }
      SafeDel(m_entities[i]);
    }

    m_entities.clear();

    m_loaded    = false;
    m_initiated = false;
  }

  void Scene::SavePrefab(Entity* entity)
  {
    // Assign a default node.
    Node* prevNode             = entity->m_node;
    entity->m_node             = new Node();
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

  void Scene::ClearEntities() { m_entities.clear(); }

  void Scene::CopyTo(Resource* other)
  {
    Resource::CopyTo(other);
    Scene* cpy  = static_cast<Scene*>(other);
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

    for (size_t listIndx = 0; listIndx < m_entities.size(); listIndx++)
    {
      Entity* ntt = m_entities[listIndx];
      // If entity isn't a prefab type but from a prefab, don't serialize it
      if (ntt->GetType() != EntityType::Entity_Prefab &&
          Prefab::GetPrefabRoot(ntt))
      {
        continue;
      }
      ntt->Serialize(doc, scene);

      NormalizeEntityID(doc,
                        scene->last_node(XmlEntityElement.c_str()),
                        listIndx);
    }

    if (!m_isPrefab)
    {
      GetEngineSettings().SerializePostProcessing(doc, nullptr);
    }
  }

  void Scene::NormalizeEntityID(XmlDocument* doc,
                                XmlNode* parent,
                                size_t listIndx) const
  {
    XmlAttribute* parentAttrib =
        parent->first_attribute(XmlParentEntityIdAttr.c_str());
    XmlAttribute* baseAttrib =
        parent->first_attribute(XmlBaseEntityIdAttr.c_str());
    parent->remove_attribute(parent->first_attribute(XmlEntityIdAttr.c_str()));
    WriteAttr(parent, doc, XmlEntityIdAttr, std::to_string(listIndx + 1));

    Entity* ntt      = m_entities[listIndx];
    Node* parentNode = ntt->m_node->m_parent;
    // If parent is in scene, save its list index too
    if (parentNode && parentNode->m_entity)
    {
      for (uint parentSrchIndx = 0; parentSrchIndx < m_entities.size();
           parentSrchIndx++)
      {
        if (parentNode->m_entity == m_entities[parentSrchIndx])
        {
          parent->remove_attribute(parentAttrib);
          WriteAttr(parent,
                    doc,
                    XmlParentEntityIdAttr,
                    std::to_string(parentSrchIndx + 1));
        }
      }
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

    ULongID lastID    = GetHandleManager()->GetNextHandle();
    ULongID biggestID = 0;
    XmlNode* node     = nullptr;

    EntityRawPtrArray prefabList;

    for (node = root->first_node(XmlEntityElement.c_str()); node;
         node = node->next_sibling(XmlEntityElement.c_str()))
    {
      XmlAttribute* typeAttr = node->first_attribute(XmlEntityTypeAttr.c_str());
      EntityType t           = (EntityType) std::atoi(typeAttr->value());
      Entity* ntt            = GetEntityFactory()->CreateByType(t);

      ntt->DeSerialize(doc, node);
      ntt->PostDeSerialize();

      if (ntt->GetType() == EntityType::Entity_Prefab)
      {
        prefabList.push_back(ntt);
      }

      // Incrementing the incoming ntt ids with current max id value...
      //   to prevent id collisions.
      ULongID listIndx = 0;
      ReadAttr(node, XmlEntityIdAttr, listIndx);
      ULongID currentID = listIndx + lastID;
      biggestID         = glm::max(biggestID, currentID);
      ntt->SetIdVal(currentID);
      ntt->_parentId += lastID;

      AddEntity(ntt);
    }
    GetHandleManager()->SetMaxHandle(biggestID);
    // do not serialize post processing settings if this is prefab
    if (!m_isPrefab)
    {
      GetEngineSettings().DeSerializePostProcessing(doc, parent);
    }

    for (Entity* prefab : prefabList)
    {
      static_cast<Prefab*>(prefab)->Init(this);
    }
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

  SceneManager::SceneManager() { m_type = ResourceType::Scene; }

  SceneManager::~SceneManager() {}

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

  ScenePtr SceneManager::GetCurrentScene() { return m_currentScene; }

  void SceneManager::SetCurrentScene(const ScenePtr& scene)
  {
    m_currentScene = scene;
  }

} // namespace ToolKit
