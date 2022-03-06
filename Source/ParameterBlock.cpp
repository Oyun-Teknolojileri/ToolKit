#include "stdafx.h"

#include "ParameterBlock.h"
#include "Util.h"
#include "DebugNew.h"

namespace ToolKit
{

  void ParameterVariant::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* node = doc->allocate_node(rapidxml::node_element, XmlParamterElement.c_str());
    WriteAttr(node, doc, XmlParamterTypeAttr, std::to_string((int)m_type));
    WriteAttr(node, doc, "name", m_name);

    switch (m_type)
    {
    case VariantType::byte:
    {
      WriteAttr(node, doc, XmlParamterValAttr.c_str(), std::to_string(GetVar<byte>()));
    }
    break;
    case VariantType::ubyte:
    {
      WriteAttr(node, doc, XmlParamterValAttr.c_str(), std::to_string(GetVar<ubyte>()));
    }
    break;
    case VariantType::Float:
    {
      WriteAttr(node, doc, XmlParamterValAttr.c_str(), std::to_string(GetVar<float>()));
    }
    break;
    case VariantType::Int:
    {
      WriteAttr(node, doc, XmlParamterValAttr.c_str(), std::to_string(GetVar<int>()));
    }
    break;
    case VariantType::UInt:
    {
      WriteAttr(node, doc, XmlParamterValAttr.c_str(), std::to_string(GetVar<uint>()));
    }
    break;
    case VariantType::Vec3:
    {
      WriteVec(node, doc, GetVar<Vec3>());
    }
    break;
    case VariantType::Vec4:
    {
      WriteVec(node, doc, GetVar<Vec4>());
    }
    break;
    case VariantType::Mat3:
    case VariantType::Mat4:
      assert(false && "Not implemented.");
      break;
    case VariantType::String:
    {
      WriteAttr(node, doc, XmlParamterValAttr.c_str(), GetVar<String>());
    }
    break;
    default:
      assert(false && "Invalid type.");
      break;
    }

    parent->append_node(node);
  }

  void ParameterVariant::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    if (parent == nullptr)
    {
      assert(false && "Unbound parameter can not exist");
      return;
    }

    XmlAttribute* attr = parent->first_attribute(XmlParamterTypeAttr.c_str());
    m_type = (VariantType)std::atoi(attr->value());
    ReadAttr(parent, "name", m_name);

    switch (m_type)
    {
    case VariantType::byte:
    {
      byte val(0);
      ReadAttr(parent, XmlParamterValAttr, val);
      m_var = val;
    }
    break;
    case VariantType::ubyte:
    {
      ubyte val(0);
      ReadAttr(parent, XmlParamterValAttr, val);
      m_var = val;
    }
    break;
    case VariantType::Float:
    {
      float val(0);
      ReadAttr(parent, XmlParamterValAttr, val);
      m_var = val;
    }
    break;
    case VariantType::Int:
    {
      int val(0);
      ReadAttr(parent, XmlParamterValAttr, val);
      m_var = val;
    }
    break;
    case VariantType::UInt:
    {
      int val(0);
      ReadAttr(parent, XmlParamterValAttr, val);
      m_var = val;
    }
    break;
    case VariantType::Vec3:
    {
      Vec3 var;
      ReadVec(parent, var);
      m_var = var;
    }
    break;
    case VariantType::Vec4:
    {
      Vec4 var;
      ReadVec(parent, var);
      m_var = var;
    }
    break;
    case VariantType::Mat3:
    case VariantType::Mat4:
    case VariantType::String:
      assert(false && "Not implemented.");
      break;
    default:
      assert(false && "Invalid type.");
      break;
    }
  }

  void ParameterBlock::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* blockNode = doc->allocate_node(rapidxml::node_element, XmlParamBlockElement.c_str());
    for (const ParameterVariant& var : m_variants)
    {
      var.Serialize(doc, blockNode);
    }
    parent->append_node(blockNode);
  }

  void ParameterBlock::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    if (XmlNode* block = parent->first_node(XmlParamBlockElement.c_str()))
    {
      XmlNode* param = block->first_node(XmlParamterElement.c_str());
      while (param != nullptr)
      {
        ParameterVariant var;
        var.DeSerialize(doc, param);
        m_variants.push_back(var);
        param = param->next_sibling();
      }
    }
  }

}

