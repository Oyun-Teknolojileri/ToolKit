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

namespace ToolKit
{

  typedef std::shared_ptr<class EnvironmentComponent> EnvironmentComponentPtr;
  typedef std::vector<EnvironmentComponentPtr> EnvironmentComponentPtrArray;

  static VariantCategory EnvironmentComponentCategory {"Environment Component", 90};

  class TK_API EnvironmentComponent : public Component
  {
   public:
    TKDeclareClass(EnvironmentComponent, Component);

    EnvironmentComponent();
    virtual ~EnvironmentComponent();

    ComponentPtr Copy(EntityPtr ntt) override;
    BoundingBox GetBBox();

    void Init(bool flushClientSideArray);

   protected:
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;

   private:
    void ParameterConstructor() override;
    void ParameterEventConstructor() override;
    void ReInitHdri(HdriPtr hdri, float exposure);

   public:
    TKDeclareParam(HdriPtr, Hdri);
    TKDeclareParam(Vec3, Size);
    TKDeclareParam(Vec3, PositionOffset);
    TKDeclareParam(bool, Illuminate);
    TKDeclareParam(float, Intensity);
    TKDeclareParam(float, Exposure);
    TKDeclareParam(MultiChoiceVariant, IBLTextureSize);
  };

} // namespace ToolKit