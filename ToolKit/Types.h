#pragma once

/**
* @file Forward declerations for frequenty used common ToolKit classes and
* relatled structures.
*/

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <limits>
#include <utility>
#include <set>
#include <filesystem>
#include <unordered_map>

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

#ifdef _WIN32  // Windows.
#define TK_STDCAL __stdcall
#ifdef TK_DLL_EXPORT  // Dynamic binding.
#define TK_API __declspec(dllexport)
#else  // Static binding.
#define TK_API
#endif
#else  // Other OS.
#define TK_API
#define TK_STDCAL
#endif

#define SafeDel(ptr) { delete ptr; ptr = nullptr; }
#define SafeDelArray(ptr) { delete[] ptr; ptr = nullptr; }

#define TKStringify(x) #x
#define TKToString(x) TKStringify(x)
#define TKLoc __FILE__ ":" TKToString(__LINE__)

// std template types require dll interface.
#pragma warning( disable: 4251 )

/// @cond
namespace rapidxml
{
  template<class Ch> class xml_document;
  template<class Ch> class xml_node;
  template<class Ch> class xml_attribute;
  template<class Ch> class file;
}
/// @endcond

namespace ToolKit
{

  // Primitive types.
  typedef char byte;
  typedef unsigned char ubyte;
  typedef uint32_t uint;
  typedef uint8_t uint8;
  typedef uint64_t ULongID;
  typedef const int16_t SignalId;
  typedef std::shared_ptr<class Resource> ResourcePtr;
  typedef std::string String;
  typedef std::vector<String> StringArray;
  typedef std::set<std::string> StringSet;
  typedef glm::ivec2 IVec2;
  typedef glm::vec2 Vec2;
  typedef std::vector<Vec2> Vec2Array;
  typedef glm::vec3 Vec3;
  typedef std::vector<Vec3> Vec3Array;
  typedef glm::vec4 Vec4;
  typedef std::vector<Vec4> Vec4Array;
  typedef glm::uvec4 UVec4;
  typedef glm::uvec2 UVec2;
  typedef glm::mat4 Mat4;
  typedef glm::mat3 Mat3;
  typedef glm::quat Quaternion;
  typedef std::shared_ptr<class AnimRecord> AnimRecordPtr;
  typedef std::unordered_map<String, AnimRecordPtr> AnimRecordPtrMap;
  typedef class AnimRecord* AnimRecordRawPtr;
  typedef std::vector<AnimRecordRawPtr> AnimRecordRawPtrArray;
  typedef std::vector<int> IntArray;
  typedef std::vector<struct VariantCategory> VariantCategoryArray;

  // Resource types.
  typedef std::shared_ptr<class Animation> AnimationPtr;
  typedef std::shared_ptr<class Material> MaterialPtr;
  typedef std::shared_ptr<class CubeMap> CubeMapPtr;
  typedef std::shared_ptr<class Texture> TexturePtr;
  typedef std::shared_ptr<class Hdri> HdriPtr;
  typedef std::shared_ptr<class RenderTarget> RenderTargetPtr;
  typedef std::shared_ptr<class SpriteSheet> SpriteSheetPtr;
  typedef std::shared_ptr<class Mesh> MeshPtr;
  typedef std::shared_ptr<class Skeleton> SkeletonPtr;
  typedef std::shared_ptr<class Shader> ShaderPtr;
  typedef std::vector<ShaderPtr> ShaderPtrArray;
  typedef std::shared_ptr<class Program> ProgramPtr;
  typedef std::shared_ptr<class SkinMesh> SkinMeshPtr;
  typedef std::shared_ptr<class Scene> ScenePtr;
  typedef std::vector<MeshPtr> MeshPtrArray;
  typedef std::vector<class Mesh*> MeshRawPtrArray;
  typedef std::vector<const class Mesh*> MeshRawCPtrArray;

  typedef std::vector<class Entity*> EntityRawPtrArray;
  typedef std::vector<class Light*> LightRawPtrArray;
  typedef std::vector<class DirectionalLight*> DirectionalLightRawPtrArray;
  typedef std::vector<class SpotLight*> SpotLightRawPtrArray;
  typedef std::vector<class PointLight*> PointLightRawPtrArray;
  typedef std::vector<std::shared_ptr<class Entity>> EntityPtrArray;
  typedef std::vector<ULongID> EntityIdArray;
  typedef std::vector<class Node*> NodePtrArray;
  typedef std::vector<class Vertex> VertexArray;
  typedef std::vector<class Face> FaceArray;
  typedef std::vector<class ParameterVariant> ParameterVariantArray;
  typedef std::vector<class ParameterVariant*> ParameterVariantRawPtrArray;
  typedef std::vector<class LineBatch*> LineBatchRawPtrArray;

  typedef rapidxml::xml_node<char> XmlNode;
  typedef rapidxml::xml_attribute<char> XmlAttribute;
  typedef rapidxml::xml_document<char> XmlDocument;
  typedef std::shared_ptr<XmlDocument> XmlDocumentPtr;
  typedef rapidxml::file<char> XmlFile;
  typedef std::shared_ptr<XmlFile> XmlFilePtr;
  typedef std::vector<class UILayer*> UILayerPtrArray;

  struct XmlDocBundle
  {
    XmlDocumentPtr doc;
    XmlFilePtr file;
  };

  typedef std::vector<class Event*> EventPool;

  // Filesystem variable types
  typedef std::filesystem::path Path;

  // Enitiy types.
  typedef std::shared_ptr<class Camera> CameraPtr;
  typedef std::shared_ptr<class Surface> SurfacePtr;

  // Callbacks.
  typedef std::function<void(class Event*, class Entity*)>
    SurfaceEventCallback;

  // Math Vector declerations.
  static const Vec3 ZERO = Vec3(0.0f);
  static const Vec3 X_AXIS = Vec3(1.0f, 0.0f, 0.0f);
  static const Vec3 Y_AXIS = Vec3(0.0f, 1.0f, 0.0f);
  static const Vec3 Z_AXIS = Vec3(0.0f, 0.0f, 1.0f);
  static const Vec3 XY_AXIS = Vec3(1.0f, 1.0f, 0.0f);
  static const Vec3 YZ_AXIS = Vec3(0.0f, 1.0f, 1.0f);
  static const Vec3 ZX_AXIS = Vec3(1.0f, 0.0f, 1.0f);
  static const Vec3 AXIS[6] =
  {
    X_AXIS,
    Y_AXIS,
    Z_AXIS,
    XY_AXIS,
    YZ_AXIS,
    ZX_AXIS
  };
  struct BoundingBox;

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
  static const String HDR(".hdr");

  // Local formats.
  static const String SCENE(".scene");
  static const String MESH(".mesh");
  static const String ANIM(".anim");
  static const String SKINMESH(".skinMesh");
  static const String SKELETON(".skeleton");
  static const String MATERIAL(".material");
  static const String SHADER(".shader");
  static const String AUDIO(".wav");

  static const ULongID NULL_HANDLE = 0;

  // Xml file IO.
  static const String XmlEntityElement("E");
  static const String XmlEntityIdAttr("i");
  static const String XmlParentEntityIdAttr("pi");
  static const String XmlEntityNameAttr("n");
  static const String XmlEntityTagAttr("ta");
  static const String XmlEntityTypeAttr("t");
  static const String XmlEntityVisAttr("vi");
  static const String XmlEntityTrLockAttr("lc");
  static const String XmlSceneElement("S");
  static const String XmlParamterElement("P");
  static const String XmlParamterValAttr("v");
  static const String XmlParamterTypeAttr("t");
  static const String XmlParamBlockElement("PB");
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
  static const String XmlResRefElement("ResourceRef");
  static const String XmlComponent("Component");

  enum class AxisLabel
  {
    None = -1,  // Order matters. Don't change.
    X,
    Y,
    Z,
    // Mod3 gives plane normal .
    YZ,  // YZ(3) % 3 = X(0)
    ZX,  // ZX(4) % 3 = Y(1)
    XY  // XY(5) % 3 = Z(2)
  };

  static const float TK_FLT_MAX = std::numeric_limits<float>::max();
  static const int TK_INT_MAX = std::numeric_limits<int>::max();

  // Graphics Api Type Overrides.
  enum class GraphicTypes
  {
    VertexShader = 0x8B31,
    FragmentShader = 0x8B30,
    UVRepeat = 0x2901,
    UVClampToEdge = 0x812F,
    SampleNearest = 0x2600,
    DrawTypeTriangle = 0x0004,
    DrawTypeLines = 0x0001,
    DrawTypeLineStrip = 0x0003,
    DrawTypeLineLoop = 0x0002,
    DrawTypePoints = 0x0000
  };

  static const char* TKVersionStr = "v0.3.12";

}  // namespace ToolKit
