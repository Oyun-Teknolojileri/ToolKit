/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "SkeletonComponent.h"

#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

  TKDefineClass(SkeletonComponent, Component);

  SkeletonComponent::SkeletonComponent() {}

  SkeletonComponent::~SkeletonComponent() { m_map = nullptr; }

  void SkeletonComponent::Init()
  {
    const SkeletonPtr& resource = GetSkeletonResourceVal();
    if (resource == nullptr)
    {
      return;
    }

    m_map = MakeNewPtr<DynamicBoneMap>();
    m_map->Init(resource.get());
  }

  ComponentPtr SkeletonComponent::Copy(EntityPtr ntt)
  {
    SkeletonComponentPtr dst = MakeNewPtr<SkeletonComponent>();
    dst->m_entity            = ntt;

    dst->SetSkeletonResourceVal(GetSkeletonResourceVal());
    dst->Init();

    return dst;
  }

  const AnimData& SkeletonComponent::GetAnimData() const { return m_animData; }

  void SkeletonComponent::ParameterConstructor()
  {
    Super::ParameterConstructor();
    SkeletonResource_Define(nullptr, SkeletonComponentCategory.Name, SkeletonComponentCategory.Priority, true, true);
  }

  XmlNode* SkeletonComponent::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    XmlNode* compNode = Super::DeSerializeImp(info, parent);
    Init();

    return compNode->first_node(StaticClass()->Name.c_str());
  }

  XmlNode* SkeletonComponent::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    if (root == nullptr)
    {
      return nullptr;
    }

    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

} // namespace ToolKit