#pragma once

namespace ToolKit
{
#define SafeDel(ptr) {delete ptr; ptr = nullptr;}
#define SafeDelArray(ptr) {delete [] ptr; ptr = nullptr;}

	typedef unsigned int uint;
	typedef unsigned char uint8;
	typedef unsigned long EntityId;
	typedef const int SignalId;

	extern std::string TexturePath(std::string file);
	extern std::string MeshPath(std::string file);
	extern std::string FontPath(std::string file);
	extern std::string SpritePath(std::string file);
	extern std::string AudioPath(std::string file);
	extern std::string AnimationPath(std::string file);
	extern std::string SkeletonPath(std::string file);
	extern std::string ShaderPath(std::string file);
	extern std::string MaterialPath(std::string file);

	static const glm::vec3 X_AXIS = glm::vec3(1.0f, 0.0f, 0.0f);
	static const glm::vec3 Y_AXIS = glm::vec3(0.0f, 1.0f, 0.0f);
	static const glm::vec3 Z_AXIS = glm::vec3(0.0f, 0.0f, 1.0f);

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