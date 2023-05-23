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

  static VariantCategory AnimRecordComponentCategory {"Animation Record Component", 90};
  typedef std::shared_ptr<class AnimControllerComponent> AnimControllerComponentPtr;

  /**
   * The component that stores animation records for the entity
   */
  class TK_API AnimControllerComponent : public Component
  {
   public:
    TKComponentType(AnimControllerComponent);

    /**
     * Empty constructor.
     */
    AnimControllerComponent();
    virtual ~AnimControllerComponent();

    ComponentPtr Copy(Entity* ntt) override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

    void Play(const String& recordName);
    void Stop();
    void Pause();
    AnimRecordPtr GetActiveRecord();
    AnimRecordPtr GetAnimRecord(const String& signalName);
    void AddSignal(const String& signalName, AnimRecordPtr record);
    void RemoveSignal(const String& signalName);

   public:
    TKDeclareParam(AnimRecordPtrMap, Records);

   private:
    AnimRecordPtr activeRecord;
  };

} // namespace ToolKit