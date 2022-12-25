#include "RenderState.h"

#include "Shader.h"
#include "Util.h"

namespace ToolKit
{

  void RenderState::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* container =
        doc->allocate_node(rapidxml::node_type::node_element, "renderState");

    if (parent != nullptr)
    {
      parent->append_node(container);
    }
    else
    {
      doc->append_node(container);
    }

    WriteAttr(container, doc, "cullMode", std::to_string(int(cullMode)));

    WriteAttr(container,
              doc,
              "depthTest",
              std::to_string(int(depthTestEnabled)));

    WriteAttr(container, doc, "depthFunc", std::to_string(int(depthFunction)));

    WriteAttr(container,
              doc,
              "blendFunction",
              std::to_string(int(blendFunction)));

    WriteAttr(container,
              doc,
              "alphaMaskTreshold",
              std::to_string((float) alphaMaskTreshold));

    WriteAttr(container, doc, "drawType", std::to_string(int(drawType)));

    WriteAttr(container,
              doc,
              "vertexLayout",
              std::to_string(int(vertexLayout)));

    WriteAttr(container, doc, "AOInUse", std::to_string(AOInUse));

    WriteAttr(container, doc, "priority", std::to_string(priority));
  }

  void RenderState::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    if (parent == nullptr)
    {
      return;
    }

    if (XmlNode* container = parent->first_node("renderState"))
    {
      ReadAttr(container, "cullMode", *reinterpret_cast<int*>(&cullMode));

      ReadAttr(container,
               "depthTest",
               *reinterpret_cast<int*>(&depthTestEnabled));

      ReadAttr(container, "depthTest", *reinterpret_cast<int*>(&depthFunction));

      ReadAttr(container,
               "blendFunction",
               *reinterpret_cast<int*>(&blendFunction));

      ReadAttr(container,
               "alphaMaskTreshold",
               *reinterpret_cast<float*>(&alphaMaskTreshold));

      ReadAttr(container, "drawType", *reinterpret_cast<int*>(&drawType));

      ReadAttr(container,
               "vertexLayout",
               *reinterpret_cast<int*>(&vertexLayout));

      ReadAttr(container, "priority", priority);

      ReadAttr(container, "AOInUse", AOInUse);
    }
  }

} // namespace ToolKit
