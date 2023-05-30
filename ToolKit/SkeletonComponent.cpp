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

#include "SkeletonComponent.h"

#include "DebugNew.h"

namespace ToolKit
{

  SkeletonComponent::SkeletonComponent()
  {
    SkeletonResource_Define(nullptr, SkeletonComponentCategory.Name, SkeletonComponentCategory.Priority, true, true);
  }

  SkeletonComponent::~SkeletonComponent() { m_map = nullptr; }

  void SkeletonComponent::Init()
  {
    const SkeletonPtr& resource = GetSkeletonResourceVal();
    if (resource == nullptr)
    {
      return;
    }

    m_map = std::make_shared<DynamicBoneMap>();
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
    Init();
  }

} // namespace ToolKit