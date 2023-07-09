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

#include "ResourceComponent.h"

#include "Animation.h"
#include "Entity.h"
#include "Material.h"
#include "Mesh.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

  TKDefineClass(AABBOverrideComponent, Component);

  AABBOverrideComponent::AABBOverrideComponent() {}

  AABBOverrideComponent::~AABBOverrideComponent() {}

  ComponentPtr AABBOverrideComponent::Copy(Entity* ntt)
  {
    AABBOverrideComponentPtr dst = MakeNewPtr<AABBOverrideComponent>();
    dst->m_entity                = ntt;
    dst->m_localData             = m_localData;

    return dst;
  }

  void AABBOverrideComponent::Init(bool flushClientSideArray) {}

  BoundingBox AABBOverrideComponent::GetAABB()
  {
    BoundingBox aabb = {};
    aabb.min         = GetPositionOffsetVal();
    aabb.max         = GetPositionOffsetVal() + GetSizeVal();
    return aabb;
  }

  void AABBOverrideComponent::SetAABB(BoundingBox aabb)
  {
    SetPositionOffsetVal(aabb.min);
    SetSizeVal(aabb.max - aabb.min);
  }

  void AABBOverrideComponent::ParameterConstructor()
  {
    Super::ParameterConstructor();

    PositionOffset_Define(Vec3(0.0f), AABBOverrideCompCategory.Name, AABBOverrideCompCategory.Priority, true, true);
    Size_Define(Vec3(1.0f), AABBOverrideCompCategory.Name, AABBOverrideCompCategory.Priority, true, true);
  }

  XmlNode* AABBOverrideComponent::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

} //  namespace ToolKit
