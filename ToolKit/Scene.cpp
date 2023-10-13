/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Scene.h"

#include "Component.h"
#include "EngineSettings.h"
#include "EnvironmentComponent.h"
#include "FileManager.h"
#include "MathUtil.h"
#include "Prefab.h"
#include "ResourceComponent.h"
#include "ToolKit.h"
#include "Util.h"
#include "Logger.h"
#include <unordered_set>

#include "DebugNew.h"

namespace ToolKit
{

  TKDefineClass(Scene, Resource);

  Scene::Scene() { m_name = "New Scene"; }

  Scene::Scene(const String& file) : Scene() { SetFile(file); }

  Scene::~Scene() { Destroy(false); }

  void Scene::Load()
  {
    if (!m_loaded)
    {
      String path = GetFile();
      m_isPrefab  = path.find("Prefabs") != String::npos;

      ParseDocument(XmlSceneElement);

      // Update parent - child relation for entities.
      for (EntityPtr ntt : m_entities)
      {
        if (ntt->_parentId != 0)
        {
          if (EntityPtr parent = GetEntity(ntt->_parentId))
          {
            parent->m_node->AddChild(ntt->m_node);
          }
        }
      }

      m_loaded = true;
    }
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
        MeshComponentPtrArray meshes;
        ntt->GetComponent<MeshComponent>(meshes);
        for (MeshComponentPtr& mesh : meshes)
        {
          mesh->Init(flushClientSideArray);
        }

        // Environment component.
        if (EnvironmentComponentPtr envCom = ntt->GetComponent<EnvironmentComponent>())
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
    const EntityPtrArray& entities = other->GetEntities();
    TKSet<uint64> newAddedIds;

    for (EntityPtr ntt : entities)
    {
      // release the handle and get new random id to ensure we have unique id
      GetHandleManager()->ReleaseHandle(ntt->GetIdVal()); 
      ULongID id = GetHandleManager()->GetNextHandle();
      ntt->SetIdVal(id);
      AddEntity(ntt);
    }

    other->RemoveAllEntities();
    GetSceneManager()->Remove(other->GetFile());
  }

  Scene::PickData Scene::PickObject(Ray ray, const EntityIdArray& ignoreList, const EntityPtrArray& extraList)
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

        Ray rayInObjectSpace       = ray;
        Mat4 ts                    = ntt->m_node->GetTransform(TransformationSpace::TS_WORLD);
        Mat4 its                   = glm::inverse(ts);
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
                         PickDataArray& pickedObjects,
                         const EntityIdArray& ignoreList,
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

        BoundingBox bb      = ntt->GetAABB(true);
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

    pickFn(m_entities);
    pickFn(extraList);
  }

  EntityPtr Scene::GetEntity(ULongID id) const
  {
    if (m_entities.size() == 0) 
    {
      return nullptr;
    }
    auto fnd = m_idToEntityMap.find(id);
    return fnd != m_idToEntityMap.end() ? m_entities[fnd->second] : nullptr;
  }

  void Scene::AddEntity(EntityPtr entity)
  {
    if (entity)
    {
      ULongID id    = entity->GetIdVal();
      bool isUnique = GetEntity(id) == nullptr;
      assert(isUnique);
      if (isUnique)
      {
        m_idToEntityMap[id] = m_entities.size();
        m_entities.push_back(entity);
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
      RemoveEntity(child->m_entity->GetIdVal());
    }
  }

  EntityPtr Scene::RemoveEntity(ULongID id, bool deep)
  {
    auto fnd = m_idToEntityMap.find(id);
    if (fnd == m_idToEntityMap.end())
    {
      return nullptr;
    }
    
    int index = m_idToEntityMap[fnd->first];
    EntityPtr removed = m_entities[index];
    EntityPtr last = m_entities.back();
    ULongID lastId = last->GetIdVal();
    // move last entity to index
    m_idToEntityMap.erase(id);
    m_entities[index]       = last;
    m_idToEntityMap[lastId] = index; // we moved last to index
    // remove last entity because we moved it to index
    m_entities.pop_back();
    
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

    return removed;
  }

  void Scene::RemoveEntity(const EntityPtrArray& entities)
  {
    for (EntityPtr ntt : entities)
    {
      RemoveEntity(ntt->GetIdVal(), false);
    }
  }

  void Scene::RemoveAllEntities() 
  {
    m_entities.clear(); 
    m_idToEntityMap.clear();
  }

  const EntityPtrArray& Scene::GetEntities() const { return m_entities; }

  LightPtrArray Scene::GetLights() const
  {
    LightPtrArray lights;
    for (EntityPtr ntt : m_entities)
    {
      if (ntt->IsA<Light>())
      {
        lights.push_back(std::static_pointer_cast<Light>(ntt));
      }
    }

    return lights;
  }

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

  // Returns the last sky added
  SkyBasePtr Scene::GetSky()
  {
    for (int i = (int) m_entities.size() - 1; i >= 0; --i)
    {
      if (m_entities[i]->IsA<SkyBase>())
      {
        return std::static_pointer_cast<SkyBase>(m_entities[i]);
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
        GetLogger()->WriteConsole(LogType::Error, "You can't use a prefab outside of Prefab folder!");
        return;
      }
    }
    PrefabPtr prefab = MakeNewPtr<Prefab>();
    prefab->SetPrefabPathVal(path);
    prefab->Init(this);
    prefab->Link();
    AddEntity(prefab);
  }

  EnvironmentComponentPtrArray Scene::GetEnvironmentVolumes()
  {
    // Find entities which have environment component
    EnvironmentComponentPtrArray environments;
    for (EntityPtr ntt : m_entities)
    {
      EnvironmentComponentPtr envCom = ntt->GetComponent<EnvironmentComponent>();
      if (envCom != nullptr && envCom->GetHdriVal() != nullptr && envCom->GetIlluminateVal())
      {
        envCom->Init(true);
        environments.push_back(envCom);
      }
    }

    return environments;
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
    m_idToEntityMap.clear();

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
    m_idToEntityMap.clear();
  }

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
    cpy->m_idToEntityMap = m_idToEntityMap;
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
      GetEngineSettings().SerializePostProcessing(doc, nullptr);
    }

    return scene;
  }

  XmlNode* Scene::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    // Match scene name with file name.
    String path = GetSerializeFile();
    DecomposePath(path, nullptr, &m_name, nullptr);

    if (m_version == TKV045)
    {
      DeSerializeImpV045(info, parent);
      return nullptr;
    }

    // Old file, keep parsing.
    // choose scene's start index, entities will have indexes starting from this, 
    // even though number overflows we don't care
    ULongID lastID    = GetHandleManager()->GetNextHandle();   
    XmlNode* node     = nullptr;

    EntityPtrArray prefabList;

    const char* xmlRootObject = XmlEntityElement.c_str();
    const char* xmlObjectType = XmlEntityTypeAttr.c_str();
    
    for (node = parent->first_node(xmlRootObject); node; node = node->next_sibling(xmlRootObject))
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
      uint64 listIndx = 0;
      ReadAttr(node, XmlEntityIdAttr, listIndx);
      ntt->SetIdVal(listIndx);
      AddEntity(ntt);
    }

    // Do not serialize post processing settings if this is prefab.
    if (!m_isPrefab)
    {
      GetEngineSettings().DeSerializePostProcessing(info.Document, parent);
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
    XmlNode* root     = info.Document->first_node(XmlSceneElement.c_str());

    // Old file, keep parsing.
    ULongID biggestID = 0;
    XmlNode* node     = nullptr;

    EntityPtrArray prefabList;

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

      AddEntity(ntt);
    }

    // Do not serialize post processing settings if this is prefab.
    if (!m_isPrefab)
    {
      GetEngineSettings().DeSerializePostProcessing(info.Document, parent);
    }

    for (EntityPtr ntt : prefabList)
    {
      PrefabPtr prefab = std::static_pointer_cast<Prefab>(ntt);
      prefab->Init(this);
      prefab->Link();
    }
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

  bool SceneManager::CanStore(TKClass* Class) { return Class == Scene::StaticClass(); }

  ResourcePtr SceneManager::CreateLocal(TKClass* Class)
  {
    if (Class == Scene::StaticClass())
    {
      return MakeNewPtr<Scene>();
    }

    return nullptr;
  }

  String SceneManager::GetDefaultResource(TKClass* Class) { return ScenePath("Sample.scene", true); }

  ScenePtr SceneManager::GetCurrentScene() { return m_currentScene; }

  void SceneManager::SetCurrentScene(const ScenePtr& scene) { m_currentScene = scene; }

} // namespace ToolKit
