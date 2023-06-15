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

  Component::Component() { m_id = GetHandleManager()->GetNextHandle(); }

  Component::~Component() {}

  ComponentType Component::GetType() const { return ComponentType::Base; }

  void Component::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* componentNode = CreateXmlNode(doc, XmlComponent, parent);
    WriteAttr(componentNode, doc, XmlParamterTypeAttr, std::to_string(static_cast<int>(GetType())));

    m_localData.Serialize(doc, componentNode);
  }

  void Component::DeSerializeImp(XmlDocument* doc, XmlNode* parent) { m_localData.DeSerialize(doc, parent); }

  Component* Component::CreateByType(ComponentType t)
  {
    switch (t)
    {
    case ComponentType::MeshComponent:
      return new MeshComponent();
      break;
    case ComponentType::DirectionComponent:
      return new DirectionComponent();
      break;
    case ComponentType::MultiMaterialComponent:
    case ComponentType::MaterialComponent:
      return new MaterialComponent();
      break;
    case ComponentType::EnvironmentComponent:
      return new EnvironmentComponent();
      break;
    case ComponentType::AnimControllerComponent:
      return new AnimControllerComponent();
      break;
    case ComponentType::SkeletonComponent:
      return new SkeletonComponent();
      break;
    case ComponentType::AABBOverrideComponent:
      return new AABBOverrideComponent;
      break;
    case ComponentType::Base:
    default:
      assert(false && "Unsupported component type.");
      break;
    }
    return nullptr;
  }

} // namespace ToolKit
