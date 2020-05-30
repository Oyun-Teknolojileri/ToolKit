#include "stdafx.h"

#include "ParameterBlock.h"
#include "Util.h"

namespace ToolKit
{

	static const String XmlParamterStr("P");

	void ParameterVariant::Serialize(XmlDocument* doc, XmlNode* parent) const
	{
		XmlNode* node = doc->allocate_node(rapidxml::node_element, XmlParamterStr.c_str());

		switch (m_type)
		{
    case VariantType::Byte:
		{
			WriteAttr(node, doc, XmlParamterStr.c_str(), std::to_string(GetVar<Byte>()));
		}
		break;
    case VariantType::UByte:
		{
			WriteAttr(node, doc, XmlParamterStr.c_str(), std::to_string(GetVar<UByte>()));
		}
		break;
    case VariantType::Float:
		{
			WriteAttr(node, doc, XmlParamterStr.c_str(), std::to_string(GetVar<float>()));
		}
    break;
    case VariantType::Int:
		{
			WriteAttr(node, doc, XmlParamterStr.c_str(), std::to_string(GetVar<int>()));
		}
      break;
    case VariantType::UInt:
		{
			WriteAttr(node, doc, XmlParamterStr.c_str(), std::to_string(GetVar<uint>()));
		}
      break;
    case VariantType::Vec3:
    {
      WriteXYZ(node, doc, GetVar<Vec3>());
    }
      break;
    case VariantType::Vec4:
		{
			WriteXYZ(node, doc, GetVar<Vec4>());
		}
      break;
    case VariantType::Mat3:
    case VariantType::Mat4:
      assert(false && "Not implemented.");
      break;
    case VariantType::String:
		{
			WriteAttr(node, doc, XmlParamterStr.c_str(), GetVar<String>());
		}
      break;
    default:
      assert(false && "Invalid type.");
      break;
		}

		parent->append_node(node);
	}

}

