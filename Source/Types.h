#pragma once

#include "Glm/glm.hpp"
#include "Glm/gtc/quaternion.hpp"
#include <vector>
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
	typedef std::shared_ptr<class SkinMesh> SkinMeshPtr;
	typedef std::vector<MeshPtr> MeshPtrArray;
	typedef std::vector<class Mesh*> MeshRawPtrArray;
	typedef std::vector<class Entity*> EntityRawPtrArray;
	typedef std::vector<std::shared_ptr<class Entity>> EntitySharedPtrArray;
	typedef std::vector<EntityId> EntityIdArray;
	typedef std::vector<class Node*> NodePtrArray;
	typedef std::vector<class Vertex> VertexArray;
	typedef std::vector<String> StringArray;

	static const Vec3 X_AXIS = Vec3(1.0f, 0.0f, 0.0f);
	static const Vec3 Y_AXIS = Vec3(0.0f, 1.0f, 0.0f);
	static const Vec3 Z_AXIS = Vec3(0.0f, 0.0f, 1.0f);
	static const Vec3 XY_AXIS = Vec3(1.0f, 1.0f, 0.0f);
	static const Vec3 YZ_AXIS = Vec3(0.0f, 1.0f, 1.0f);
	static const Vec3 ZX_AXIS = Vec3(1.0f, 0.0f, 1.0f);
	static const Vec3 AXIS[6] = { X_AXIS, Y_AXIS, Z_AXIS, XY_AXIS, YZ_AXIS, ZX_AXIS };

	// Supported file formats.
	static const String FBX(".fbx");
	static const String glTF(".glb");
	static const String OBJ(".obj");
	static const String PNG(".png");
	// Local formats.
	static const String MESH(".mesh");
	static const String ANIM(".anim");
	static const String SKINMESH(".skinMesh");
	static const String SKELETON(".skeleton");
	static const String MATERIAL(".material");

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

	static const float TK_FLT_MAX = std::numeric_limits<float>::max();

}