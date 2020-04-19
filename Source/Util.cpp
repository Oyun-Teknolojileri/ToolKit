#include "stdafx.h"
#include "Util.h"
#include "rapidxml.hpp"
#include "Primative.h"
#include "DebugNew.h"

#include <fstream>

namespace ToolKit
{

	void ExtractXYFromNode(void* nodev, Vec2& val)
	{
		rapidxml::xml_node<>* node = (rapidxml::xml_node<>*) nodev;
		rapidxml::xml_attribute<>* attr = node->first_attribute("x");
		val.x = (float)std::atof(attr->value());

		attr = node->first_attribute("y");
		val.y = (float)std::atof(attr->value());
	}

	void ExtractXYZFromNode(void* nodev, Vec3& val)
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

	void ExtractWXYZFromNode(void* nodev, Vec4& val)
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

	void ExtractQuatFromNode(void* nodev, Quaternion& val)
	{
		rapidxml::xml_node<>* node = (rapidxml::xml_node<>*) nodev;

		Vec4 tmp;
		ExtractWXYZFromNode(node, tmp);
		val = Quaternion(tmp.w, tmp.xyz);
	}

	bool CheckFile(String path)
	{
		std::ifstream f(path.c_str());
		return f.good();
	}

	// split a string into multiple sub strings, based on a separator string
	// for example, if separator="::",
	// s = "abc::def xy::st:" -> "abc", "def xy" and "st:",
	// https://stackoverflow.com/questions/53849/how-do-i-tokenize-a-string-in-c?page=2&tab=votes#tab-top
	void Split(const String& s, const String& sep, std::vector<String>& v)
	{
		typedef String::const_iterator iter;
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
					v.push_back(String(b, e));
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
				v.push_back(String(b, i));
				b = i;
			}
		}
	}

	// https://stackoverflow.com/questions/5878775/how-to-find-and-replace-string
	void ReplaceStringInPlace(String& subject, const String& search, const String& replace)
	{
		size_t pos = 0;
		while ((pos = subject.find(search, pos)) != std::string::npos) {
			subject.replace(pos, search.length(), replace);
			pos += replace.length();
		}
	}

	LineBatch* CreatePlaneDebugObject(PlaneEquation plane, float size)
	{
		// Searching perpendicular axes on the plane.
		Vec3 z = plane.normal;
		Vec3 x = z + Vec3(1.0f);
		Vec3 y = glm::cross(z, x);
		x = glm::normalize(glm::cross(y, z));
		y = glm::normalize(glm::cross(z, x));

		NormalzePlaneEquation(plane);
		Vec3 o = plane.normal * plane.d;

		float hSize = size * 0.5f;
		std::vector<Vec3> corners
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
