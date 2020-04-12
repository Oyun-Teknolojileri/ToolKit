#pragma once

#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"

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

	// split a string into multiple sub strings, based on a separator string
	// for example, if separator="::",
	// s = "abc::def xy::st:" -> "abc", "def xy" and "st:",
	// https://stackoverflow.com/questions/53849/how-do-i-tokenize-a-string-in-c?page=2&tab=votes#tab-top
	void Split(const std::string& s, const std::string& sep, std::vector<std::string>& v);

}
