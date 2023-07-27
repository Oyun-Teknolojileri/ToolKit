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

#include "MaterialComponent.h"

#include "Entity.h"
#include "Material.h"
#include "Mesh.h"
#include "MeshComponent.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

  TKDefineClass(MaterialComponent, Component);

  MaterialComponent::MaterialComponent() {}

  MaterialComponent::~MaterialComponent() {}

  ComponentPtr MaterialComponent::Copy(Entity* ntt)
  {
    MaterialComponentPtr mc = MakeNewPtr<MaterialComponent>();
    mc->m_localData         = m_localData;
    mc->m_entity            = ntt;
    mc->m_materialList      = m_materialList;

    return mc;
  }

  void MaterialComponent::Init(bool flushClientSideArray) {}

  const char *XmlMatCountAttrib = "MaterialCount", *XmlMatIdAttrib = "ID";

  void MaterialComponent::ParameterConstructor()
  {
    Super::ParameterConstructor();

    // DEPRECATED.
    MaterialPtr defMat = GetMaterialManager()->GetCopyOfDefaultMaterial();
    Material_Define(defMat, MaterialComponentCategory.Name, MaterialComponentCategory.Priority, true, true);
  }

  XmlNode* MaterialComponent::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    if (m_version == String("v0.4.5"))
    {
      DeSerializeImpV045(info.Document, parent);
      return nullptr;
    }

    // Old file, keep parsing.
    XmlNode* compNode = Super::DeSerializeImp(info, parent);

    uint matCount     = 0;
    ReadAttr(parent, XmlMatCountAttrib, matCount);
    m_materialList.resize(matCount);
    for (size_t i = 0; i < m_materialList.size(); i++)
    {
      XmlNode* resourceNode = parent->first_node(std::to_string(i).c_str());
      if (!resourceNode)
      {
        m_materialList[i] = GetMaterialManager()->GetCopyOfDefaultMaterial();
        continue;
      }
      m_materialList[i] = GetMaterialManager()->Create<Material>(MaterialPath(Resource::DeserializeRef(resourceNode)));
    }

    // Deprecated Here for compatibility with 0.4.1
    if (m_materialList.empty())
    {
      if (MaterialPtr mat = GetMaterialVal())
      {
        // Transfer the old material in to the list.
        m_materialList.push_back(mat);
        m_localData.Remove(ParamMaterial().m_id);
      }
    }

    return compNode->first_node(StaticClass()->Name.c_str());
  }

  void MaterialComponent::DeSerializeImpV045(XmlDocument* doc, XmlNode* parent)
  {
    SerializationFileInfo sfi;
    sfi.Document = doc;
    sfi.Version  = "v0.4.5";

    Super::DeSerializeImp(sfi, parent);

    XmlNode* comNode = parent->first_node(Component::StaticClass()->Name.c_str());
    XmlNode* matNode = comNode->first_node(StaticClass()->Name.c_str());

    uint matCount    = 0;
    ReadAttr(matNode, XmlMatCountAttrib, matCount);
    m_materialList.resize(matCount);

    for (size_t i = 0; i < m_materialList.size(); i++)
    {
      XmlNode* resourceNode = matNode->first_node(std::to_string(i).c_str());
      if (resourceNode)
      {
        String matRef     = Resource::DeserializeRef(resourceNode);
        m_materialList[i] = GetMaterialManager()->Create<Material>(MaterialPath(matRef));
      }
      else
      {
        m_materialList[i] = GetMaterialManager()->GetCopyOfDefaultMaterial();
        continue;
      }
    }

    // Deprecated Here for compatibility with 0.4.1
    if (m_materialList.empty())
    {
      if (MaterialPtr mat = GetMaterialVal())
      {
        // Transfer the old material in to the list.
        m_materialList.push_back(mat);
        m_localData.Remove(ParamMaterial().m_id);
      }
    }
  }

  XmlNode* MaterialComponent::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* compNode = Super::SerializeImp(doc, parent);
    XmlNode* matNode  = CreateXmlNode(doc, StaticClass()->Name, compNode);

    WriteAttr(matNode, doc, XmlMatCountAttrib, std::to_string(m_materialList.size()));
    for (size_t i = 0; i < m_materialList.size(); i++)
    {
      if (m_materialList[i]->m_dirty)
      {
        m_materialList[i]->Save(true);
      }
      XmlNode* resourceRefNode = CreateXmlNode(doc, std::to_string(i), matNode);
      m_materialList[i]->SerializeRef(doc, resourceRefNode);
    }

    return matNode;
  }

  void MaterialComponent::AddMaterial(MaterialPtr mat) { m_materialList.push_back(mat); }

  void MaterialComponent::RemoveMaterial(uint index)
  {
    assert(m_materialList.size() >= index && "Material List overflow");
    m_materialList.erase(m_materialList.begin() + index);
  }

  const MaterialPtrArray& MaterialComponent::GetMaterialList() const { return m_materialList; }

  MaterialPtrArray& MaterialComponent::GetMaterialList() { return m_materialList; }

  void MaterialComponent::UpdateMaterialList()
  {
    m_materialList.clear();
    MeshComponentPtrArray meshComps;
    m_entity->GetComponent<MeshComponent>(meshComps);

    for (MeshComponentPtr meshComp : meshComps)
    {
      if (meshComp == nullptr || meshComp->GetMeshVal() == nullptr)
      {
        continue;
      }
      MeshRawPtrArray meshCollector;
      meshComp->GetMeshVal()->GetAllMeshes(meshCollector);

      for (uint i = 0; i < meshCollector.size(); i++)
      {
        m_materialList.push_back(meshCollector[i]->m_material);
      }
    }
  }

  MaterialPtr MaterialComponent::GetFirstMaterial()
  {
    if (m_materialList.empty())
    {
      if (const MeshComponentPtr& mc = m_entity->GetMeshComponent())
      {
        if (MeshPtr m = mc->GetMeshVal())
        {
          if (m->m_material)
          {
            // Either return the material on the mesh.
            return m->m_material;
          }
        }
      }
    }
    else
    {
      // First found material.
      return m_materialList.front();
    }

    // Worst case, a default material.
    return GetMaterialManager()->GetCopyOfDefaultMaterial();
  }

  void MaterialComponent::SetFirstMaterial(const MaterialPtr& material)
  {
    if (m_materialList.empty())
    {
      m_materialList.push_back(material);
    }
    else
    {
      m_materialList[0] = material;
    }
  }

} // namespace ToolKit