#pragma once

#include "Glm/glm.hpp"
#include "Glm/gtc/quaternion.hpp"
#include <string>
#include <memory>

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
	typedef glm::mat3 Mat3;
	typedef glm::quat Quaternion;
	typedef std::shared_ptr<class Material> MaterialPtr;
	typedef std::shared_ptr<class Mesh> MeshPtr;

	static const Vec3 X_AXIS = Vec3(1.0f, 0.0f, 0.0f);
	static const Vec3 Y_AXIS = Vec3(0.0f, 1.0f, 0.0f);
	static const Vec3 Z_AXIS = Vec3(0.0f, 0.0f, 1.0f);
	static const Vec3 XY_AXIS = Vec3(1.0f, 1.0f, 0.0f);
	static const Vec3 YZ_AXIS = Vec3(0.0f, 1.0f, 1.0f);
	static const Vec3 ZX_AXIS = Vec3(1.0f, 0.0f, 1.0f);
	static const Vec3 AXIS[6] = { X_AXIS, Y_AXIS, Z_AXIS, XY_AXIS, YZ_AXIS, ZX_AXIS };

	static const EntityId NULL_ENTITY = 0;

	enum class AxisLabel
	{
		None = -1, // Order matters. Don't change.
		X,
		Y,
		Z,
		XY,
		YZ,
		ZX
	};

}