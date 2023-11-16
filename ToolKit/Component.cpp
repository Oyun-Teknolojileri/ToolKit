/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Component.h"

#include "AnimationControllerComponent.h"
#include "Component.h"
#include "DirectionComponent.h"
#include "EnvironmentComponent.h"
#include "MaterialComponent.h"
#include "MeshComponent.h"
#include "Object.h"
#include "ResourceComponent.h"
#include "SkeletonComponent.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

  TKDefineClass(Component, Object);

  Component::Component() {}

  Component::~Component() {}

  void Component::ParameterConstructor()
  {
    Super::ParameterConstructor();
    ParamId().m_exposed = false;
  }

  XmlNode* Component::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* objNode       = Super::SerializeImp(doc, parent);
    XmlNode* componentNode = CreateXmlNode(doc, StaticClass()->Name, objNode);

    return componentNode;
  }

  XmlNode* Component::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    parent = Super::DeSerializeImp(info, parent);
    if (m_version > TKV044)
    {
      return parent->first_node(StaticClass()->Name.c_str());
    }

    return parent;
  }

  ComponentPtr ComponentFactory::Create(ComponentType Class)
  {
    switch (Class)
    {
    case ComponentType::MeshComponent:
      return MakeNewPtr<MeshComponent>();
    case ComponentType::DirectionComponent:
      return MakeNewPtr<DirectionComponent>();
    case ComponentType::MultiMaterialComponent:
    case ComponentType::MaterialComponent:
      return MakeNewPtr<MaterialComponent>();
    case ComponentType::EnvironmentComponent:
      return MakeNewPtr<EnvironmentComponent>();
    case ComponentType::AnimControllerComponent:
      return MakeNewPtr<AnimControllerComponent>();
    case ComponentType::SkeletonComponent:
      return MakeNewPtr<SkeletonComponent>();
    case ComponentType::AABBOverrideComponent:
      return MakeNewPtr<AABBOverrideComponent>();
    case ComponentType::Base:
    default:
      assert(0 && "Unknown Component Type !");
      break;
    }

    return nullptr;
  }

} // namespace ToolKit