#pragma once

#include "MathUtil.h"
#include "Types.h"

#include <assert.h>
#include <string.h>
#include <vector>

namespace ToolKit
{

	void ExtractXYFromNode(XmlNode* node, Vec2& val);
	void ExtractXYZFromNode(XmlNode* node, Vec3& val);
	void ExtractXYZFromNode(XmlNode* node, glm::ivec3& val);
	void ExtractWXYZFromNode(XmlNode* node, Vec4& val);
	void ExtractWXYZFromNode(XmlNode* node, glm::uvec4& val);
	void ExtractWXYZFromNode(XmlNode* node, glm::ivec4& val);
	void ExtractQuatFromNode(XmlNode* node, Quaternion& val);
	void WriteXY(XmlNode* node, XmlDocument* doc, const Vec2& val);
	void WriteXYZ(XmlNode* node, XmlDocument* doc, const Vec3& val);
	void WriteXYZW(XmlNode* node, XmlDocument* doc, const Vec4& val);
	void WriteXYZW(XmlNode* node, XmlDocument* doc, const Quaternion& val);
	void WriteAttr(XmlNode* node, XmlDocument* doc, const String& name, const String& val);
	bool CheckFile(const String& path);
	void DecomposePath(const String fullPath, String* path, String* name, String* ext);
	void NormalizePath(String& path);
	void Split(const String& s, const String& sep, StringArray& v);
	void ReplaceStringInPlace(String& subject, const String& search, const String& replace);
	class LineBatch* CreatePlaneDebugObject(PlaneEquation plane, float size);
	class LineBatch* GenerateBoundingVolumeGeometry(const BoundingBox& box, Mat4* transform = nullptr);
	void ToEntityIdArray(EntityIdArray& idArray, const EntityRawPtrArray& ptrArray);

	template<typename T>
	void pop_front(std::vector<T>& vec)
	{
		assert(!vec.empty());
		vec.erase(vec.begin());
	}

}
