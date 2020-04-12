#include "stdafx.h"
#include "Util.h"
#include "rapidxml.hpp"
#include "Primative.h"
#include "DebugNew.h"

#include <fstream>

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

	// split a string into multiple sub strings, based on a separator string
	// for example, if separator="::",
	// s = "abc::def xy::st:" -> "abc", "def xy" and "st:",
	// https://stackoverflow.com/questions/53849/how-do-i-tokenize-a-string-in-c?page=2&tab=votes#tab-top
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

	ToolKit::LineBatch* CreatePlaneDebugObject(PlaneEquation plane, float size)
	{
		// Searching perpendicular axes on the plane.
		glm::vec3 z = plane.normal;
		glm::vec3 x = z + glm::vec3(1.0f);
		glm::vec3 y = glm::cross(z, x);
		x = glm::normalize(glm::cross(y, z));
		y = glm::normalize(glm::cross(z, x));

		NormalzePlaneEquation(plane);
		glm::vec3 o = plane.normal * plane.d;

		float hSize = size * 0.5f;
		std::vector<glm::vec3> corners
		{
			o + x * hSize + y * hSize,
			o - x * hSize + y * hSize,
			o - x * hSize - y * hSize,
			o + x * hSize - y * hSize
		};

		LineBatch* obj = new LineBatch(corners, X_AXIS, DrawType::LineLoop, 5.0f);
		return obj;
	}

}
