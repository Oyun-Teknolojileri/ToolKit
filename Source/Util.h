#pragma once

#include "MathUtil.h"
#include "Types.h"

#include <assert.h>
#include <string.h>
#include <vector>

namespace ToolKit
{

	void ExtractXYFromNode(void* node, Vec2& val);
	void ExtractXYZFromNode(void* node, Vec3& val);
	void ExtractXYZFromNode(void* node, glm::ivec3& val);
	void ExtractWXYZFromNode(void* node, Vec4& val);
	void ExtractWXYZFromNode(void* node, glm::uvec4& val);
	void ExtractWXYZFromNode(void* node, glm::ivec4& val);
	void ExtractQuatFromNode(void* node, Quaternion& val);
	bool CheckFile(String path);
	void Split(const String& s, const String& sep, std::vector<String>& v);
	void ReplaceStringInPlace(String& subject, const String& search, const String& replace);
	class LineBatch* CreatePlaneDebugObject(PlaneEquation plane, float size);

	template<typename T>
	void pop_front(std::vector<T>& vec)
	{
		assert(!vec.empty());
		vec.erase(vec.begin());
	}

}
