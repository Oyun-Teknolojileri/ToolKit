#include "stdafx.h"
#include "Util.h"
#include "rapidxml.hpp"
#include "Primative.h"
#include "DebugNew.h"

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
      if constexpr (std::is_integral_v<T::value_type>)
      {
        val[i] = std::atoi(attr->value());
      }
      else if (std::is_floating_point_v<T::value_type>)
      {
        val[i] = (float)std::atof(attr->value());
      }
      else
      {
        assert(false);
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

  template<typename T>
  T ReadAttr(XmlNode* node, const String& name)
  {
    if (XmlAttribute* attr = node->first_attribute(name.c_str()))
    {
      return (T)std::atoi(attr->value());
    }

    return (T)0;
  }

  template Byte ReadAttr<Byte>(XmlNode* node, const String& name);
  template UByte ReadAttr<UByte>(XmlNode* node, const String& name);
  template int ReadAttr<int>(XmlNode* node, const String& name);
  template uint ReadAttr<uint>(XmlNode* node, const String& name);
  template float ReadAttr<float>(XmlNode* node, const String& name);

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

  bool CheckFile(const String& path)
  {
    std::ifstream f(path.c_str());
    return f.good();
  }

  void DecomposePath(const String fullPath, String* path, String* name, String* ext)
  {
    String normal = fullPath;
    NormalizePath(normal);

    size_t ind1 = normal.find_last_of('\\');
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
    ReplaceStringInPlace(path, "/", "\\");
  }

  char GetPathSeparator()
  {
    return '\\';
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

  ToolKit::LineBatch* GenerateBoundingVolumeGeometry(const BoundingBox& box, Mat4* transform)
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
