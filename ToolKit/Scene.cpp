/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Scene.h"

#include "AABBOverrideComponent.h"
#include "Component.h"
#include "EngineSettings.h"
#include "EnvironmentComponent.h"
#include "FileManager.h"
#include "Logger.h"
#include "MathUtil.h"
#include "Mesh.h"
#include "Prefab.h"
#include "ToolKit.h"
#include "Util.h"

namespace ToolKit
{
  TKDefineClass(Scene, Resource);

  Scene::Scene() { m_name = "New Scene"; }

  Scene::~Scene() { Destroy(false); }

  void Scene::NativeConstruct() { Super::NativeConstruct(); }

  void Scene::NativeConstruct(const String& file)
  {
    NativeConstruct();
    SetFile(file);
  }

  void Scene::Load()
  {
    if (!m_loaded)
    {
      String path = GetFile();
      m_isPrefab  = path.find("Prefabs") != String::npos;

      ParseDocument(XmlSceneElement);

      m_loaded = true;
    }
  }

  void Scene::Save(bool onlyIfDirty)
  {
    // get post processing settings
    m_postProcessSettings = GetEngineSettings().PostProcessing;

    String fullPath       = GetFile();
    if (fullPath.empty())
    {
      fullPath = ScenePath(m_name + SCENE);
    }

    String path;
    DecomposePath(fullPath, &path, nullptr, nullptr);

    std::error_code err;
    std::filesystem::create_directories(path, err);

    if (err)
    {
      TK_ERR("Save scene failed: %s", err.message().c_str());
      return;
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
    else
    {
      TK_ERR("Save scene failed. File %s can't be opened.", fullPath.c_str());
    }
  }

  void Scene::Init(bool flushClientSideArray)
  {
    if (m_initiated)
    {
      return;
    }

    PrefabRawPtrArray prefabs;
    const EntityPtrArray& ntties = GetEntities();
    for (EntityPtr ntt : ntties)
    {
      if (SkyBase* sky = ntt->As<SkyBase>())
      {
        sky->Init();
      }
      else if (Prefab* prefab = ntt->As<Prefab>())
      {
        prefab->Init(Self<Scene>());
        prefabs.push_back(prefab);
      }
      else if (ntt->IsDrawable())
      {
        // Mesh component.
        bool containsSkinMesh = false;
        MeshComponentPtrArray meshes;
        if (MeshComponentPtr meshComp = ntt->GetComponent<MeshComponent>())
        {
          meshComp->Init(flushClientSideArray);
          if (meshComp->GetMeshVal()->IsSkinned())
          {
            containsSkinMesh = true;
          }
        }

        if (containsSkinMesh)
        {
          if (ntt->GetComponent<AABBOverrideComponent>() == nullptr)
          {
            AABBOverrideComponentPtr aabbOverride = MakeNewPtr<AABBOverrideComponent>();
            ntt->AddComponent(aabbOverride);
            aabbOverride->SetBoundingBox(ntt->GetBoundingBox());
          }
        }

        // Environment component.
        if (EnvironmentComponentPtr envCom = ntt->GetComponent<EnvironmentComponent>())
        {
          envCom->Init(true);
        }
      }
    }

    // Link all prefabs.
    for (Prefab* prefab : prefabs)
    {
      prefab->Link();
    }

    m_initiated = true;
  }

  void Scene::UnInit() { Destroy(false); }

  void Scene::Update(float deltaTime)
  {
    m_environmentVolumeCache.clear();

    for (const EntityPtr& ntt : m_entities)
    {
      // Update volume caches.
      if (const EnvironmentComponentPtr& envComp = ntt->GetComponent<EnvironmentComponent>())
      {
        if (envComp->GetHdriVal() != nullptr && envComp->GetIlluminateVal())
        {
          envComp->Init(true);
          m_environmentVolumeCache.push_back(envComp);
        }
      }
    }

    for (Light* light : m_lightCache)
    {
      light->UpdateShadowCamera();
    }
  }

  void Scene::Merge(ScenePtr other)
  {
    HandleManager* handleMan = GetHandleManager();
    for (EntityPtr otherNtt : other->GetEntities())
    {
      otherNtt->SetIdVal(handleMan->GenerateHandle());
      AddEntity(otherNtt);
    }

    other->RemoveAllEntities();
    GetSceneManager()->Remove(other->GetFile());
  }

  Scene::PickData Scene::PickObject(const Ray& ray, const IDArray& ignoreList, const EntityPtrArray& extraList)
  {
    PickData pd;
    pd.pickPos                  = ray.position + ray.direction * 5.0f;

    float closestPickedDistance = TK_FLT_MAX;

    auto pickFn                 = [&](const EntityPtrArray& entities) -> void
    {
      for (EntityPtr ntt : entities)
      {
        if (!ntt->IsDrawable())
        {
          continue;
        }

        if (contains(ignoreList, ntt->GetIdVal()))
        {
          continue;
        }

        float dist = TK_FLT_MAX;
        if (RayEntityIntersection(ray, ntt, dist))
        {
          if (dist < closestPickedDistance && dist > 0.0f)
          {
            pd.entity             = ntt;
            pd.pickPos            = ray.position + ray.direction * dist;
            closestPickedDistance = dist;
          }
        }
      }
    };

    pickFn(extraList);

    float dist          = TK_FLT_MAX;
    EntityPtr pickedNtt = m_aabbTree.RayQuery(ray, true, &dist, ignoreList);

    if (dist < closestPickedDistance)
    {
      pd.entity  = pickedNtt;
      pd.pickPos = PointOnRay(ray, dist);
    }

    return pd;
  }

  void Scene::PickObject(const Frustum& frustum,
                         PickDataArray& pickedObjects,
                         const IDArray& ignoreList,
                         const EntityPtrArray& extraList)
  {
    // Create pick data for the  entities.
    auto pickFn = [&](const EntityPtrArray& entities, bool skipTest) -> void
    {
      for (EntityPtr ntt : entities)
      {
        assert(ntt != nullptr);

        if (!ntt->IsDrawable())
        {
          continue;
        }

        if (contains(ignoreList, ntt->GetIdVal()))
        {
          continue;
        }

        IntersectResult res    = IntersectResult::Inside;
        const BoundingBox& box = ntt->GetBoundingBox(true);

        if (!skipTest)
        {
          // If frustum test is applied via aabb tree this check can be skipped.
          res = FrustumBoxIntersection(frustum, box);
        }

        if (res != IntersectResult::Outside)
        {
          PickData pd;
          pd.pickPos = (box.max + box.min) * 0.5f;
          pd.entity  = ntt;

          pickedObjects.push_back(pd);
        }
      }
    };

    pickFn(extraList, false); // Test extra list with frustum.

    EntityRawPtrArray entitiesInTheFrustum = m_aabbTree.VolumeQuery(frustum);
    pickFn(ToEntityPtrArray(entitiesInTheFrustum), true); // Skip the frustum test.
  }

  EntityPtr Scene::GetEntity(ULongID id, int* index) const
  {
    for (int i = 0; i < (int) m_entities.size(); i++)
    {
      EntityPtr ntt = m_entities[i];
      if (ntt->GetIdVal() == id)
      {
        if (index != nullptr)
        {
          *index = i;
        }

        return ntt;
      }
    }

    if (index != nullptr)
    {
      *index = -1;
    }

    return nullptr;
  }

  void Scene::AddEntity(EntityPtr entity, int index)
  {
    if (entity != nullptr)
    {
      bool isUnique = GetEntity(entity->GetIdVal()) == nullptr;
      assert(isUnique);

      if (isUnique)
      {
        if (m_loaded)
        {
          // Don't link prefabs if the scene is in loading phase.
          // Id conflicts may occur. Linking for prefabs separately handed on load.
          if (Prefab* prefab = entity->As<Prefab>())
          {
            prefab->Link();
          }
        }

        UpdateEntityCaches(entity, true);

        if (index < 0 || index >= (int) m_entities.size())
        {
          m_entities.push_back(entity);
        }
        else
        {
          m_entities.insert(m_entities.begin() + index, entity);
        }

        entity->m_scene = Self<Scene>();

        if (entity->m_partOfAABBTree)
        {
          m_aabbTree.CreateNode(entity, entity->GetBoundingBox(true));
        }
      }
    }
  }

  void Scene::_RemoveChildren(EntityPtr removed)
  {
    NodeRawPtrArray& children = removed->m_node->m_children;

    for (Node* child : children)
    {
      if (EntityPtr childNtt = child->OwnerEntity())
      {
        RemoveEntity(childNtt->GetIdVal());
      }
    }
  }

  EntityPtr Scene::RemoveEntity(ULongID id, bool deep)
  {
    if (m_entities.empty())
    {
      return nullptr;
    }

    int indx          = -1;
    EntityPtr removed = GetEntity(id, &indx);
    if (removed == nullptr)
    {
      return nullptr;
    }

    if (Prefab* prefab = removed->As<Prefab>())
    {
      prefab->Unlink(); // This operation may alter the removed index.
      indx              = -1;
      EntityPtr removed = GetEntity(id, &indx);
    }

    UpdateEntityCaches(removed, false);
    m_entities.erase(m_entities.begin() + indx);

    if (deep)
    {
      _RemoveChildren(removed);
    }
    else
    {
      removed->m_node->OrphanAllChildren(true);
    }

    if (removed->m_aabbTreeNodeProxy != AABBTree::nullNode)
    {
      m_aabbTree.RemoveNode(removed->m_aabbTreeNodeProxy);
      removed->m_scene.reset();
    }

    return removed;
  }

  void Scene::RemoveEntity(const EntityPtrArray& entities, bool deep)
  {
    for (size_t i = 0; i < entities.size(); i++)
    {
      RemoveEntity(entities[i]->GetIdVal(), deep);
    }
  }

  void Scene::RemoveAllEntities() { m_entities.clear(); }

  const EntityPtrArray& Scene::GetEntities() const { return m_entities; }

  const LightRawPtrArray& Scene::GetLights() const { return m_lightCache; }

  const LightRawPtrArray& Scene::GetDirectionalLights() const { return m_directionalLightCache; }

  SkyBasePtr& Scene::GetSky() { return m_skyCache; }

  EnvironmentComponentPtrArray& Scene::GetEnvironmentVolumes() const { return m_environmentVolumeCache; }

  EntityPtr Scene::GetFirstByName(const String& name)
  {
    for (EntityPtr ntt : m_entities)
    {
      if (ntt->GetNameVal() == name)
      {
        return ntt;
      }
    }
    return nullptr;
  }

  EntityPtrArray Scene::GetByTag(const String& tag)
  {
    EntityPtrArray arrayByTag;
    for (EntityPtr ntt : m_entities)
    {
      StringArray tokens;
      Split(ntt->GetTagVal(), ".", tokens);

      for (const String& token : tokens)
      {
        if (token == tag)
        {
          arrayByTag.push_back(ntt);
        }
      }
    }

    return arrayByTag;
  }

  EntityPtr Scene::GetFirstByTag(const String& tag)
  {
    EntityPtrArray res = GetByTag(tag);
    return res.empty() ? nullptr : res.front();
  }

  EntityPtrArray Scene::Filter(std::function<bool(EntityPtr)> filter)
  {
    EntityPtrArray filtered;
    std::copy_if(m_entities.begin(), m_entities.end(), std::back_inserter(filtered), filter);
    return filtered;
  }

  void Scene::LinkPrefab(const String& fullPath)
  {
    if (fullPath == GetFile())
    {
      TK_ERR("You can't prefab same scene.");
      return;
    }

    String path = GetRelativeResourcePath(fullPath);
    // Check if file is from Prefab folder
    {
      String folder     = fullPath.substr(0, fullPath.length() - path.length());
      String prefabPath = PrefabPath("");
      if (folder != PrefabPath(""))
      {
        TK_ERR("You can't use a prefab outside of Prefab folder.");
        return;
      }
    }

    PrefabPtr prefab = MakeNewPtr<Prefab>();
    prefab->SetPrefabPathVal(path);
    prefab->Load();
    prefab->Init(Self<Scene>());

    AddEntity(prefab);
  }

  void Scene::Destroy(bool removeResources)
  {
    PrefabRawPtrArray prefabs;
    for (EntityPtr ntt : m_entities)
    {
      if (Prefab* prefab = ntt->As<Prefab>())
      {
        prefabs.push_back(prefab);
      }
    }

    for (Prefab* prefab : prefabs)
    {
      prefab->UnInit();
    }

    int maxCnt = (int) m_entities.size() - 1;

    for (int i = maxCnt; i >= 0; i--)
    {
      EntityPtr ntt = m_entities[i];

      if (removeResources)
      {
        ntt->RemoveResources();
      }
    }

    m_entities.clear();
    m_aabbTree.Reset();

    m_lightCache.clear();
    m_directionalLightCache.clear();
    m_environmentVolumeCache.clear();
    m_skyCache  = nullptr;

    m_loaded    = false;
    m_initiated = false;
  }

  void Scene::SavePrefab(EntityPtr entity, String name, String path)
  {
    // Assign a default node.
    Node* prevNode             = entity->m_node;
    entity->m_node             = new Node();
    entity->m_node->m_children = prevNode->m_children;

    // Construct prefab.
    ScenePtr prefab            = MakeNewPtr<Scene>();
    prefab->AddEntity(entity);
    GetChildren(entity, prefab->m_entities);
    String prefabName = name.empty() ? entity->GetNameVal() + SCENE : name + SCENE;
    String prefabPath = path.empty() ? prefabName : ConcatPaths({path, prefabName});
    String fullPath   = PrefabPath(prefabPath);
    prefab->SetFile(fullPath);
    prefab->m_name = name;
    prefab->Save(false);
    prefab->m_entities.clear();

    // Restore the old node.
    entity->m_node->m_children.clear();
    SafeDel(entity->m_node);
    entity->m_node = prevNode;
  }

  void Scene::ClearEntities()
  {
    m_aabbTree.Reset();
    m_entities.clear();
  }

  const BoundingBox& Scene::GetSceneBoundary() { return m_aabbTree.GetRootBoundingBox(); }

  void Scene::CopyTo(Resource* other)
  {
    Resource::CopyTo(other);
    Scene* cpy  = static_cast<Scene*>(other);
    cpy->m_name = m_name + "_cpy";

    cpy->m_entities.reserve(m_entities.size());
    EntityPtrArray roots;
    GetRootEntities(m_entities, roots);

    for (EntityPtr ntt : roots)
    {
      DeepCopy(ntt, cpy->m_entities);
    }
  }

  void Scene::UpdateEntityCaches(const EntityPtr& ntt, bool add)
  {
    if (SkyBasePtr sky = SafeCast<SkyBase>(ntt))
    {
      if (add)
      {
        m_skyCache = sky;
      }
      else
      {
        m_skyCache = nullptr;
      }
    }
    else if (Light* light = ntt->As<Light>())
    {
      if (add)
      {
        m_lightCache.push_back(light);
        if (light->GetLightType() == Light::LightType::Directional)
        {
          m_directionalLightCache.push_back(light);
        }
      }
      else
      {
        remove(m_lightCache, light);
        if (light->GetLightType() == Light::LightType::Directional)
        {
          remove(m_directionalLightCache, light);
        }
      }
    }

    if (const EnvironmentComponentPtr& envComp = ntt->GetComponent<EnvironmentComponent>())
    {
      if (envComp->GetHdriVal() != nullptr && envComp->GetIlluminateVal())
      {
        if (add)
        {
          m_environmentVolumeCache.push_back(envComp);
        }
        else
        {
          remove(m_environmentVolumeCache, envComp);
        }
      }
    }
  }

  XmlNode* Scene::SerializeImp(XmlDocument* doc, XmlNode* parent) const
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
      EntityPtr ntt = m_entities[listIndx];

      // If entity isn't a prefab type but from a prefab, don't serialize it
      if (!ntt->IsA<Prefab>() && Prefab::GetPrefabRoot(ntt))
      {
        continue;
      }

      ntt->Serialize(doc, scene);
    }

    if (!m_isPrefab)
    {
      m_postProcessSettings.Serialize(doc, scene);
    }

    return scene;
  }

  XmlNode* Scene::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    // Match scene name with file name.
    String path = GetSerializeFile();
    DecomposePath(path, nullptr, &m_name, nullptr);

    if (m_version >= TKV045)
    {
      DeSerializeImpV045(info, parent);
      return nullptr;
    }

    // For old type of scenes, load the entities and find the parent-child relations
    const char* xmlRootObject = XmlEntityElement.c_str();
    const char* xmlObjectType = XmlEntityTypeAttr.c_str();

    EntityPtrArray prefabList;
    EntityPtrArray deserializedEntities;

    for (XmlNode* node = parent->first_node(xmlRootObject); node; node = node->next_sibling(xmlRootObject))
    {
      XmlAttribute* typeAttr      = node->first_attribute(xmlObjectType);
      EntityFactory::EntityType t = (EntityFactory::EntityType) std::atoi(typeAttr->value());
      EntityPtr ntt               = EntityFactory::CreateByType(t);
      ntt->m_version              = m_version;

      ntt->DeSerialize(info, node);

      if (ntt->IsA<Prefab>())
      {
        prefabList.push_back(ntt);
      }

      // Old file id trick.
      ULongID id = 0;
      ReadAttr(node, XmlEntityIdAttr, id);

      // Temporary id. This will be regenerated later. Do not use this id until it is regenerated later.
      ntt->SetIdVal(id);
      deserializedEntities.push_back(ntt);
    }

    // Solve the parent-child relations
    for (EntityPtr ntt : deserializedEntities)
    {
      for (EntityPtr parentCandidate : deserializedEntities)
      {
        if (parentCandidate->GetIdVal() == ntt->_parentId)
        {
          parentCandidate->m_node->AddChild(ntt->m_node);
          break;
        }
      }
    }

    // Old file id trick.
    // Regenerate ids and add to scene
    // Solve the parent-child relations
    for (EntityPtr ntt : deserializedEntities)
    {
      // Generate new handle for old version scene entities
      ntt->SetIdVal(GetHandleManager()->GenerateHandle());
      AddEntity(ntt);
    }

    // Do not serialize post processing settings if this is prefab.
    if (!m_isPrefab)
    {
      m_postProcessSettings.DeSerialize(info.Document, parent);
    }

    for (EntityPtr ntt : prefabList)
    {
      Prefab* prefab = static_cast<Prefab*>(ntt.get());
      prefab->Init(Self<Scene>());
      prefab->Link();
    }

    return nullptr;
  }

  void Scene::DeSerializeImpV045(const SerializationFileInfo& info, XmlNode* parent)
  {
    XmlNode* root = info.Document->first_node(XmlSceneElement.c_str());
    XmlNode* node = nullptr;

    EntityPtrArray prefabList;
    EntityPtrArray deserializedEntities;

    const char* xmlRootObject = Object::StaticClass()->Name.c_str();
    const char* xmlObjectType = XmlObjectClassAttr.data();

    for (node = root->first_node(xmlRootObject); node; node = node->next_sibling(xmlRootObject))
    {
      XmlAttribute* typeAttr = node->first_attribute(xmlObjectType);
      EntityPtr ntt          = MakeNewPtrCasted<Entity>(typeAttr->value());

      ntt->DeSerialize(info, node);

      if (Prefab* prefab = ntt->As<Prefab>())
      {
        prefab->Load();
        prefabList.push_back(ntt);
      }

      deserializedEntities.push_back(ntt);
    }

    // Solve the parent-child relations
    m_entities.reserve(deserializedEntities.size());

    for (EntityPtr ntt : deserializedEntities)
    {
      if (ntt->_parentId == NULL_HANDLE)
      {
        AddEntity(ntt);
        continue;
      }

      for (EntityPtr parentCandidate : deserializedEntities)
      {
        ULongID id = parentCandidate->_idBeforeCollision;
        if (id == NULL_HANDLE)
        {
          id = parentCandidate->GetIdVal();
        }

        if (ntt->_parentId == id)
        {
          parentCandidate->m_node->AddChild(ntt->m_node);
          break;
        }
      }

      AddEntity(ntt);
    }

    m_postProcessSettings.DeSerialize(info.Document, parent);
  }

  ULongID Scene::GetBiggestEntityId()
  {
    ULongID lastId = 0;
    for (EntityPtr ntt : m_entities)
    {
      lastId = glm::max(lastId, ntt->GetIdVal());
    }

    return lastId;
  }

  SceneManager::SceneManager() { m_baseType = Scene::StaticClass(); }

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

  bool SceneManager::CanStore(ClassMeta* Class) { return Class == Scene::StaticClass(); }

  String SceneManager::GetDefaultResource(ClassMeta* Class) { return ScenePath("Sample.scene", true); }

  ScenePtr SceneManager::GetCurrentScene() { return m_currentScene; }

  void SceneManager::SetCurrentScene(const ScenePtr& scene)
  {
    m_currentScene                     = scene;

    // Apply scene post processing effects.
    GetEngineSettings().PostProcessing = m_currentScene->m_postProcessSettings;
  }

} // namespace ToolKit
