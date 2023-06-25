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

#include "Scene.h"
#include "ToolKit.h"

#include <utility>

#include "DebugNew.h"

namespace ToolKit
{

  TKDefineClass(Prefab, Entity);

  Prefab::Prefab()
  {
    ParameterConstructor();
    ParameterEventConstructor();
  }

  Prefab::~Prefab() { UnInit(); }

  void Prefab::UnInit()
  {
    Unlink();
    if (m_initiated)
    {
      for (Entity* ntt : m_instanceEntities)
      {
        SafeDel(ntt);
      }
    }
    m_initiated = false;
  }

  EntityType Prefab::GetType() const { return EntityType::Entity_Prefab; }

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
      for (Entity* child : m_instanceEntities)
      {
        m_currentScene->AddEntity(child);
      }
    }
  }

  Prefab* Prefab::GetPrefabRoot(Entity* ntt)
  {
    if (ntt->IsA<Prefab>())
    {
      return static_cast<Prefab*>(ntt);
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
        child->SetTransformLockVal(true);
        child->ParamTransformLock().m_editable = false;
      }
      m_instanceEntities.insert(m_instanceEntities.end(), instantiatedEntityList.begin(), instantiatedEntityList.end());
    }

    for (Entity* ntt : m_instanceEntities)
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

  void Prefab::DeSerializeImp(XmlDocument* doc, XmlNode* parent)
  {
    Entity::DeSerializeImp(doc, parent);
    parent = parent->last_node();

    for (XmlNode* rNode = parent->first_node(); rNode; rNode = rNode->next_sibling())
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

  void Prefab::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    Super::SerializeImp(doc, parent);
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
  }

  void Prefab::ParameterConstructor()
  {
    PrefabPath_Define("", PrefabCategory.Name, PrefabCategory.Priority, true, false);
  }

  void Prefab::ParameterEventConstructor() {}
} // namespace ToolKit
