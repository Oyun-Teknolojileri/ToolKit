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

#include "Component.h"

#include "AnimationControllerComponent.h"
#include "DirectionComponent.h"
#include "EnvironmentComponent.h"
#include "ResourceComponent.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

  TKDefineClass(Component, TKObject);

  Component::Component() {}

  Component::~Component() {}

  XmlNode* Component::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* objNode       = Super::SerializeImp(doc, parent);
    XmlNode* componentNode = CreateXmlNode(doc, StaticClass()->Name, objNode);

    return componentNode;
  }

  void Component::DeSerializeImp(XmlDocument* doc, XmlNode* parent) { m_localData.DeSerialize(doc, parent); }

  void ComponentFactory::Init()
  {
    m_constructorFunctions[MeshComponent::StaticClass()->Name] = []() -> Component* { return new MeshComponent(); };

    m_constructorFunctions[DirectionComponent::StaticClass()->Name] = []() -> Component*
    { return new DirectionComponent(); };

    m_constructorFunctions[MaterialComponent::StaticClass()->Name] = []() -> Component*
    { return new MaterialComponent(); };

    m_constructorFunctions[EnvironmentComponent::StaticClass()->Name] = []() -> Component*
    { return new EnvironmentComponent(); };

    m_constructorFunctions[AnimControllerComponent::StaticClass()->Name] = []() -> Component*
    { return new AnimControllerComponent(); };

    m_constructorFunctions[SkeletonComponent::StaticClass()->Name] = []() -> Component*
    { return new SkeletonComponent(); };

    m_constructorFunctions[AABBOverrideComponent::StaticClass()->Name] = []() -> Component*
    { return new AABBOverrideComponent(); };
  }

  Component* ComponentFactory::Create(ComponentType cls)
  {
    switch (cls)
    {
    case ComponentType::MeshComponent:
      return Create(MeshComponent::StaticClass());
    case ComponentType::DirectionComponent:
      return Create(DirectionComponent::StaticClass());
    case ComponentType::MultiMaterialComponent:
    case ComponentType::MaterialComponent:
      return Create(MaterialComponent::StaticClass());
    case ComponentType::EnvironmentComponent:
      return Create(EnvironmentComponent::StaticClass());
    case ComponentType::AnimControllerComponent:
      return Create(AnimControllerComponent::StaticClass());
    case ComponentType::SkeletonComponent:
      return Create(SkeletonComponent::StaticClass());
    case ComponentType::AABBOverrideComponent:
      return Create(AABBOverrideComponent::StaticClass());
    case ComponentType::Base:
    default:
      assert(0 && "Unknown Component Type !");
      break;
    }
    return nullptr;
  }

  Component* ComponentFactory::Create(StringView cls)
  {
    Component* com = nullptr;
    auto comIt     = m_constructorFunctions.find(cls.data());
    if (comIt != m_constructorFunctions.end())
    {
      com = comIt->second();
      com->NativeConstruct();
    }

    return com;
  }

  Component* ComponentFactory::Create(TKClass* cls) { return Create(cls->Name); }

} // namespace ToolKit