#include "stdafx.h"
#include "Util.h"
#include "rapidxml.hpp"
#include "Primative.h"
#include "DebugNew.h"

#include <cstdarg>
#include <fstream>
#include <filesystem>
#include <algorithm>

namespace ToolKit
{

  template<typename T>
  void ReadVec(XmlNode* node, T& val)
  {
    static const char letters[] = { 'x', 'y', 'z', 'w' };
    int limit = glm::min(val.length(), 4);
    for (int i = 0; i < limit; i++)
    {
      XmlAttribute* attr = node->first_attribute(letters + i, 1);
      if constexpr (std::is_integral_v<typename T::value_type>)
      {
        val[i] = (int)std::atoi(attr->value());
      }
      else if constexpr (std::is_floating_point_v<typename T::value_type>)
      {
        val[i] = (float)std::atof(attr->value());
      }
    }
  }

  template void ReadVec(XmlNode* node, Vec2& val);
  template void ReadVec(XmlNode* node, Vec3& val);
  template void ReadVec(XmlNode* node, glm::ivec3& val);
  template void ReadVec(XmlNode* node, Quaternion& val);
  template void ReadVec(XmlNode* node, Vec4& val);
  template void ReadVec(XmlNode* node, UVec4& val);

  template<typename T>
  void WriteVec(XmlNode* node, XmlDocument* doc, const T& val)
  {
    static const String letters[] = { "x", "y", "z", "w" };
    int limit = glm::min(val.length(), 4);
    for (int i = 0; i < limit; i++)
    {
      WriteAttr(node, doc, letters[i], std::to_string(val[i]));
    }
  }

  template void WriteVec(XmlNode* node, XmlDocument* doc, const Vec2& val);
  template void WriteVec(XmlNode* node, XmlDocument* doc, const Vec3& val);
  template void WriteVec(XmlNode* node, XmlDocument* doc, const Vec4& val);
  template void WriteVec(XmlNode* node, XmlDocument* doc, const Quaternion& val);

  void WriteAttr(XmlNode* node, XmlDocument* doc, const String& name, const String& val)
  {
    node->append_attribute
    (
      doc->allocate_attribute
      (
        doc->allocate_string(name.c_str(), 0),
        doc->allocate_string(val.c_str(), 0)
      )
    );
  }

  template<typename T>
  T ReadVal(XmlNode* node, const String& name)
  {
    if (XmlAttribute* attr = node->first_attribute(name.c_str()))
    {
      if constexpr (std::is_integral_v<T>)
      {
        return (int)std::atoi(attr->value());
      }
      else if constexpr (std::is_floating_point_v<T>)
      {
        return (float)std::atof(attr->value());
      }
    }

    return 0;
  }

  void ReadAttr(XmlNode* node, const String& name, bool& val)
  {
    val = ReadVal<bool>(node, name);
  }

  void ReadAttr(XmlNode* node, const String& name, float& val)
  {
    val = ReadVal<float>(node, name);
  }

  void ReadAttr(XmlNode* node, const String& name, int& val)
  {
    val = ReadVal<int>(node, name);
  }

  void ReadAttr(XmlNode* node, const String& name, uint& val)
  {
    val = ReadVal<uint>(node, name);
  }

  void ReadAttr(XmlNode* node, const String& name, Byte& val)
  {
    val = ReadVal<Byte>(node, name);
  }

  void ReadAttr(XmlNode* node, const String& name, UByte& val)
  {
    val = ReadVal<UByte>(node, name);
  }

  void ReadAttr(XmlNode* node, const String& name, String& val)
  {
    if (XmlAttribute* attr = node->first_attribute(name.c_str()))
    {
      val = attr->value();
    }
  }

  bool CheckFile(const String& path)
  {
    return std::filesystem::exists(path);
  }

  String CreateCopyFileFullPath(const String& fullPath)
  {
    String cpyPath;
    if (!fullPath.empty())
    {
      String path, name, ext;
      DecomposePath(fullPath, &path, &name, &ext);
      cpyPath = ConcatPaths({ path, name + "_copy" + ext });
      if (CheckFile(cpyPath))
      {
        int i = 1;
        do
        {
          cpyPath = ConcatPaths({ path, name + "_copy(" + std::to_string(i++) + ")" + ext });
        } while (CheckFile(cpyPath));
      }
    }

    return cpyPath;
  }

  void DecomposePath(const String& fullPath, String* path, String* name, String* ext)
  {
    String normal = fullPath;
    NormalizePath(normal);

    size_t ind1 = normal.find_last_of(GetPathSeparator());
    if (path != nullptr)
    {
      *path = normal.substr(0, ind1);
    }

    size_t ind2 = normal.find_last_of('.');
    if (ind2 != String::npos)
    {
      if (name != nullptr)
      {
        *name = normal.substr(ind1 + 1, ind2 - ind1 - 1);
      }

      if (ext != nullptr)
      {
        *ext = normal.substr(ind2);
      }
    }
  }

  void NormalizePath(String& path)
  {
#ifndef __clang__
    ReplaceStringInPlace(path, "/", "\\");
#else
    ReplaceStringInPlace(path, "\\", "/");
#endif
  }

  String ConcatPaths(const StringArray& entries)
  {
    String path;
    if (entries.empty())
    {
      return path;
    }

    for (size_t i = 0; i < entries.size() - 1; i++)
    {
      path += entries[i] + GetPathSeparatorAsStr();
    }

    return path + entries.back();
  }

  ResourceType GetResourceType(const String& ext)
  {
    if (ext == MESH || ext == SKINMESH || SupportedMeshFormat(ext))
    {
      return ResourceType::Mesh;
    }

    if (ext == ANIM)
    {
      return ResourceType::Animation;
    }

    if (ext == MATERIAL)
    {
      return ResourceType::Material;
    }

    if (SupportedImageFormat(ext))
    {
      return ResourceType::Texture;
    }

    if (ext == SHADER)
    {
      return ResourceType::Shader;
    }

    if (ext == AUDIO)
    {
      return ResourceType::Audio;
    }

    assert(false);
    return ResourceType::Base;
  }

  String GetTypeString(ResourceType type)
  {
    switch (type)
    {
    case ResourceType::Base:
      return "Base";
      break;
    case ResourceType::Animation:
      return "Animation";
      break;
    case ResourceType::Audio:
      return "Audio";
      break;
    case ResourceType::Material:
      return "Material";
      break;
    case ResourceType::Mesh:
      return "Mesh";
      break;
    case ResourceType::Shader:
      return "Shader";
      break;
    case ResourceType::SkinMesh:
      return "SkinMesh";
      break;
    case ResourceType::SpriteSheet:
      return "SpriteSheet";
      break;
    case ResourceType::Texture:
      return "Texture";
      break;
    case ResourceType::CubeMap:
      return "CubeMap";
      break;
    case ResourceType::RenderTarget:
      return "RenderTarget";
      break;
    case ResourceType::Scene:
      return "Scene";
      break;
    default:
      assert(false);
      break;
    }

    return String();
  }

  String GetExtFromType(ResourceType type)
  {
    switch (type)
    {
    case ResourceType::Base:
      break;
    case ResourceType::Animation:
      return ANIM;
      break;
    case ResourceType::Audio:
      return AUDIO;
      break;
    case ResourceType::Material:
      return MATERIAL;
      break;
    case ResourceType::Mesh:
      return MESH;
      break;
    case ResourceType::Shader:
      return SHADER;
      break;
    case ResourceType::SkinMesh:
      return SKINMESH;
      break;
    case ResourceType::SpriteSheet:
      assert(false);
      break;
    case ResourceType::Texture:
      assert(false);
      break;
    case ResourceType::CubeMap:
      assert(false);
      break;
    case ResourceType::RenderTarget:
      assert(false);
      break;
    case ResourceType::Scene:
      return SCENE;
      break;
    default:
      assert(false);
      break;
    }
    
    return String();
  }

  String GetResourcePath(ResourceType type)
  {
    String path;
    switch (type)
    {
    case ResourceType::Base:
      break;
    case ResourceType::Animation:
      path = AnimationPath("");
      break;
    case ResourceType::Audio:
      path = AudioPath("");
      break;
    case ResourceType::Material:
      path = MaterialPath("");
      break;
    case ResourceType::Mesh:
    case ResourceType::SkinMesh:
      path = MeshPath("");
      break;
    case ResourceType::Shader:
      path = ShaderPath("");
      break;
    case ResourceType::SpriteSheet:
      path = SpritePath("");
      break;
    case ResourceType::Texture:
    case ResourceType::CubeMap:
      path = TexturePath("");
      break;
    case ResourceType::RenderTarget:
      break;
    default:
      assert(false);
      break;
    }

    return path;
  }

  String GetRelativeResourcePath(const String& fullPath)
  {
    String ext;
    DecomposePath(fullPath, nullptr, nullptr, &ext);
    ResourceType type = GetResourceType(ext);
    String path = GetResourcePath(type);

    size_t pos = fullPath.find(path);
    if (pos != String::npos)
    {
      return fullPath.substr(pos + path.size());
    }
    
    return fullPath;
  }

  char GetPathSeparator()
  {
#ifndef __clang__
    return '\\';
#else
    return '/';
#endif
  }

  String GetPathSeparatorAsStr()
  {
    return String() + GetPathSeparator();
  }

  bool SupportedImageFormat(const String& ext) 
  {
    static String supportedFormats(PNG + JPG + JPEG + TGA + BMP + PSD);
    return supportedFormats.find(ToLower(ext)) != String::npos;
  }

  bool SupportedMeshFormat(const String& ext)
  {
    static String supportedFormats(FBX + GLB + GLTF + OBJ);
    return supportedFormats.find(ToLower(ext)) != String::npos;
  }

  // split a string into multiple sub strings, based on a separator string
  // for example, if separator="::",
  // s = "abc::def xy::st:" -> "abc", "def xy" and "st:",
  // https://stackoverflow.com/questions/53849/how-do-i-tokenize-a-string-in-c?page=2&tab=votes#tab-top
  void Split(const String& s, const String& sep, StringArray& v)
  {
    typedef String::const_iterator iter;
    iter b = s.begin(), e = s.end(), i;
    iter sep_b = sep.begin(), sep_e = sep.end();

    // search through s
    while (b != e)
    {
      i = search(b, e, sep_b, sep_e);

      // no more separator found
      if (i == e)
      {
        // it's not an empty string
        if (b != e)
        {
          v.push_back(String(b, e));
        }
        break;
      }
      else if (i == b)
      {
        // the separator is found and right at the beginning
        // in this case, we need to move on and search for the
        // next separator
        b = i + sep.length();
      }
      else
      {
        // found the separator
        v.push_back(String(b, i));
        b = i;
      }
    }
  }

  // https://stackoverflow.com/questions/5878775/how-to-find-and-replace-string
  void ReplaceStringInPlace(String& subject, const String& search, const String& replace)
  {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
      subject.replace(pos, search.length(), replace);
      pos += replace.length();
    }
  }

  LineBatch* CreatePlaneDebugObject(PlaneEquation plane, float size)
  {
    // Searching perpendicular axes on the plane.
    Vec3 z = plane.normal;
    Vec3 x = z + Vec3(1.0f);
    Vec3 y = glm::cross(z, x);
    x = glm::normalize(glm::cross(y, z));
    y = glm::normalize(glm::cross(z, x));

    NormalizePlaneEquation(plane);
    Vec3 o = plane.normal * plane.d;

    float hSize = size * 0.5f;
    Vec3Array corners
    {
      o + x * hSize + y * hSize,
      o - x * hSize + y * hSize,
      o - x * hSize - y * hSize,
      o + x * hSize - y * hSize
    };

    LineBatch* obj = new LineBatch(corners, X_AXIS, DrawType::LineLoop, 5.0f);
    return obj;
  }

  class LineBatch* CreateLineDebugObject(const Vec3Array& corners)
  {
    LineBatch* obj = new LineBatch(corners, X_AXIS, DrawType::LineLoop, 5.0f);
    return obj;
  }

  LineBatch* GenerateBoundingVolumeGeometry(const BoundingBox& box, Mat4* transform)
  {
    Vec3Array corners;
    GetCorners(box, corners);

    std::vector<Vec3> vertices =
    {
      corners[0], // FTL
      corners[3], // FBL
      corners[2], // FBR
      corners[1], // FTR
      corners[0], // FTL
      corners[4], // BTL
      corners[7], // BBL
      corners[6], // BBR
      corners[5], // BTR
      corners[4], // BTL
      corners[5], // BTR
      corners[1], // FTR
      corners[2], // FBR
      corners[6], // BBR
      corners[7], // BBL
      corners[3] // FBL
    };

    if (transform != nullptr)
    {
      for (Vec3& v : vertices)
      {
        v = *transform * Vec4(v, 1.0f);
      }
    }

    LineBatch* lineForm = new LineBatch(vertices, X_AXIS, DrawType::LineStrip, 2.0f);
    return lineForm;
  }

  void ToEntityIdArray(EntityIdArray& idArray, const EntityRawPtrArray& ptrArray)
  {
    idArray.reserve(ptrArray.size());
    for (Entity* ntt : ptrArray)
    {
      idArray.push_back(ntt->m_id);
    }
  }

  bool IsInArray(const EntityRawPtrArray& nttArray, Entity* ntt)
  {
    for (Entity* e : nttArray)
    {
      if (e == ntt)
      {
        return true;
      }
    }

    return false;
  }

  void RootsOnly(const EntityRawPtrArray& entities, EntityRawPtrArray& roots, Entity* child)
  {
    auto AddUnique = [&roots](Entity* e)
    {
      assert(e != nullptr);

      bool unique = std::find(roots.begin(), roots.end(), e) == roots.end();
      if (unique)
      {
        roots.push_back(e);
      }
    };

    Node* parent = child->m_node->m_parent;
    if (parent != nullptr)
    {
      Entity* parentEntity = parent->m_entity;
      if (std::find(entities.begin(), entities.end(), parentEntity) != entities.end())
      {
        RootsOnly(entities, roots, parentEntity);
      }
      else
      {
        AddUnique(child);
      }
    }
    else
    {
      AddUnique(child);
    }
  }

  void GetRootEntities(const EntityRawPtrArray& entities, EntityRawPtrArray& roots)
  {
    for (Entity* e : entities)
    {
      RootsOnly(entities, roots, e);
    }
  }

  String ToLower(const String& str)
  {
    String lwr = str;
    transform(lwr.begin(), lwr.end(), lwr.begin(), ::tolower);
    return lwr;
  }

}
