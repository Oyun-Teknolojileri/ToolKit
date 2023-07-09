/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "GeometryTypes.h"
#include "Types.h"

namespace ToolKit
{

  // Xml Processing.
  ///////////////////////////////////////////////////////
  template <typename T>
  void ReadVec(XmlNode* node, T& val);
  template <typename T>
  void WriteVec(XmlNode* node, XmlDocument* doc, const T& val);
  TK_API void WriteAttr(XmlNode* node, XmlDocument* doc, const StringView& name, const StringView& val);

  TK_API void ReadAttr(XmlNode* node, const String& name, bool& val);
  TK_API void ReadAttr(XmlNode* node, const String& name, float& val);
  TK_API void ReadAttr(XmlNode* node, const String& name, int& val);
  TK_API void ReadAttr(XmlNode* node, const String& name, uint& val);
  TK_API void ReadAttr(XmlNode* node, const String& name, ULongID& val);
  TK_API void ReadAttr(XmlNode* node, const String& name, byte& val);
  TK_API void ReadAttr(XmlNode* node, const String& name, ubyte& val);
  TK_API void ReadAttr(XmlNode* node, const String& name, String& val);
  TK_API XmlNode* Query(XmlDocument* doc, const StringArray& path);

  // Updates or inject the attribute with val. Returns true if successful.
  TK_API bool UpdateAttribute(XmlDocument* doc, const StringArray& path, const String& attribute, const String& val);

  // Create an xml node with given name.
  // Append it to parent if not null else append it to doc.
  TK_API XmlNode* CreateXmlNode(XmlDocument* doc, const StringView& name, XmlNode* parent = nullptr);

  TK_API void WriteMaterial(XmlNode* parent, XmlDocument* doc, const String& file);
  TK_API MaterialPtr ReadMaterial(XmlNode* parent);

  // File path operations.
  ///////////////////////////////////////////////////////
  TK_API bool CheckSystemFile(StringView path);
  TK_API bool CheckFile(const String& path);
  TK_API String CreateCopyFileFullPath(const String& fullPath);
  TK_API void DecomposePath(const String& fullPath, String* path, String* name, String* ext);

  TK_API void NormalizePath(String& path);
  TK_API void UnixifyPath(String& path);
  TK_API void DosifyPath(String& path);
  TK_API String ConcatPaths(const StringArray& entries);

  /**
   * When a full path of a resource provided, converts it to shorter path
   * relative to its Root folder. If rootFolder pointer is given, set the value
   * to the extracted root folder name.
   * @params path Absolute resource path.
   * @params rootFolder optional output parameter to provide resource root
   * folder.
   * @returns Relative resource path to its root folder.
   */
  TK_API String GetRelativeResourcePath(const String& path, String* rootFolder = nullptr);

  /**
   * Checks if a resource has default path.
   * @return true if the resource is from Engine's directory.
   */
  TK_API bool IsDefaultResource(const String& path);

  /**
   * Checks if the path points to ToolKit folder.
   * @param path is the file path to check.
   * @returns true if file path root is ToolKit.
   */
  TK_API bool HasToolKitRoot(const String& path);

  /**
   * Extracts the file name with the extension from a path.
   */
  TK_API String GetFileName(const String& path);

  enum class ResourceType;
  TK_API String CreatePathFromResourceType(const String& file, ResourceType type);

  TK_API ResourceType GetResourceType(const String& ext);
  TK_API String GetTypeString(ResourceType type);
  TK_API String GetExtFromType(ResourceType type);
  TK_API String GetResourcePath(ResourceType type);

  TK_API char GetPathSeparator();
  TK_API String GetPathSeparatorAsStr();
  TK_API bool SupportedImageFormat(const String& ext);
  TK_API bool SupportedMeshFormat(const String& ext);
  TK_API bool IsLayer(const String& file);

  // String operations.
  ///////////////////////////////////////////////////////
  TK_API void Split(const String& s, const String& sep, StringArray& v);

  // Replace all occurrences of a string in another string.
  TK_API void ReplaceStringInPlace(String& subject, const String& search, const String& replace);

  // Replace first occurrences of a string in another string.
  TK_API void ReplaceFirstStringInPlace(String& subject, const String& search, const String& replace);

  TK_API void ReplaceCharInPlace(String& subject, const char search, const char replace);

  TK_API int CountChar(const String& str, const char chr);

  /**
   * Transform ascii chars to lower. Intended usage is extention comparison.
   */
  TK_API String ToLower(const String& str);
  TK_API String Format(const char* msg, ...);
  TK_API String Trim(const String& str, const String& whitespace = " \t");
  TK_API bool StartsWith(const String& str, const String& prefix);
  TK_API bool EndsWith(const String& str, const String& suffix);

  TK_API bool Utf8CaseInsensitiveSearch(const String& text, const String& search);

  // Debug geometries.
  ///////////////////////////////////////////////////////
  class LineBatch;
  TK_API LineBatch* CreatePlaneDebugObject(PlaneEquation plane, float size);
  TK_API LineBatch* CreateLineDebugObject(const Vec3Array& corners);
  TK_API LineBatch* CreateBoundingBoxDebugObject(const BoundingBox& box,
                                                 const Vec3& color     = Vec3(1.0f, 0.0f, 0.0f),
                                                 float size            = 2.0f,
                                                 const Mat4* transform = nullptr);

  // Entity operations.
  TK_API String EntityTypeToString(enum class EntityType type);
  TK_API void ToEntityIdArray(EntityIdArray& idArray, const EntityRawPtrArray& ptrArray);

  TK_API bool IsInArray(const EntityRawPtrArray& nttArray, Entity* ntt);
  TK_API void GetRootEntities(const EntityRawPtrArray& entities, EntityRawPtrArray& roots);

  TK_API void GetParents(const Entity* ntt, EntityRawPtrArray& parents);
  // Gather hierarchy from parent (indx 0) to child (indx end).
  // Revert the array for child to parent.
  TK_API void GetChildren(const Entity* ntt, EntityRawPtrArray& children);

  // {copies} First one is the copy root, fallowing are attached children.
  TK_API Entity* DeepCopy(Entity* root, EntityRawPtrArray& copies);

  // Memory operations.
  ///////////////////////////////////////////////////////
  // Useful to force plugin modules to allocate from main toolkit module.
  TK_API void* TKMalloc(size_t sz);
  //  Use in combination with TKMalloc to free from main toolkit module.
  TK_API void TKFree(void* m);

  // Vector operations.
  ///////////////////////////////////////////////////////
  TK_API int IndexOf(Entity* ntt, const EntityRawPtrArray& entities);
  TK_API bool Exist(const IntArray& vec, int val);

  template <typename T, typename Predicate>
  void move_values(std::vector<T>& from, std::vector<T>& to, Predicate p)
  {
    auto it = std::remove_if(from.begin(),
                             from.end(),
                             [&](const T& val)
                             {
                               if (p(val))
                               {
                                 to.push_back(std::move(val));
                                 return true;
                               }
                               return false;
                             });
    from.erase(it, from.end());
  }

  template <typename T>
  void pop_front(std::vector<T>& vec)
  {
    assert(!vec.empty());
    vec.erase(vec.begin());
  }

  template <typename T, typename Pred>
  void erase_if(T& vec, Pred pred)
  {
    vec.erase(std::remove_if(vec.begin(), vec.end(), pred), vec.end());
  }

  template <typename T>
  bool contains(const std::vector<T>& arr, const T& val)
  {
    return std::find(arr.cbegin(), arr.cend(), val) != arr.cend();
  }

  /**
   * Find index of val in given array.
   * @param arr array that we want to search.
   * @returns if given value exist, returns index of val otherwise -1
   */
  template <typename T>
  int FindIndex(const std::vector<T>& arr, const T& val)
  {
    auto it = std::find(arr.cbegin(), arr.cend(), val);
    return it == arr.cend() ? -1 : int(it - arr.cbegin());
  }

  //  Time.
  ///////////////////////////////////////////////////////
  TK_API float MillisecToSec(float ms);
  //  Returns elapsed time from the ToolKit Init.
  TK_API float GetElapsedMilliSeconds();

} // namespace ToolKit
