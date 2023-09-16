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
#include "MaterialComponent.h"
#include "MeshComponent.h"
#include "ResourceComponent.h"
#include "SkeletonComponent.h"

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
    if (m_version > String("v0.4.4"))
    {
      return parent->first_node(StaticClass()->Name.c_str());
    }

    return parent;
  }

  Component* ComponentFactory::Create(ComponentType Class)
  {
    switch (Class)
    {
    case ComponentType::MeshComponent:
      return MakeNew<MeshComponent>();
    case ComponentType::DirectionComponent:
      return MakeNew<DirectionComponent>();
    case ComponentType::MultiMaterialComponent:
    case ComponentType::MaterialComponent:
      return MakeNew<MaterialComponent>();
    case ComponentType::EnvironmentComponent:
      return MakeNew<EnvironmentComponent>();
    case ComponentType::AnimControllerComponent:
      return MakeNew<AnimControllerComponent>();
    case ComponentType::SkeletonComponent:
      return MakeNew<SkeletonComponent>();
    case ComponentType::AABBOverrideComponent:
      return MakeNew<AABBOverrideComponent>();
    case ComponentType::Base:
    default:
      assert(0 && "Unknown Component Type !");
      break;
    }

    return nullptr;
  }

} // namespace ToolKit