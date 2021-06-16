#pragma once

#include "MathUtil.h"
#include "Types.h"

#include <assert.h>
#include <string.h>
#include <vector>

namespace ToolKit
{

  template<typename T>
  void ReadVec(XmlNode* node, T& val);
  template<typename T>
  void WriteVec(XmlNode* node, XmlDocument* doc, const T& val);
  template<typename T>
  T ReadAttr(XmlNode* node, const String& name);
  void WriteAttr(XmlNode* node, XmlDocument* doc, const String& name, const String& val);

  bool CheckFile(const String& path);
  void DecomposePath(const String fullPath, String* path, String* name, String* ext);
  void NormalizePath(String& path);
  char GetPathSeparator();
  String GetPathSeparatorAsStr();
  void Split(const String& s, const String& sep, StringArray& v);
  void ReplaceStringInPlace(String& subject, const String& search, const String& replace);
  class LineBatch* CreatePlaneDebugObject(PlaneEquation plane, float size);
  class LineBatch* CreateLineDebugObject(const Vec3Array& corners);
  class LineBatch* GenerateBoundingVolumeGeometry(const BoundingBox& box, Mat4* transform = nullptr);
  void ToEntityIdArray(EntityIdArray& idArray, const EntityRawPtrArray& ptrArray);
  bool IsInArray(const EntityRawPtrArray& nttArray, Entity* ntt);
  void GetRootEntities(const EntityRawPtrArray& entities, EntityRawPtrArray& roots);

  template<typename T>
  void pop_front(std::vector<T>& vec)
  {
    assert(!vec.empty());
    vec.erase(vec.begin());
  }

}
