#pragma once

#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"

void ExtractXYFromNode(void* node, glm::vec2& val);
void ExtractXYZFromNode(void* node, glm::vec3& val);
void ExtractXYZFromNode(void* node, glm::ivec3& val);
void ExtractWXYZFromNode(void* node, glm::vec4& val);
void ExtractWXYZFromNode(void* node, glm::uvec4& val);
void ExtractWXYZFromNode(void* node, glm::ivec4& val);
void ExtractQuatFromNode(void* node, glm::quat& val);
bool CheckFile(std::string path);
