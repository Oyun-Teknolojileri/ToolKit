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
  void WriteAttr(XmlNode* node, XmlDocument* doc, const String& name, const String& val);

  void ReadAttr(XmlNode* node, const String& name, bool& val);
  void ReadAttr(XmlNode* node, const String& name, float& val);
  void ReadAttr(XmlNode* node, const String& name, int& val);
  void ReadAttr(XmlNode* node, const String& name, uint& val);
  void ReadAttr(XmlNode* node, const String& name, Byte& val);
  void ReadAttr(XmlNode* node, const String& name, UByte& val);
  void ReadAttr(XmlNode* node, const String& name, String& val);
  XmlNode* Query(XmlDocument* doc, const StringArray& path);

  void WriteMaterial(XmlNode* parent, XmlDocument* doc, const String& file);
  MaterialPtr ReadMaterial(XmlNode* parent);

  // File path operations.
  bool CheckFile(const String& path);
  String CreateCopyFileFullPath(const String& fullPath);
  void DecomposePath(const String& fullPath, String* path, String* name, String* ext);
  void NormalizePath(String& path);
  void UnixifyPath(String& path);
  String ConcatPaths(const StringArray& entries);
  String GetRelativeResourcePath(const String& path);

  enum class ResourceType;
  ResourceType GetResourceType(const String& ext);
  String GetTypeString(ResourceType type);
  String GetExtFromType(ResourceType type);
  String GetResourcePath(ResourceType type);
  
  char GetPathSeparator();
  String GetPathSeparatorAsStr();
  bool SupportedImageFormat(const String& ext);
  bool SupportedMeshFormat(const String& ext);

  // String operations.
  void Split(const String& s, const String& sep, StringArray& v);
  void ReplaceStringInPlace(String& subject, const String& search, const String& replace);
  String ToLower(const String& str);
  String Format(const char* msg, ...);

  // Debug geometries.
  class LineBatch* CreatePlaneDebugObject(PlaneEquation plane, float size);
  class LineBatch* CreateLineDebugObject(const Vec3Array& corners);
  class LineBatch* CreateBoundingBoxDebugObject(const BoundingBox& box, const Mat4* transform = nullptr);

  // Entity operations.
  void ToEntityIdArray(EntityIdArray& idArray, const EntityRawPtrArray& ptrArray);
  bool IsInArray(const EntityRawPtrArray& nttArray, Entity* ntt);
  void GetRootEntities(const EntityRawPtrArray& entities, EntityRawPtrArray& roots);
  void GetParents(const Entity* ntt, EntityRawPtrArray& parents);
  Entity* DeepCopy(Entity* root, EntityRawPtrArray& copies); // copies: First one is the copy root, fallowings are attached children.
  Entity* DeepInstantiate(Entity* root, EntityRawPtrArray& instances);

  // Vector operations.

  int IndexOf(Entity* ntt, const EntityRawPtrArray& entities);

  template<typename T>
  void pop_front(std::vector<T>& vec)
  {
    assert(!vec.empty());
    vec.erase(vec.begin());
  }

}
