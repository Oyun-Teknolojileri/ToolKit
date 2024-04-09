/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Scene.h"

#include "BVH.h"
#include "Component.h"
#include "EngineSettings.h"
#include "EnvironmentComponent.h"
#include "FileManager.h"
#include "Logger.h"
#include "MathUtil.h"
#include "Mesh.h"
#include "Prefab.h"
#include "ResourceComponent.h"
#include "ToolKit.h"
#include "Util.h"

#include "DebugNew.h"

namespace ToolKit
{
  TKDefineClass(Scene, Resource);

  Scene::Scene()
  {
    m_name = "New Scene";
    m_bvh  = MakeNewPtr<BVH>(this);
  }

  Scene::Scene(const String& file) : Scene() { SetFile(file); }

  Scene::~Scene()
  {
    Destroy(false);
    m_bvh = nullptr;
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

    m_boundingBox                = BoundingBox();
    const EntityPtrArray& ntties = GetEntities();
    for (EntityPtr ntt : ntties)
    {
      if (SkyBase* sky = ntt->As<SkyBase>())
      {
        sky->Init();
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

      BoundingBox worldBox = ntt->GetBoundingBox(true);
      m_boundingBox.UpdateBoundary(worldBox);
    }

    m_initiated = true;
  }

  void Scene::UnInit() { Destroy(false); }

  void Scene::Update(float deltaTime)
  {
    // Update caches.
    m_lightCache.clear();
    m_cameraCache.clear();
    m_environmentVolumeCache.clear();
    m_skyCache    = nullptr;
    m_boundingBox = BoundingBox();

    for (int i = 0; i < m_entities.size(); i++)
    {
      EntityPtr& ntt        = m_entities[i];

      // update bounding box.
      const BoundingBox& bb = ntt->GetBoundingBox(true);
      if (bb.IsValid())
      {
        m_boundingBox.UpdateBoundary(bb);
      }

      if (const EnvironmentComponentPtr& envComp = ntt->GetComponent<EnvironmentComponent>())
      {
        if (envComp->GetHdriVal() != nullptr && envComp->GetIlluminateVal())
        {
          envComp->Init(true);
          m_environmentVolumeCache.push_back(envComp);
        }
      }

      if (const LightPtr& light = SafeCast<Light>(ntt))
      {
        light->UpdateShadowCamera();
        m_lightCache.push_back(light);
      }
      else if (const CameraPtr& cam = SafeCast<Camera>(ntt))
      {
        m_cameraCache.push_back(cam);
      }
      else if (const SkyBasePtr& sky = SafeCast<SkyBase>(ntt))
      {
        m_skyCache = sky;
      }

      const ULongID id = ntt->GetIdVal();
      auto it          = std::find(m_entitesToBVHUpdate.begin(), m_entitesToBVHUpdate.end(), id);
      if (it != m_entitesToBVHUpdate.end())
      {
        m_bvh->UpdateEntity(ntt);
      }
    }

    m_entitesToBVHUpdate.clear();

    m_bvh->Update();
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

  Scene::PickData Scene::PickObject(Ray ray, const IDArray& ignoreList, const EntityPtrArray& extraList)
  {
    PickData pd;
    pd.pickPos                  = ray.position + ray.direction * 5.0f;

    float closestPickedDistance = FLT_MAX;

    auto pickFn = [&ignoreList, &ray, &pd, &closestPickedDistance](const EntityPtrArray& entities) -> void
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
    if (pd.entity == nullptr)
    {
      m_bvh->PickObject(ray, pd, ignoreList, closestPickedDistance);
      // pickFn(m_entities);
    }

    return pd;
  }

  void Scene::PickObject(const Frustum& frustum,
                         PickDataArray& pickedObjects,
                         const IDArray& ignoreList,
                         const EntityPtrArray& extraList,
                         bool pickPartiallyInside)
  {
    auto pickFn = [&frustum, &pickedObjects, &ignoreList, &pickPartiallyInside](const EntityPtrArray& entities) -> void
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

        BoundingBox bb      = ntt->GetBoundingBox(true);
        IntersectResult res = FrustumBoxIntersection(frustum, bb);
        if (res != IntersectResult::Outside)
        {
          PickData pd;
          pd.pickPos = (bb.max + bb.min) * 0.5f;
          pd.entity  = ntt;

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

    pickFn(extraList);

    m_bvh->PickObject(frustum, pickedObjects, ignoreList, extraList, pickPartiallyInside);
    // pickFn(m_entities);
  }

  EntityPtr Scene::GetEntity(ULongID id) const
  {
    for (EntityPtr ntt : m_entities)
    {
      if (ntt->GetIdVal() == id)
      {
        return ntt;
      }
    }

    return nullptr;
  }

  void Scene::AddEntity(EntityPtr entity)
  {
    if (entity)
    {
      bool isUnique = GetEntity(entity->GetIdVal()) == nullptr;
      assert(isUnique);
      if (isUnique)
      {
        m_entities.push_back(entity);
        entity->m_scene = this;
        m_bvh->AddEntity(entity);
      }
    }
  }

  EntityPtrArray& Scene::AccessEntityArray() { return m_entities; }

  void Scene::RemoveChildren(EntityPtr removed)
  {
    NodeRawPtrArray& children = removed->m_node->m_children;

    // recursive remove children
    // (RemoveEntity function will call all children recursively).
    while (!children.empty())
    {
      Node* child = children.back();
      children.pop_back();
      if (EntityPtr childNtt = child->OwnerEntity())
      {
        RemoveEntity(childNtt->GetIdVal());
      }
    }
  }

  EntityPtr Scene::RemoveEntity(ULongID id, bool deep)
  {
    EntityPtr removed = nullptr;
    for (size_t i = 0; i < m_entities.size(); i++)
    {
      if (m_entities[i]->GetIdVal() == id)
      {
        removed = m_entities[i];
        m_entities.erase(m_entities.begin() + i);
        m_bvh->RemoveEntity(removed);
        removed->m_scene = nullptr;

        // Keep hierarchy if its prefab.
        if (removed->GetPrefabRoot() == nullptr)
        {
          removed->m_node->OrphanSelf();
        }

        if (deep)
        {
          RemoveChildren(removed);
        }
        else
        {
          removed->m_node->OrphanAllChildren(true);
        }
        break;
      }
    }

    return removed;
  }

  void Scene::RemoveEntity(const EntityPtrArray& entities)
  {
    erase_if(m_entities, [entities](EntityPtr ntt) -> bool { return FindIndex(entities, ntt) != -1; });
  }

  void Scene::RemoveAllEntities()
  {
    m_entities.clear();
    m_entitesToBVHUpdate.clear();
  }

  const EntityPtrArray& Scene::GetEntities() const { return m_entities; }

  LightPtrArray& Scene::GetLights() const { return m_lightCache; }

  CameraPtrArray& Scene::GetCameras() const { return m_cameraCache; }

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
    prefab->Init(this);
    prefab->Link();
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
      ntt->m_scene  = nullptr;
      if (removeResources)
      {
        ntt->RemoveResources();
      }
    }

    m_entities.clear();
    m_entitesToBVHUpdate.clear();

    m_loaded    = false;
    m_initiated = false;
  }

  void Scene::SavePrefab(EntityPtr entity)
  {
    // Assign a default node.
    Node* prevNode             = entity->m_node;
    entity->m_node             = new Node();
    entity->m_node->m_children = prevNode->m_children;

    // Construct prefab.
    ScenePtr prefab            = MakeNewPtr<Scene>();
    prefab->AddEntity(entity);
    GetChildren(entity, prefab->m_entities);
    String name = entity->GetNameVal() + SCENE;
    prefab->SetFile(PrefabPath(name));
    prefab->m_name = name;
    prefab->Save(false);
    prefab->m_entities.clear();
    prefab->m_entitesToBVHUpdate.clear();

    // Restore the old node.
    entity->m_node->m_children.clear();
    SafeDel(entity->m_node);
    entity->m_node = prevNode;
  }

  void Scene::ClearEntities()
  {
    m_entities.clear();
    m_entitesToBVHUpdate.clear();
  }

  void Scene::UpdateBVHEntity(ULongID id) { m_entitesToBVHUpdate.push_back(id); }

  void Scene::RebuildBVH() { m_bvh->ReBuild(); }

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

    for (EntityPtr ntt : cpy->m_entities)
    {
      ntt->m_scene = static_cast<Scene*>(other);
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
      prefab->Init(this);
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

      if (ntt->IsA<Prefab>())
      {
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

    // Do not serialize post processing settings if this is prefab.
    if (!m_isPrefab)
    {

      m_postProcessSettings.DeSerialize(info.Document, parent);
    }

    for (EntityPtr ntt : prefabList)
    {
      PrefabPtr prefab = std::static_pointer_cast<Prefab>(ntt);
      prefab->Init(this);
      prefab->Link();
    }
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

    // apply scene post processing effects
    GetEngineSettings().PostProcessing = m_currentScene->m_postProcessSettings;
    m_currentScene->RebuildBVH(); // Rebuild BVH with new parameters
  }

} // namespace ToolKit
