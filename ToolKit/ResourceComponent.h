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
#include "MathUtil.h"
#include "Types.h"

namespace ToolKit
{

  typedef std::shared_ptr<class AABBOverrideComponent> AABBOverrideComponentPtr;
  typedef std::vector<AABBOverrideComponentPtr> AABBOverrideComponentPtrArray;

  static VariantCategory AABBOverrideCompCategory {"AABB Override Component", 90};

  class TK_API AABBOverrideComponent : public Component
  {
   public:
    TKDeclareClass(AABBOverrideComponent, Component);

    AABBOverrideComponent();
    virtual ~AABBOverrideComponent();

    /**
     * Creates a copy of the AABB Override Component.
     * @param ntt Parent Entity of the component.
     * @return Copy of the AABBOverrideComponent.
     */
    ComponentPtr Copy(Entity* ntt) override;

    void Init(bool flushClientSideArray);
    BoundingBox GetAABB();

    // AABB should be in entity space (not world space)
    void SetAABB(BoundingBox aabb);

   protected:
    void ParameterConstructor() override;
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;

   private:
    TKDeclareParam(Vec3, PositionOffset);
    TKDeclareParam(Vec3, Size);
  };
} //  namespace ToolKit
