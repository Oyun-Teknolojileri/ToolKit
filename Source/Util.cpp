#include "stdafx.h"
#include "Util.h"
#include "rapidxml.hpp"
#include "Primative.h"
#include "DebugNew.h"

#include <fstream>

namespace ToolKit
{

	void ExtractXYFromNode(XmlNode* node, Vec2& val)
	{
		XmlAttribute* attr = node->first_attribute("x");
		val.x = (float)std::atof(attr->value());

		attr = node->first_attribute("y");
		val.y = (float)std::atof(attr->value());
	}

	void ExtractXYZFromNode(XmlNode* node, Vec3& val)
	{
		XmlAttribute* attr = node->first_attribute("x");
		val.x = (float)std::atof(attr->value());

		attr = node->first_attribute("y");
		val.y = (float)std::atof(attr->value());

		attr = node->first_attribute("z");
		val.z = (float)std::atof(attr->value());
	}

	void ExtractXYZFromNode(XmlNode* node, glm::ivec3& val)
	{
		XmlAttribute* attr = node->first_attribute("x");
		val.x = std::atoi(attr->value());

		attr = node->first_attribute("y");
		val.y = std::atoi(attr->value());

		attr = node->first_attribute("z");
		val.z = std::atoi(attr->value());
	}

	void ExtractWXYZFromNode(XmlNode* node, Vec4& val)
	{
		XmlAttribute* attr = node->first_attribute("x");
		val.x = (float)std::atof(attr->value());

		attr = node->first_attribute("y");
		val.y = (float)std::atof(attr->value());

		attr = node->first_attribute("z");
		val.z = (float)std::atof(attr->value());

		attr = node->first_attribute("w");
		val.w = (float)std::atof(attr->value());
	}

	void ExtractWXYZFromNode(XmlNode* node, glm::uvec4& val)
	{
		XmlAttribute* attr = node->first_attribute("x");
		val.x = std::atoi(attr->value());

		attr = node->first_attribute("y");
		val.y = std::atoi(attr->value());

		attr = node->first_attribute("z");
		val.z = std::atoi(attr->value());

		attr = node->first_attribute("w");
		val.w = std::atoi(attr->value());
	}

	void ExtractWXYZFromNode(XmlNode* node, glm::ivec4& val)
	{
		XmlAttribute* attr = node->first_attribute("x");
		val.x = std::atoi(attr->value());

		attr = node->first_attribute("y");
		val.y = std::atoi(attr->value());

		attr = node->first_attribute("z");
		val.z = std::atoi(attr->value());

		attr = node->first_attribute("w");
		val.w = std::atoi(attr->value());
	}

	void ExtractQuatFromNode(XmlNode* node, Quaternion& val)
	{
		Vec4 tmp;
		ExtractWXYZFromNode(node, tmp);
		val = Quaternion(tmp.w, tmp.xyz);
	}

	void WriteXY(XmlNode* node, XmlDocument* doc, const Vec2& val)
	{
		WriteAttr(node, doc, "x", std::to_string(val.x));
		WriteAttr(node, doc, "y", std::to_string(val.y));
	}

	void WriteXYZ(XmlNode* node, XmlDocument* doc, const Vec3& val)
	{
		WriteXY(node, doc, val.xy);
		WriteAttr(node, doc, "z", std::to_string(val.z));
	}

	void WriteXYZW(XmlNode* node, XmlDocument* doc, const Vec4& val)
	{
		WriteXYZ(node, doc, val.xyz);
		WriteAttr(node, doc, "w", std::to_string(val.w));
	}

	void WriteXYZW(XmlNode* node, XmlDocument* doc, const Quaternion& val)
	{
		Vec4 dummy(val.x, val.y, val.z, val.w);
		WriteXYZW(node, doc, dummy);
	}

	void WriteAttr(XmlNode* node, XmlDocument* doc, const String& name, const String& val)
	{
		node->append_attribute
		(
			doc->allocate_attribute
			(
				doc->allocate_string(name.c_str(), 0),
				doc->allocate_string(val.c_str(), 0)
			)
		);
	}

	template<typename T>
	T ReadAttr(XmlNode* node, const String& name)
	{
		if (XmlAttribute* attr = node->first_attribute(name.c_str()))
		{
			return (T)std::atoi(attr->value());
		}

		return (T)0;
	}

	template Byte ReadAttr<Byte>(XmlNode* node, const String& name);
	template UByte ReadAttr<UByte>(XmlNode* node, const String& name);
	template int ReadAttr<int>(XmlNode* node, const String& name);
	template uint ReadAttr<uint>(XmlNode* node, const String& name);
	template float ReadAttr<float>(XmlNode* node, const String& name);

	bool CheckFile(const String& path)
	{
		std::ifstream f(path.c_str());
		return f.good();
	}

	void DecomposePath(const String fullPath, String* path, String* name, String* ext)
	{
		String normal = fullPath;
		NormalizePath(normal);

		size_t ind1 = normal.find_last_of('\\');
		if (path != nullptr)
		{
			*path = normal.substr(0, ind1);
		}

		size_t ind2 = normal.find_last_of('.');
		if (ind2 != String::npos)
		{
			if (name != nullptr)
			{
				*name = normal.substr(ind1 + 1, ind2 - ind1 - 1);
			}
			
			if (ext != nullptr)
			{
				*ext = normal.substr(ind2);
			}
		}
	}

	void NormalizePath(String& path)
	{
		ReplaceStringInPlace(path, "/", "\\");
	}

	// split a string into multiple sub strings, based on a separator string
	// for example, if separator="::",
	// s = "abc::def xy::st:" -> "abc", "def xy" and "st:",
	// https://stackoverflow.com/questions/53849/how-do-i-tokenize-a-string-in-c?page=2&tab=votes#tab-top
	void Split(const String& s, const String& sep, StringArray& v)
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

		NormalizePlaneEquation(plane);
		Vec3 o = plane.normal * plane.d;

		float hSize = size * 0.5f;
		Vec3Array corners
		{
			o + x * hSize + y * hSize,
			o - x * hSize + y * hSize,
			o - x * hSize - y * hSize,
			o + x * hSize - y * hSize
		};

		LineBatch* obj = new LineBatch(corners, X_AXIS, DrawType::LineLoop, 5.0f);
		return obj;
	}

	class LineBatch* CreateLineDebugObject(const Vec3Array& corners)
	{
		LineBatch* obj = new LineBatch(corners, X_AXIS, DrawType::LineLoop, 5.0f);
		return obj;
	}

	ToolKit::LineBatch* GenerateBoundingVolumeGeometry(const BoundingBox& box, Mat4* transform)
	{
		Vec3 scale = box.max - box.min;
		Cube cube(scale);

		std::vector<Vec3> vertices =
		{
			Vec3(-0.5f, 0.5f, 0.5f) * scale, // FTL.
			Vec3(-0.5f, -0.5f, 0.5f) * scale, // FBL.
			Vec3(0.5f, -0.5f, 0.5f) * scale, // FBR.
			Vec3(0.5f, 0.5f, 0.5f) * scale, // FTR.
			Vec3(-0.5f, 0.5f, 0.5f) * scale, // FTL.
			Vec3(-0.5f, 0.5f, -0.5f) * scale, // BTL.
			Vec3(-0.5f, -0.5f, -0.5f) * scale, // BBL.
			Vec3(0.5f, -0.5f, -0.5f) * scale, // BBR.
			Vec3(0.5f, 0.5f, -0.5f) * scale, // BTR.
			Vec3(-0.5f, 0.5f, -0.5f) * scale, // BTL.
			Vec3(0.5f, 0.5f, -0.5f) * scale, // BTR.
			Vec3(0.5f, 0.5f, 0.5f) * scale, // FTR.
			Vec3(0.5f, -0.5f, 0.5f) * scale, // FBR.
			Vec3(0.5f, -0.5f, -0.5f) * scale, // BBR.
			Vec3(-0.5f, -0.5f, -0.5f) * scale, // BBL.
			Vec3(-0.5f, -0.5f, 0.5f) * scale // FBL.
		};

		Vec3 mid = (box.min + box.max) * 0.5f;
		for (Vec3& v : vertices)
		{
			v += mid;
			if (transform != nullptr)
			{
				v = *transform * Vec4(v, 1.0f);
			}
		}

		LineBatch* lineForm = new LineBatch(vertices, X_AXIS, DrawType::LineStrip, 2.0f);
		return lineForm;
	}

	void ToEntityIdArray(EntityIdArray& idArray, const EntityRawPtrArray& ptrArray)
	{
		idArray.reserve(ptrArray.size());
		for (Entity* ntt : ptrArray)
		{
			idArray.push_back(ntt->m_id);
		}
	}

}
