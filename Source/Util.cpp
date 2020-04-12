#include "stdafx.h"
#include "Util.h"
#include "rapidxml.hpp"
#include <fstream>
#include "DebugNew.h"

namespace ToolKit
{

	void ExtractXYFromNode(void* nodev, glm::vec2& val)
	{
		rapidxml::xml_node<>* node = (rapidxml::xml_node<>*) nodev;
		rapidxml::xml_attribute<>* attr = node->first_attribute("x");
		val.x = (float)std::atof(attr->value());

		attr = node->first_attribute("y");
		val.y = (float)std::atof(attr->value());
	}

	void ExtractXYZFromNode(void* nodev, glm::vec3& val)
	{
		rapidxml::xml_node<>* node = (rapidxml::xml_node<>*) nodev;
		rapidxml::xml_attribute<>* attr = node->first_attribute("x");
		val.x = (float)std::atof(attr->value());

		attr = node->first_attribute("y");
		val.y = (float)std::atof(attr->value());

		attr = node->first_attribute("z");
		val.z = (float)std::atof(attr->value());
	}

	void ExtractXYZFromNode(void* nodev, glm::ivec3& val)
	{
		rapidxml::xml_node<>* node = (rapidxml::xml_node<>*) nodev;
		rapidxml::xml_attribute<>* attr = node->first_attribute("x");
		val.x = std::atoi(attr->value());

		attr = node->first_attribute("y");
		val.y = std::atoi(attr->value());

		attr = node->first_attribute("z");
		val.z = std::atoi(attr->value());
	}

	void ExtractWXYZFromNode(void* nodev, glm::vec4& val)
	{
		rapidxml::xml_node<>* node = (rapidxml::xml_node<>*) nodev;

		rapidxml::xml_attribute<>* attr = node->first_attribute("x");
		val.x = (float)std::atof(attr->value());

		attr = node->first_attribute("y");
		val.y = (float)std::atof(attr->value());

		attr = node->first_attribute("z");
		val.z = (float)std::atof(attr->value());

		attr = node->first_attribute("w");
		val.w = (float)std::atof(attr->value());
	}

	void ExtractWXYZFromNode(void* nodev, glm::uvec4& val)
	{
		rapidxml::xml_node<>* node = (rapidxml::xml_node<>*) nodev;

		rapidxml::xml_attribute<>* attr = node->first_attribute("x");
		val.x = std::atoi(attr->value());

		attr = node->first_attribute("y");
		val.y = std::atoi(attr->value());

		attr = node->first_attribute("z");
		val.z = std::atoi(attr->value());

		attr = node->first_attribute("w");
		val.w = std::atoi(attr->value());
	}

	void ExtractWXYZFromNode(void* nodev, glm::ivec4& val)
	{
		rapidxml::xml_node<>* node = (rapidxml::xml_node<>*) nodev;

		rapidxml::xml_attribute<>* attr = node->first_attribute("x");
		val.x = std::atoi(attr->value());

		attr = node->first_attribute("y");
		val.y = std::atoi(attr->value());

		attr = node->first_attribute("z");
		val.z = std::atoi(attr->value());

		attr = node->first_attribute("w");
		val.w = std::atoi(attr->value());
	}

	void ExtractQuatFromNode(void* nodev, glm::quat& val)
	{
		rapidxml::xml_node<>* node = (rapidxml::xml_node<>*) nodev;

		glm::vec4 tmp;
		ExtractWXYZFromNode(node, tmp);
		val = glm::quat(tmp.w, tmp.xyz);
	}

	bool CheckFile(std::string path)
	{
		std::ifstream f(path.c_str());
		return f.good();
	}

	void Split(const std::string& s, const std::string& sep, std::vector<std::string>& v)
	{
		typedef std::string::const_iterator iter;
		iter b = s.begin(), e = s.end(), i;
		iter sep_b = sep.begin(), sep_e = sep.end();

		// search through s
		while (b != e)
		{
			i = search(b, e, sep_b, sep_e);

			// no more separator found
			if (i == e)
			{
				// it's not an empty string
				if (b != e)
				{
					v.push_back(std::string(b, e));
				}
				break;
			}
			else if (i == b)
			{
				// the separator is found and right at the beginning
				// in this case, we need to move on and search for the
				// next separator
				b = i + sep.length();
			}
			else
			{
				// found the separator
				v.push_back(std::string(b, i));
				b = i;
			}
		}
	}

}
