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

#include "Prefab.h"

#include "Material.h"
#include "Scene.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

  TKDefineClass(Prefab, Entity);

  Prefab::Prefab() {}

  Prefab::~Prefab() { UnInit(); }

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
        m_currentScene->RemoveEntity(m_instanceEntities);
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
    }
  }

  PrefabPtr Prefab::GetPrefabRoot(const EntityPtr ntt)
  {
    if (ntt->IsA<Prefab>())
    {
      return std::static_pointer_cast<Prefab>(ntt);
    }
    else if (ntt->m_node->m_parent == nullptr || ntt->m_node->m_parent->m_entity == nullptr)
    {
      return nullptr;
    }

    return GetPrefabRoot(ntt->m_node->m_parent->m_entity);
  }

  Entity* Prefab::CopyTo(Entity* other) const
  {
    Entity::CopyTo(other);
    Prefab* prefab = (Prefab*) other;
    prefab->Init(m_currentScene);
    prefab->Link();
    return other;
  }

  void Prefab::Init(Scene* curScene)
  {
    if (m_initiated)
    {
      return;
    }
    m_currentScene = curScene;
    m_prefabScene  = GetSceneManager()->Create<Scene>(PrefabPath(GetPrefabPathVal()));
    if (m_prefabScene == nullptr)
    {
      GetLogger()->WriteConsole(LogType::Warning, "Prefab scene isn't found!");
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
      m_node->AddChild(instantiatedEntityList[0]->m_node);
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
      MaterialComponentPtrArray mmComps;
      child->GetComponent<MaterialComponent>(mmComps);
      for (MaterialComponentPtr mmComp : mmComps)
      {
        for (MaterialPtr mat : mmComp->GetMaterialList())
        {
          mat->Save(true);
        }
      }
    }

    return prefabNode;
  }

  XmlNode* Prefab::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    if (info.Version == TKV045)
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

  void Prefab::ParameterConstructor()
  {
    Super::ParameterConstructor();
    PrefabPath_Define("", PrefabCategory.Name, PrefabCategory.Priority, true, false);
  }

} // namespace ToolKit
