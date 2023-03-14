#include "SkeletonComponent.h"

#include "DebugNew.h"

namespace ToolKit
{

  SkeletonComponent::SkeletonComponent()
  {
    SkeletonResource_Define(nullptr,
                            SkeletonComponentCategory.Name,
                            SkeletonComponentCategory.Priority,
                            true,
                            true);

    m_map = nullptr;
  }

  SkeletonComponent::~SkeletonComponent()
  {
    if (m_map)
    {
      delete m_map;
    }
  }

  void SkeletonComponent::Init()
  {
    const SkeletonPtr& resource = GetSkeletonResourceVal();
    if (resource == nullptr)
    {
      return;
    }

    m_map = new DynamicBoneMap;
    m_map->Init(resource.get());
  }

  ComponentPtr SkeletonComponent::Copy(Entity* ntt)
  {
    SkeletonComponentPtr dst = std::make_shared<SkeletonComponent>();
    dst->m_entity            = ntt;

    dst->SetSkeletonResourceVal(GetSkeletonResourceVal());
    dst->Init();

    return dst;
  }

  void SkeletonComponent::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Component::DeSerialize(doc, parent);
    m_map = new DynamicBoneMap;
    m_map->Init(GetSkeletonResourceVal().get());
  }

} // namespace ToolKit