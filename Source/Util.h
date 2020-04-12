#pragma once

#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"
#include "MathUtil.h"

#include <assert.h>
#include <string.h>
#include <vector>

namespace ToolKit
{

	void ExtractXYFromNode(void* node, glm::vec2& val);
	void ExtractXYZFromNode(void* node, glm::vec3& val);
	void ExtractXYZFromNode(void* node, glm::ivec3& val);
	void ExtractWXYZFromNode(void* node, glm::vec4& val);
	void ExtractWXYZFromNode(void* node, glm::uvec4& val);
	void ExtractWXYZFromNode(void* node, glm::ivec4& val);
	void ExtractQuatFromNode(void* node, glm::quat& val);
	bool CheckFile(std::string path);
	void Split(const std::string& s, const std::string& sep, std::vector<std::string>& v);
	class LineBatch* CreatePlaneDebugObject(PlaneEquation plane, float size);

	template<typename T>
	void pop_front(std::vector<T>& vec)
	{
		assert(!vec.empty());
		vec.erase(vec.begin());
	}

}
