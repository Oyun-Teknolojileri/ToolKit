#pragma once

#include "Glm/glm.hpp"
#include <string>

namespace ToolKit
{
#define SafeDel(ptr) {delete ptr; ptr = nullptr;}
#define SafeDelArray(ptr) {delete [] ptr; ptr = nullptr;}

	typedef unsigned int uint;
	typedef unsigned char uint8;
	typedef unsigned long EntityId;
	typedef const int SignalId;
	typedef std::string String;
	typedef glm::vec2 Vec2;
	typedef glm::vec3 Vec3;
	typedef glm::vec4 Vec4;
	typedef glm::mat4 Mat4;

	static const Vec3 X_AXIS = Vec3(1.0f, 0.0f, 0.0f);
	static const Vec3 Y_AXIS = Vec3(0.0f, 1.0f, 0.0f);
	static const Vec3 Z_AXIS = Vec3(0.0f, 0.0f, 1.0f);
	static const Vec3 AXIS[3] = { X_AXIS, Y_AXIS, Z_AXIS };

	static const EntityId NULL_ENTITY = 0;

	enum class AxisLabel
	{
		None = -1, // Order matters. Don't change.
		X,
		Y,
		Z,
		XY,
		XZ,
		YZ
	};

}