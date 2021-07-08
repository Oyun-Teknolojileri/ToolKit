#pragma once

#include "Glm/glm.hpp"
#include "Glm/gtc/quaternion.hpp"
#include <vector>
#include <string>
#include <memory>

namespace rapidxml
{
  template<class Ch> class xml_document;
  template<class Ch> class xml_node;
  template<class Ch> class xml_attribute;
  template<class Ch> class file;
}

namespace ToolKit
{
#define SafeDel(ptr) { delete ptr; ptr = nullptr; };
#define SafeDelArray(ptr) { delete[] ptr; ptr = nullptr; };

  typedef char Byte;
  typedef unsigned char UByte;
  typedef unsigned int uint;
  typedef unsigned char uint8;
  typedef unsigned long EntityId;
  typedef unsigned long NodeId;
  typedef unsigned long SceneId;
  typedef const int SignalId;
  typedef std::shared_ptr<class Resource> ResourcePtr;
  typedef std::string String;
  typedef std::vector<String> StringArray;
  typedef glm::ivec2 IVec2;
  typedef glm::vec2 Vec2;
  typedef std::vector<Vec2> Vec2Array;
  typedef glm::vec3 Vec3;
  typedef std::vector<Vec3> Vec3Array;
  typedef glm::vec4 Vec4;
  typedef std::vector<Vec4> Vec4Array;
  typedef glm::uvec4 UVec4;
  typedef glm::mat4 Mat4;
  typedef glm::mat3 Mat3;
  typedef glm::quat Quaternion;
  typedef std::shared_ptr<class Animation> AnimationPtr;
  typedef std::shared_ptr<class Material> MaterialPtr;
  typedef std::shared_ptr<class Texture> TexturePtr;
  typedef std::shared_ptr<class RenderTarget> RenderTargetPtr;
  typedef std::shared_ptr<class SpriteSheet> SpriteSheetPtr;
  typedef std::shared_ptr<class Mesh> MeshPtr;
  typedef std::shared_ptr<class Shader> ShaderPtr;
  typedef std::vector<ShaderPtr> ShaderPtrArray;
  typedef std::shared_ptr<class Program> ProgramPtr;
  typedef std::shared_ptr<class SkinMesh> SkinMeshPtr;
  typedef std::vector<MeshPtr> MeshPtrArray;
  typedef std::vector<class Mesh*> MeshRawPtrArray;
  typedef std::vector<const class Mesh*> MeshRawCPtrArray;
  typedef std::vector<class Entity*> EntityRawPtrArray;
  typedef std::vector<class Light*> LightRawPtrArray;
  typedef std::vector<std::shared_ptr<class Entity>> EntityPtrArray;
  typedef std::vector<EntityId> EntityIdArray;
  typedef std::vector<class Node*> NodePtrArray;
  typedef std::vector<class Vertex> VertexArray;
  typedef std::vector<class Face> FaceArray;
  typedef rapidxml::xml_document<char> XmlDocument;
  typedef rapidxml::xml_node<char> XmlNode;
  typedef rapidxml::xml_attribute<char> XmlAttribute;
  typedef rapidxml::file<char> XmlFile;

  static const Vec3 X_AXIS = Vec3(1.0f, 0.0f, 0.0f);
  static const Vec3 Y_AXIS = Vec3(0.0f, 1.0f, 0.0f);
  static const Vec3 Z_AXIS = Vec3(0.0f, 0.0f, 1.0f);
  static const Vec3 XY_AXIS = Vec3(1.0f, 1.0f, 0.0f);
  static const Vec3 YZ_AXIS = Vec3(0.0f, 1.0f, 1.0f);
  static const Vec3 ZX_AXIS = Vec3(1.0f, 0.0f, 1.0f);
  static const Vec3 AXIS[6] = { X_AXIS, Y_AXIS, Z_AXIS, XY_AXIS, YZ_AXIS, ZX_AXIS };

  // Supported file formats.
  static const String FBX(".fbx");
  static const String GLTF(".gltf");
  static const String GLB(".glb");
  static const String OBJ(".obj");
  static const String PNG(".png");
  static const String JPG(".jpg");
  static const String JPEG(".jpeg");
  static const String TGA(".tga");
  static const String BMP(".bmp");
  static const String PSD(".psd");

  // Local formats.
  static const String SCENE(".scene");
  static const String MESH(".mesh");
  static const String ANIM(".anim");
  static const String SKINMESH(".skinMesh");
  static const String SKELETON(".skeleton");
  static const String MATERIAL(".material");
  static const String SHADER(".shader");
  static const String AUDIO(".wav");

  static const EntityId NULL_ENTITY = 0;

  // Xml file IO.
  const static String XmlEntityElement("E");
  const static String XmlEntityIdAttr("i");
  const static String XmlParentEntityIdAttr("pi");
  const static String XmlEntityNameAttr("n");
  const static String XmlEntityTagAttr("ta");
  const static String XmlEntityTypeAttr("t");
  const static String XmlSceneElement("S");
  const static String XmlParamterElement("P");
  const static String XmlParamterValAttr("v");
  const static String XmlParamterTypeAttr("t");
  const static String XmlParamBlockElement("PB");
  static const String XmlMeshElement("M");
  static const String XmlFileAttr("f");
  static const String XmlNodeElement("N");
  static const String XmlNodeIdAttr("i");
  static const String XmlNodeParentIdAttr("pi");
  static const String XmlNodeInheritScaleAttr("is");
  static const String XmlNodeInheritTranslateOnlyAttr("ito");
  static const String XmlTranslateElement("T");
  static const String XmlRotateElement("R");
  static const String XmlScaleElement("S");

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