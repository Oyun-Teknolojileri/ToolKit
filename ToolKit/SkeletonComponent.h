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

#pragma once

#include "Component.h"
#include "Skeleton.h"

namespace ToolKit
{

  static VariantCategory SkeletonComponentCategory {"Skeleton Component", 90};
  typedef std::shared_ptr<class SkeletonComponent> SkeletonComponentPtr;

  /**
   * The component that stores skeleton resource reference and dynamic bone
      transformation info
   */
  class TK_API SkeletonComponent : public Component
  {
   public:
    TKComponentType(SkeletonComponent);

    /**
     * Empty constructor.
     */
    SkeletonComponent();
    virtual ~SkeletonComponent();
    void Init();

    ComponentPtr Copy(Entity* ntt) override;
    void DeSerializeImp(XmlDocument* doc, XmlNode* parent) override;

   public:
    TKDeclareParam(SkeletonPtr, SkeletonResource);
    DynamicBoneMapPtr m_map = nullptr;
    bool isDirty            = true;
  };

} // namespace ToolKit