#pragma once

namespace ToolKit
{
#define SafeDel(ptr) {delete ptr; ptr = nullptr;}
#define SafeDelArray(ptr) {delete [] ptr; ptr = nullptr;}

	typedef unsigned int uint;
	typedef unsigned char uint8;
	typedef unsigned long EntityId;
	typedef const int SignalId;

	static const glm::vec3 X_AXIS = glm::vec3(1.0f, 0.0f, 0.0f);
	static const glm::vec3 Y_AXIS = glm::vec3(0.0f, 1.0f, 0.0f);
	static const glm::vec3 Z_AXIS = glm::vec3(0.0f, 0.0f, 1.0f);
	static const glm::vec3 AXIS[3] = { X_AXIS, Y_AXIS, Z_AXIS };

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