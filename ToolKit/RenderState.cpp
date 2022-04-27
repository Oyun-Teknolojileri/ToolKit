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

    WriteAttr(container, doc, "cullMode", std::to_string((int)cullMode));
    WriteAttr(container, doc, "depthTest", std::to_string((int)depthTestEnabled));
    WriteAttr(container, doc, "blendFunction", std::to_string((int)blendFunction));
    WriteAttr(container, doc, "drawType", std::to_string((int)drawType));
    WriteAttr(container, doc, "vertexLayout", std::to_string((int)vertexLayout));
  }

  void RenderState::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    if (parent == nullptr)
    {
      return;
    }

    if (XmlNode* container = parent->first_node("renderState"))
    {
      ReadAttr(container, "cullMode", *(int*)&cullMode);
      ReadAttr(container, "depthTest", *(int*)&depthTestEnabled);
      ReadAttr(container, "blendFunction", *(int*)&blendFunction);
      ReadAttr(container, "drawType", *(int*)&drawType);
      ReadAttr(container, "vertexLayout", *(int*)&vertexLayout);
    }
  }

}
