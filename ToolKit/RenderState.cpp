#include "RenderState.h"
#include "Util.h"

namespace ToolKit
{

  void RenderState::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* container = doc->allocate_node
    (
      rapidxml::node_type::node_element,
      "renderState"
    );

    if (parent != nullptr)
    {
      parent->append_node(container);
    }
    else
    {
      doc->append_node(container);
    }

    WriteAttr
    (
      container,
      doc,
      "cullMode",
      std::to_string(static_cast<int> (cullMode))
    );

    WriteAttr
    (
      container,
      doc,
      "depthTest",
      std::to_string(static_cast<int> (depthTestEnabled))
    );

    WriteAttr
    (
      container,
      doc,
      "blendFunction",
      std::to_string(static_cast<int> (blendFunction))
    );

    WriteAttr
    (
      container,
      doc,
      "drawType",
      std::to_string(static_cast<int> (drawType))
    );

    WriteAttr
    (
      container,
      doc,
      "vertexLayout",
      std::to_string(static_cast<int> (vertexLayout))
    );

    WriteAttr
    (
      container,
      doc,
      "priority",
      std::to_string(priority)
    );
  }

  void RenderState::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    if (parent == nullptr)
    {
      return;
    }

    if (XmlNode* container = parent->first_node("renderState"))
    {
      ReadAttr(container, "cullMode", *reinterpret_cast<int*> (&cullMode));

      ReadAttr
      (
        container,
        "depthTest",
        *reinterpret_cast<int*> (&depthTestEnabled)
      );

      ReadAttr
      (
        container,
        "blendFunction",
        *reinterpret_cast<int*> (&blendFunction)
      );

      ReadAttr
      (
        container,
        "drawType",
        *reinterpret_cast<int*> (&drawType)
      );

      ReadAttr
      (
        container,
        "vertexLayout",
        *reinterpret_cast<int*> (&vertexLayout)
      );

      ReadAttr(container, "vertexLayout", priority);
    }
  }

}  // namespace ToolKit
