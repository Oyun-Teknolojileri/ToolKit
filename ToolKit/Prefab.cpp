#include "Prefab.h"

#include "Scene.h"
#include "ToolKit.h"

#include <utility>

#include "DebugNew.h"

namespace ToolKit
{
  Prefab::Prefab()
  {
    ParameterConstructor();
    ParameterEventConstructor();
  }

  Prefab::~Prefab() { UnInit(); }

  void Prefab::UnInit()
  {
    if (m_initiated)
    {
      m_currentScene->RemoveEntity(m_instanceEntities);
      for (Entity* ntt : m_instanceEntities)
      {
        SafeDel(ntt);
      }
    }
    m_initiated = false;
  }

  EntityType Prefab::GetType() const { return EntityType::Entity_Prefab; }

  Prefab* Prefab::GetPrefabRoot(Entity* ntt)
  {
    if (ntt->GetType() == EntityType::Entity_Prefab)
    {
      return static_cast<Prefab*>(ntt);
    }
    else if (ntt->m_node->m_parent == nullptr ||
             ntt->m_node->m_parent->m_entity == nullptr)
    {
      return nullptr;
    }
    return GetPrefabRoot(ntt->m_node->m_parent->m_entity);
  }

  Entity* Prefab::CopyTo(Entity* other) const
  {
    Entity::CopyTo(other);
    ((Prefab*) other)->Init(m_currentScene);
    return other;
  }

  void Prefab::Init(Scene* curScene)
  {
    if (m_initiated)
    {
      return;
    }
    m_currentScene = curScene;
    m_prefabScene =
        GetSceneManager()->Create<Scene>(PrefabPath(GetPrefabPathVal()));
    if (m_prefabScene == nullptr)
    {
      GetLogger()->WriteConsole(LogType::Warning, "Prefab scene isn't found!");
      return;
    }
    m_prefabScene->Init();
    m_instanceEntities.clear();

    EntityRawPtrArray rootEntities;
    GetRootEntities(m_prefabScene->GetEntities(), rootEntities);

    assert(rootEntities.size() != 0 && "Prefab scene is empty");
    for (Entity* root : rootEntities)
    {
      EntityRawPtrArray instantiatedEntityList;
      DeepCopy(root, instantiatedEntityList);
      m_node->AddChild(instantiatedEntityList[0]->m_node);
      for (Entity* child : instantiatedEntityList)
      {
        m_currentScene->AddEntity(child);
        child->SetTransformLockVal(true);
        child->ParamTransformLock().m_editable = false;
      }
      m_instanceEntities.insert(m_instanceEntities.end(),
                                instantiatedEntityList.begin(),
                                instantiatedEntityList.end());
    }

    for (Entity* ntt : m_instanceEntities)
    {
      auto foundParamArray = m_childCustomDatas.find(ntt->GetNameVal());
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

  void Prefab::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Entity::DeSerialize(doc, parent);
    parent = parent->last_node();

    for (XmlNode* rNode = parent->first_node(); rNode;
         rNode          = rNode->next_sibling())
    {
      String rootName = rNode->name();
      ParameterVariantArray vars;
      for (XmlNode* var = rNode->first_node(); var; var = var->next_sibling())
      {
        ParameterVariant param;
        param.DeSerialize(doc, var);
        vars.push_back(param);
      }

      m_childCustomDatas.insert(std::make_pair(rootName, vars));
    }
  }

  void Prefab::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    Entity::Serialize(doc, parent);
    parent = CreateXmlNode(doc, "PrefabRoots", parent->last_node());

    EntityRawPtrArray childs;
    GetChildren(this, childs);
    for (Entity* child : childs)
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
      {
        MaterialComponentPtrArray matComps;
        child->GetComponent<MaterialComponent>(matComps);
        for (MaterialComponentPtr matComp : matComps) {
          matComp->GetMaterialVal()->Save(true);
        }

        MultiMaterialPtrArray mmComps;
        child->GetComponent<MultiMaterialComponent>(mmComps);
        for (MultiMaterialPtr mmComp : mmComps) {
          for (MaterialPtr mat : mmComp->GetMaterialList()) {
            mat->Save(true);
          }
        }
      }
    }
  }

  void Prefab::ParameterConstructor()
  {
    PrefabPath_Define("",
                      PrefabCategory.Name,
                      PrefabCategory.Priority,
                      true,
                      false);
  }

  void Prefab::ParameterEventConstructor() {}
} // namespace ToolKit
