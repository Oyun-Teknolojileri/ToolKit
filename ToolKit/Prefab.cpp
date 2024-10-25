/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Prefab.h"

#include "Material.h"
#include "MathUtil.h"
#include "Scene.h"
#include "ToolKit.h"

namespace ToolKit
{

  TKDefineClass(Prefab, Entity);

  Prefab::Prefab() {}

  Prefab::~Prefab() { UnInit(); }

  bool Prefab::IsDrawable() const
  {
    for (int i = 0; i < (int) m_instanceEntities.size(); i++)
    {
      if (m_instanceEntities[i]->IsDrawable())
      {
        return true;
      }
    }

    return false;
  }

  void Prefab::UnInit()
  {
    Unlink();
    m_instanceEntities.clear();
    m_initiated = false;
  }

  void Prefab::Unlink()
  {
    if (m_initiated)
    {
      if (m_linked)
      {
        m_linked = false;

        EntityPtrArray roots;
        GetRootEntities(m_instanceEntities, roots);
        m_currentScene->RemoveEntity(roots);

        // Detach roots from prefab.
        for (EntityPtr root : roots)
        {
          root->m_node->OrphanSelf();
        }
      }
    }
  }

  void Prefab::Link()
  {
    assert(!m_linked);
    if (!m_linked)
    {
      m_linked = true;
      for (EntityPtr child : m_instanceEntities)
      {
        m_currentScene->AddEntity(child);
      }

      // Attach roots to prefab.
      EntityPtrArray roots;
      GetRootEntities(m_instanceEntities, roots);
      for (EntityPtr root : roots)
      {
        m_node->AddChild(root->m_node);
      }
    }
  }

  PrefabPtr Prefab::GetPrefabRoot(const EntityPtr ntt)
  {
    if (ntt->IsA<Prefab>())
    {
      return Cast<Prefab>(ntt);
    }

    if (EntityPtr parent = ntt->Parent())
    {
      return GetPrefabRoot(parent);
    }

    return nullptr;
  }

  Entity* Prefab::CopyTo(Entity* other) const
  {
    Entity::CopyTo(other);
    Prefab* prefab = (Prefab*) other;
    prefab->Init(m_currentScene);

    return other;
  }

  EntityPtr Prefab::GetFirstByName(const String& name)
  {
    if (!m_initiated || !m_linked || m_currentScene == nullptr)
    {
      return nullptr;
    }

    for (uint i = 0; i < m_instanceEntities.size(); ++i)
    {
      EntityPtr instanceEntity = m_instanceEntities[i];
      if (instanceEntity->GetNameVal() == name)
      {
        return instanceEntity;
      }
    }

    return nullptr;
  }

  EntityPtr Prefab::GetFirstByTag(const String& tag)
  {
    if (!m_initiated || !m_linked || m_currentScene == nullptr)
    {
      return nullptr;
    }

    for (uint i = 0; i < m_instanceEntities.size(); ++i)
    {
      EntityPtr instanceEntity = m_instanceEntities[i];
      if (instanceEntity->GetTagVal() == tag)
      {
        return instanceEntity;
      }
    }

    return nullptr;
  }

  const EntityPtrArray& Prefab::GetInstancedEntities() { return m_instanceEntities; }

  void Prefab::Init(Scene* curScene)
  {
    if (m_initiated)
    {
      return;
    }

    m_currentScene    = curScene;
    String prefabPath = GetPrefabPathVal();
    prefabPath        = PrefabPath(prefabPath);
    m_prefabScene     = GetSceneManager()->Create<Scene>(prefabPath);
    if (m_prefabScene == nullptr)
    {
      TK_ERR("Prefab scene isn't found.");
      return;
    }

    m_prefabScene->Init();
    m_instanceEntities.clear();

    EntityPtrArray rootEntities;
    GetRootEntities(m_prefabScene->GetEntities(), rootEntities);

    assert(rootEntities.size() != 0 && "Prefab scene is empty");
    for (EntityPtr root : rootEntities)
    {
      EntityPtrArray instantiatedEntityList;
      DeepCopy(root, instantiatedEntityList);

      for (EntityPtr child : instantiatedEntityList)
      {
        child->SetTransformLockVal(true);
        child->ParamTransformLock().m_editable = false;
      }
      m_instanceEntities.insert(m_instanceEntities.end(), instantiatedEntityList.begin(), instantiatedEntityList.end());
    }

    for (EntityPtr ntt : m_instanceEntities)
    {
      ntt->_prefabRootEntity = this;

      auto foundParamArray   = m_childCustomDatas.find(ntt->GetNameVal());
      if (foundParamArray != m_childCustomDatas.end())
      {
        for (ParameterVariant& var : ntt->m_localData.m_variants)
        {
          for (ParameterVariant& serializedVar : foundParamArray->second)
          {
            if (var.m_name == serializedVar.m_name)
            {
              var = serializedVar;
            }
          }
        }
      }
    }

    // We need this data only at deserialization, no later
    m_childCustomDatas.clear();
    m_initiated = true;
  }

  XmlNode* Prefab::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* nttNode    = Super::SerializeImp(doc, parent);
    XmlNode* prefabNode = CreateXmlNode(doc, StaticClass()->Name, nttNode);
    parent              = CreateXmlNode(doc, "PrefabRoots", prefabNode);

    EntityPtrArray childs;
    GetChildren(Self<Entity>(), childs);
    for (EntityPtr child : childs)
    {
      XmlNode* rootSer = CreateXmlNode(doc, child->GetNameVal(), parent);
      for (const ParameterVariant& var : child->m_localData.m_variants)
      {
        if (var.m_category.Name == CustomDataCategory.Name)
        {
          var.Serialize(doc, rootSer);
        }
      }

      // Save material changes
      if (MaterialComponentPtr matComp = child->GetComponent<MaterialComponent>())
      {
        for (MaterialPtr mat : matComp->GetMaterialList())
        {
          mat->Save(true);
        }
      }
    }

    return prefabNode;
  }

  XmlNode* Prefab::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    if (info.Version > TKV044)
    {
      return DeSerializeImpV045(info, parent);
    }

    // old file.

    XmlNode* nttNode = Super::DeSerializeImp(info, parent);
    parent           = parent->last_node();

    for (XmlNode* rNode = parent->first_node(); rNode; rNode = rNode->next_sibling())
    {
      String rootName = rNode->name();
      ParameterVariantArray vars;
      for (XmlNode* var = rNode->first_node(); var; var = var->next_sibling())
      {
        ParameterVariant param;
        param.DeSerialize(info, var);
        vars.push_back(param);
      }

      m_childCustomDatas.insert(std::make_pair(rootName, vars));
    }

    return nttNode;
  }

  XmlNode* Prefab::DeSerializeImpV045(const SerializationFileInfo& info, XmlNode* parent)
  {
    XmlNode* nttNode     = Super::DeSerializeImp(info, parent);
    XmlNode* prefabNode  = nttNode->first_node(StaticClass()->Name.c_str());
    XmlNode* prefabRoots = prefabNode->first_node("PrefabRoots");

    for (XmlNode* rNode = prefabRoots->first_node(); rNode; rNode = rNode->next_sibling())
    {
      String rootName = rNode->name();
      ParameterVariantArray vars;
      for (XmlNode* var = rNode->first_node(); var; var = var->next_sibling())
      {
        ParameterVariant param;
        param.DeSerialize(info, var);
        vars.push_back(param);
      }

      m_childCustomDatas.insert(std::make_pair(rootName, vars));
    }

    return prefabNode;
  }

  void Prefab::UpdateLocalBoundingBox()
  {
    if (m_prefabScene)
    {
      m_localBoundingBoxCache = m_prefabScene->GetSceneBoundary();
    }
    else
    {
      m_localBoundingBoxCache = infinitesimalBox;
    }
  }

  void Prefab::ParameterConstructor()
  {
    Super::ParameterConstructor();
    PrefabPath_Define("", PrefabCategory.Name, PrefabCategory.Priority, true, false);
  }

} // namespace ToolKit
