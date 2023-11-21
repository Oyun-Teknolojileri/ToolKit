/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

/**
 * @file Forward declerations for frequenty used common ToolKit classes and
 * relatled structures.
 */

#ifdef __ANDROID__
  // GLM
  #define GLM_FORCE_XYZW_ONLY
  #define GLM_FORCE_CTOR_INIT
  #define GLM_ENABLE_EXPERIMENTAL
  #ifndef GLM_FORCE_SWIZZLE
    #define GLM_FORCE_SWIZZLE
  #endif

  #include "glm/glm.hpp"
  #include "glm/gtc/epsilon.hpp"
  #include "glm/gtc/matrix_access.hpp"
  #include "glm/gtc/matrix_inverse.hpp"
  #include "glm/gtc/matrix_transform.hpp"
  #include "glm/gtc/quaternion.hpp"
  #include "glm/gtc/random.hpp"
  #include "glm/gtx/closest_point.hpp"
  #include "glm/gtx/component_wise.hpp"
  #include "glm/gtx/euler_angles.hpp"
  #include "glm/gtx/matrix_operation.hpp"
  #include "glm/gtx/matrix_query.hpp"
  #include "glm/gtx/quaternion.hpp"
  #include "glm/gtx/scalar_relational.hpp"
  #include "glm/gtx/string_cast.hpp"
  #include "glm/gtx/vector_query.hpp"
#else
  #include <glm/ext.hpp>
  #include <glm/glm.hpp>
#endif

// RapidXml
#include <RapidXml/rapidxml_ext.h>
#include <RapidXml/rapidxml_utils.hpp>

#include <filesystem>
#include <functional>
#include <limits>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#ifdef _WIN32 // Windows.
  #define TK_STDCAL __stdcall
  #ifdef TK_DLL_EXPORT // Dynamic binding.
    #define TK_API __declspec(dllexport)
  #elif defined(TK_DLL_IMPORT)
    #define TK_API __declspec(dllimport)
  #else // Static binding.
    #define TK_API
  #endif
#elif defined(__clang__)
  #define TK_API __attribute__((visibility("default")))
  #define TK_STDCAL
#endif

#define SafeDel(ptr)                                                                                                   \
  {                                                                                                                    \
    delete ptr;                                                                                                        \
    ptr = nullptr;                                                                                                     \
  }
#define SafeDelArray(ptr)                                                                                              \
  {                                                                                                                    \
    delete[] ptr;                                                                                                      \
    ptr = nullptr;                                                                                                     \
  }

#define TKStringify(x) #x
#define TKToString(x)  TKStringify(x)
#define TKLoc          __FILE__ ":" TKToString(__LINE__)

// std template types require dll interface.
#pragma warning(disable : 4251)

/// @cond
namespace rapidxml
{
  template <class Ch>
  class xml_document;
  template <class Ch>
  class xml_node;
  template <class Ch>
  class xml_attribute;
  template <class Ch>
  class file;
} // namespace rapidxml

/// @endcond

namespace ToolKit
{

  // Primitive types.
  typedef char byte;
  typedef unsigned char ubyte;
  typedef std::vector<byte> ByteArray;
  typedef uint32_t uint;
  typedef uint8_t uint8;
  typedef uint64_t uint64;
  typedef uint64_t ULongID;
  typedef const int16_t SignalId;
  typedef std::string String;
  typedef std::string_view StringView;
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
  typedef std::vector<int> IntArray;
  typedef std::vector<uint> UIntArray;
  typedef std::vector<struct VariantCategory> VariantCategoryArray;
  typedef std::vector<struct RenderJob> RenderJobArray;

  // Resource types.
  typedef std::shared_ptr<class Animation> AnimationPtr;
  typedef std::shared_ptr<class Material> MaterialPtr;
  typedef std::vector<MaterialPtr> MaterialPtrArray;
  typedef std::shared_ptr<class CubeMap> CubeMapPtr;
  typedef std::shared_ptr<class Texture> TexturePtr;
  typedef std::shared_ptr<class DataTexture> DataTexturePtr;
  typedef std::shared_ptr<class LightDataTexture> LightDataTexturePtr;
  typedef std::shared_ptr<class Hdri> HdriPtr;
  typedef std::shared_ptr<class RenderTarget> RenderTargetPtr;
  typedef std::shared_ptr<class Framebuffer> FramebufferPtr;
  typedef std::shared_ptr<class SpriteSheet> SpriteSheetPtr;
  typedef std::shared_ptr<class Mesh> MeshPtr;
  typedef std::shared_ptr<class Skeleton> SkeletonPtr;
  typedef std::shared_ptr<class DynamicBoneMap> DynamicBoneMapPtr;
  typedef std::shared_ptr<class Shader> ShaderPtr;
  typedef std::vector<ShaderPtr> ShaderPtrArray;
  typedef std::shared_ptr<class GpuProgram> GpuProgramPtr;
  typedef std::shared_ptr<class SkinMesh> SkinMeshPtr;
  typedef std::shared_ptr<class Scene> ScenePtr;
  typedef std::vector<MeshPtr> MeshPtrArray;
  typedef std::vector<class Mesh*> MeshRawPtrArray;
  typedef std::vector<const class Mesh*> MeshRawCPtrArray;
  typedef std::shared_ptr<class AnimRecord> AnimRecordPtr;
  typedef std::unordered_map<String, AnimRecordPtr> AnimRecordPtrMap;
  typedef class AnimRecord* AnimRecordRawPtr;
  typedef std::vector<AnimRecordRawPtr> AnimRecordRawPtrArray;
  struct BlendTarget;

  // Entity types.
  typedef std::shared_ptr<class Entity> EntityPtr;
  typedef std::weak_ptr<class Entity> EntityWeakPtr;
  typedef std::vector<EntityPtr> EntityPtrArray;
  typedef std::vector<class Entity*> EntityRawPtrArray;
  typedef std::vector<class Light*> LightRawPtrArray;
  typedef std::shared_ptr<class Light> LightPtr;
  typedef std::vector<LightPtr> LightPtrArray;
  typedef std::vector<class DirectionalLight*> DirectionalLightRawPtrArray;
  typedef std::vector<class SpotLight*> SpotLightRawPtrArray;
  typedef std::vector<class PointLight*> PointLightRawPtrArray;
  typedef std::vector<ULongID> EntityIdArray;
  typedef std::vector<class Node*> NodeRawPtrArray;
  typedef std::vector<class Vertex> VertexArray;
  typedef std::vector<class Face> FaceArray;
  typedef std::vector<class ParameterVariant> ParameterVariantArray;
  typedef std::vector<class ParameterVariant*> ParameterVariantRawPtrArray;
  typedef std::shared_ptr<class LineBatch> LineBatchPtr;
  typedef std::vector<class LineBatch*> LineBatchRawPtrArray;
  typedef std::vector<LineBatchPtr> LineBatchPtrArray;
  typedef std::shared_ptr<class SkyBase> SkyBasePtr;
  typedef std::shared_ptr<class Sky> SkyPtr;
  typedef std::shared_ptr<class GradientSky> GradientSkyPtr;
  typedef std::shared_ptr<class Prefab> PrefabPtr;
  typedef std::vector<PrefabPtr> PrefabPtrArray;
  typedef std::vector<class Prefab*> PrefabRawPtrArray;
  typedef std::shared_ptr<class Canvas> CanvasPtr;
  typedef std::shared_ptr<class Billboard> BillboardPtr;
  typedef std::shared_ptr<class Camera> CameraPtr;
  typedef std::shared_ptr<class Camera> CameraPtr;
  typedef std::shared_ptr<class Surface> SurfacePtr;

  // Xml types.
  typedef rapidxml::xml_node<char> XmlNode;
  typedef rapidxml::xml_attribute<char> XmlAttribute;
  typedef rapidxml::xml_document<char> XmlDocument;
  typedef std::shared_ptr<XmlDocument> XmlDocumentPtr;
  typedef rapidxml::file<char> XmlFile;
  typedef std::shared_ptr<XmlFile> XmlFilePtr;

  struct XmlDocBundle
  {
    XmlDocumentPtr doc;
    XmlFilePtr file;
  };

  // System Types
  typedef std::vector<class Event*> EventPool;
  typedef std::shared_ptr<class Viewport> ViewportPtr;

  // File system variable types
  typedef std::filesystem::path Path;

  // Callbacks.
  typedef std::function<void(class Event*, EntityPtr)> SurfaceEventCallback;
  typedef std::function<void(const String&)> GlReportCallback;

  // Math Vector decelerations.
  static const Vec3 ZERO    = Vec3(0.0f);
  static const Vec3 X_AXIS  = Vec3(1.0f, 0.0f, 0.0f);
  static const Vec3 Y_AXIS  = Vec3(0.0f, 1.0f, 0.0f);
  static const Vec3 Z_AXIS  = Vec3(0.0f, 0.0f, 1.0f);
  static const Vec3 XY_AXIS = Vec3(1.0f, 1.0f, 0.0f);
  static const Vec3 YZ_AXIS = Vec3(0.0f, 1.0f, 1.0f);
  static const Vec3 ZX_AXIS = Vec3(1.0f, 0.0f, 1.0f);
  static const Vec3 AXIS[6] = {X_AXIS, Y_AXIS, Z_AXIS, XY_AXIS, YZ_AXIS, ZX_AXIS};
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
  static const String LAYER(".layer");

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
  static const String XmlTranslateElement("T");
  static const String XmlRotateElement("R");
  static const String XmlScaleElement("S");
  static const String XmlResRefElement("ResourceRef");
  static const String XmlComponent("Component");
  static const StringView XmlNodeSettings("Settings");
  static const StringView XmlNodeName("name");

  // V > 0.4.4
  static const StringView XmlObjectClassAttr("Cl");
  static const StringView XmlObjectElement("Ob");
  static const StringView XmlObjectIdAttr("i");
  static const StringView XmlComponentArrayElement("Ca");
  static const StringView XmlVersion("version");

  enum class AxisLabel
  {
    None = -1, // Order matters. Don't change.
    X,
    Y,
    Z,
    // Mod3 gives plane normal .
    YZ, // YZ(3) % 3 = X(0)
    ZX, // ZX(4) % 3 = Y(1)
    XY, // XY(5) % 3 = Z(2)
    // Don't apply mod3
    XYZ
  };

  enum class DirectionLabel
  {
    None = -1,
    N,
    S,
    E,
    W,
    SE,
    SW,
    NE,
    NW,
    CENTER
  };

  constexpr float TK_FLT_MAX = std::numeric_limits<float>::max();
  constexpr float TK_FLT_MIN = std::numeric_limits<float>::min();
  constexpr int TK_INT_MAX   = std::numeric_limits<int>::max();
  constexpr int TK_UINT_MAX  = std::numeric_limits<unsigned int>::max();

  // Graphics Api Type Overrides.
  enum class GraphicTypes
  {
    VertexShader               = 0x8B31,
    FragmentShader             = 0x8B30,
    UVRepeat                   = 0x2901,
    UVClampToEdge              = 0x812F,
    UVClampToBorder            = 0x812D,
    SampleNearest              = 0x2600,
    SampleLinear               = 0x2601,
    SampleNearestMipmapNearest = 0x2700,
    SampleLinearMipmapLinear   = 0x2703,
    SampleLinearMipmapNearest  = 0x2701,
    FormatRed                  = 0x1903,
    FormatR8                   = 0x8229,
    FormatRG                   = 0x8227,
    FormatRG8                  = 0x822B,
    FormatRGB                  = 0x1907,
    FormatRGB8                 = 0x8051,
    FormatRGBA8                = 0x8058,
    FormatRGBA                 = 0x1908,
    FormatR16F                 = 0x822A,
    FormatR32F                 = 0x822E,
    FormatRG16F                = 0x822F,
    FormatRG32F                = 0x8230,
    FormatRGB16F               = 0x881B,
    FormatRGBA16F              = 0x881A,
    FormatRGB32F               = 0x8815,
    FormatRGBA32F              = 0x8814,
    FormatR16SNorm             = 0x8F98,
    FormatSRGB8_A8             = 0x8C43,
    FormatDepthComponent       = 0x1902,
    ColorAttachment0           = 0x8CE0,
    DepthAttachment            = 0x8D00,
    TypeFloat                  = 0x1406,
    TypeUnsignedByte           = 0x1401,
    Target2D                   = 0x0DE1,
    TargetCubeMap              = 0x8513,
    Target2DArray              = 0x8C1A
  };

  static const String TKResourcePak = "MinResources.pak";

  static const char* TKVersionStr   = "v0.4.5";
  static const String TKV044        = "v0.4.4";
  static const String TKV045        = "v0.4.5";

} // namespace ToolKit
