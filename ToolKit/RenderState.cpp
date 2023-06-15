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

#include "RenderState.h"

#include "Shader.h"
#include "Util.h"

#include "DebugNew.h"

namespace ToolKit
{

  void RenderState::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* container = doc->allocate_node(rapidxml::node_type::node_element, "renderState");

    if (parent != nullptr)
    {
      parent->append_node(container);
    }
    else
    {
      doc->append_node(container);
    }

    WriteAttr(container, doc, "cullMode", std::to_string(int(cullMode)));
    WriteAttr(container, doc, "depthTest", std::to_string(int(depthTestEnabled)));
    WriteAttr(container, doc, "depthFunc", std::to_string(int(depthFunction)));
    WriteAttr(container, doc, "blendFunction", std::to_string(int(blendFunction)));
    WriteAttr(container, doc, "alphaMaskTreshold", std::to_string((float) alphaMaskTreshold));
    WriteAttr(container, doc, "drawType", std::to_string(int(drawType)));
  }

  void RenderState::DeSerializeImp(XmlDocument* doc, XmlNode* parent)
  {
    if (parent == nullptr)
    {
      return;
    }

    if (XmlNode* container = parent->first_node("renderState"))
    {
      ReadAttr(container, "cullMode", *reinterpret_cast<int*>(&cullMode));
      auto validateCullFn = [](CullingType& ct) -> void
      {
        switch (ct)
        {
        case CullingType::TwoSided:
        case CullingType::Front:
        case CullingType::Back:
          break;
        default:
          ct = CullingType::Back;
          break;
        }
      };
      validateCullFn(cullMode);

      ReadAttr(container, "depthTest", *reinterpret_cast<int*>(&depthTestEnabled));

      ReadAttr(container, "depthFunc", *reinterpret_cast<int*>(&depthFunction));
      auto validateCmpFn = [](CompareFunctions& val) -> void
      {
        switch (val)
        {
        case CompareFunctions::FuncNever:
        case CompareFunctions::FuncLess:
        case CompareFunctions::FuncEqual:
        case CompareFunctions::FuncLequal:
        case CompareFunctions::FuncGreater:
        case CompareFunctions::FuncNEqual:
        case CompareFunctions::FuncGEqual:
        case CompareFunctions::FuncAlways:
          break;
        default:
          val = CompareFunctions::FuncLess;
        }
      };
      validateCmpFn(depthFunction);

      ReadAttr(container, "blendFunction", *reinterpret_cast<int*>(&blendFunction));
      auto validateBlendFn = [](BlendFunction& bf) -> void
      {
        switch (bf)
        {
        case BlendFunction::NONE:
        case BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA:
        case BlendFunction::ALPHA_MASK:
        case BlendFunction::ONE_TO_ONE:
          break;
        default:
          bf = BlendFunction::NONE;
        }
      };
      validateBlendFn(blendFunction);

      ReadAttr(container, "alphaMaskTreshold", *reinterpret_cast<float*>(&alphaMaskTreshold));

      ReadAttr(container, "drawType", *reinterpret_cast<int*>(&drawType));
      auto validateDrawFn = [](DrawType& dt) -> void
      {
        switch (dt)
        {
        case DrawType::Triangle:
        case DrawType::Line:
        case DrawType::LineStrip:
        case DrawType::LineLoop:
        case DrawType::Point:
          break;
        default:
          dt = DrawType::Triangle;
        }
      };
      validateDrawFn(drawType);
    }
  }

} // namespace ToolKit
