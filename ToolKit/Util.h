/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "GeometryTypes.h"
#include "Types.h"

namespace ToolKit
{

  // Xml Processing.
  //////////////////////////////////////////

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
  TK_API void ReadAttr(XmlNode* node, const String& name, String& val, StringView defaultVal = "");
  TK_API XmlNode* Query(XmlDocument* doc, const StringArray& path);

  // Updates or inject the attribute with val. Returns true if successful.
  TK_API bool UpdateAttribute(XmlDocument* doc, const StringArray& path, const String& attribute, const String& val);

  // Create an xml node with given name.
  // Append it to parent if not null else append it to doc.
  TK_API XmlNode* CreateXmlNode(XmlDocument* doc, const StringView& name, XmlNode* parent = nullptr);

  TK_API void WriteMaterial(XmlNode* parent, XmlDocument* doc, const String& file);
  TK_API MaterialPtr ReadMaterial(XmlNode* parent);

  // File path operations.
  //////////////////////////////////////////

  /**
   * Checks if a file exist in the host system.
   * @param path is the full file path to check for.
   * @return true if file exist in the given path.
   */
  TK_API bool CheckSystemFile(StringView path);

  /**
   * Uses file manager to check if a file exist. First searches the file among resources, then in the system and finally
   * in the pak file. This function is more suitable to check files inside the Resources.
   * @param path is the full file path to check or path to relative Resource folder.
   * @return true if the file exist in the given path.
   */
  TK_API bool CheckFile(const String& path);

  TK_API String CreateCopyFileFullPath(const String& fullPath);
  TK_API void DecomposePath(const String& fullPath, String* path, String* name, String* ext);

  TK_API String NormalizePath(String path);
  TK_API void NormalizePathInplace(String& path);
  TK_API void UnixifyPath(String& path);
  TK_API void DosifyPath(String& path);
  TK_API String ConcatPaths(const StringArray& entries);

  // copys all of the directories and folders recursively
  // note that ignored names can be empty and ignoredExtensions will not copied
  TK_API void RecursiveCopyDirectory(const String& source,
                                     const String& destination,
                                     const StringArray& ignoredExtensions);

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

  TK_API String CreatePathFromResourceType(const String& file, struct ClassMeta* Class);

  TK_API struct ClassMeta* GetResourceType(const String& ext);
  TK_API String GetExtFromType(struct ClassMeta* Class);
  TK_API String GetResourcePath(struct ClassMeta* Class);

  TK_API char GetPathSeparator();
  TK_API String GetPathSeparatorAsStr();
  TK_API bool SupportedImageFormat(const String& ext);
  TK_API bool SupportedMeshFormat(const String& ext);
  TK_API bool IsLayer(const String& file);
  TK_API String GetPluginExtention();

  // String operations.
  //////////////////////////////////////////

  TK_API void Split(const String& s, const String& sep, StringArray& v);

  // Replace all occurrences of a string in another string.
  TK_API void ReplaceStringInPlace(String& subject, const String& search, const String& replace);

  // Replace first occurrences of a string in another string.
  TK_API void ReplaceFirstStringInPlace(String& subject, const String& search, const String& replace);

  TK_API void ReplaceCharInPlace(String& subject, const char search, const char replace);

  TK_API int CountChar(const String& str, const char chr);

  /**
   * Removes the first occurrence of the specified substring "toRemove" from the given string "str".
   * @param str The string from which the substring will be removed.
   * @param toRemove The substring to be removed from "str".
   * @return true if the substring was found and removed; otherwise, false.
   */
  TK_API bool RemoveString(String& str, const String& toRemove);

  /**
   * Transform ascii chars to lower. Intended usage is extension comparison.
   */
  TK_API String ToLower(const String& str);
  TK_API String Format(const char* msg, ...);
  TK_API String Trim(const String& str, const String& whitespace = " \t");
  TK_API bool StartsWith(const String& str, const String& prefix);
  TK_API bool EndsWith(const String& str, const String& suffix);

  TK_API bool Utf8CaseInsensitiveSearch(const String& text, const String& search);

  // Debug geometries.
  //////////////////////////////////////////

  TK_API LineBatchPtr CreatePlaneDebugObject(PlaneEquation plane, float size);
  TK_API LineBatchPtr CreateLineDebugObject(const Vec3Array& corners);
  TK_API LineBatchPtr CreateBoundingBoxDebugObject(const BoundingBox& box,
                                                   const Vec3& color     = Vec3(1.0f, 0.0f, 0.0f),
                                                   float size            = 2.0f,
                                                   const Mat4* transform = nullptr);

  TK_API LineBatchPtr CreateDebugFrustum(const CameraPtr camera,
                                         const Vec3& color = Vec3(1.0f, 0.0f, 0.0f),
                                         float size        = 2.0f);

  // Entity operations.
  //////////////////////////////////////////

  TK_API IDArray ToEntityIdArray(const EntityPtrArray& ptrArray);
  TK_API EntityRawPtrArray ToEntityRawPtrArray(const EntityPtrArray& ptrArray);
  TK_API EntityPtrArray ToEntityPtrArray(const EntityRawPtrArray& rawPtrArray);
  TK_API bool IsInArray(const EntityRawPtrArray& nttArray, Entity* ntt);
  TK_API void GetRootEntities(const EntityPtrArray& entities, EntityPtrArray& roots);

  /** Converts an ObjectPTr to RawPtr */
  template <typename T>
  std::vector<T*> ToRawPtrArray(const std::vector<std::shared_ptr<T>>& objectArray)
  {
    // static_assert(T::StaticClass()->IsA(Object));

    std::vector<T*> rawPtrArray;
    rawPtrArray.resize(objectArray.size());
    for (size_t i = 0; i < objectArray.size(); i++)
    {
      rawPtrArray.push_back(objectArray[i].get());
    }

    return rawPtrArray;
  }

  /** Move entities of type T to filtered array. */
  template <typename T>
  void MoveByType(EntityRawPtrArray& entities, std::vector<T*>& filtered)
  {
    auto it = std::partition(entities.begin(),
                             entities.end(),
                             [&filtered](Entity* ntt)
                             {
                               if (static_cast<Object*>(ntt)->IsA<T>())
                               {
                                 filtered.emplace_back(static_cast<T*>(ntt));
                                 return false; // Keep elements of type T.
                               }
                               return true; // Remove elements thats not of type T.
                             });

    entities.erase(it, entities.end());
  }

  TK_API void GetParents(const EntityPtr ntt, EntityPtrArray& parents);

  // Gather hierarchy from parent (indx 0) to child (indx end).
  // Revert the array for child to parent.
  TK_API void GetChildren(const EntityPtr ntt, EntityPtrArray& children);

  // {copies} First one is the copy root, fallowing are attached children.
  TK_API EntityPtr DeepCopy(EntityPtr root, EntityPtrArray& copies);

  // Copies the node with children
  TK_API Node* DeepNodeCopy(Node* node);

  // Memory operations.
  //////////////////////////////////////////

  // Useful to force plugin modules to allocate from main toolkit module.
  TK_API void* TKMalloc(size_t sz);
  //  Use in combination with TKMalloc to free from main toolkit module.
  TK_API void TKFree(void* m);

  // Container operations.
  //////////////////////////////////////////

  TK_API int IndexOf(EntityPtr ntt, const EntityPtrArray& entities);
  TK_API bool Exist(const IntArray& vec, int val);

  template <typename T, typename Predicate>
  void move_values(std::vector<T>& from, std::vector<T>& to, Predicate p)
  {
    to.reserve(from.size()); // Reserve space for potential moved elements.

    auto it = std::partition(from.begin(),
                             from.end(),
                             [&to, &p](T& val)
                             {
                               if (p(val))
                               {
                                 to.emplace_back(std::move(val));
                                 return false; // Keep elements that satisfy the predicate.
                               }
                               return true; // Remove elements that don't satisfy the predicate.
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

  template <class _Tp, class _Up>
  inline std::shared_ptr<_Tp> tk_reinterpret_pointer_cast(const std::shared_ptr<_Up>& __r)
  {
    return std::shared_ptr<_Tp>(__r, reinterpret_cast<_Tp*>(__r.get()));
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

  template <typename T>
  bool remove(std::vector<T>& arr, const T& val)
  {
    int i = FindIndex(arr, val);
    if (i != -1)
    {
      arr.erase(arr.begin() + i);
      return false;
    }

    return true;
  }

  template <typename T, uint64 N>
  inline constexpr uint64 ArraySize(const T (&)[N])
  {
    return N;
  }

  template <typename Key, typename Value>
  bool HaveSameKeys(const std::unordered_map<Key, Value>& map1, const std::unordered_map<Key, Value>& map2)
  {
    if (map1.size() != map2.size())
    {
      return false; // If the sizes are different, they can't have the same keys
    }

    for (const auto& pair : map1)
    {
      if (map2.find(pair.first) == map2.end())
      {
        return false; // Key not found in map2
      }
    }

    return true; // All keys in map1 are found in map2
  }

  /** Remove duplicate elements in a range from the vector. */
  template <typename T>
  void RemoveDuplicates(std::vector<T>& vec,
                        typename std::vector<T>::iterator begin,
                        typename std::vector<T>::iterator end)
  {
    // First, sort the range
    std::sort(begin, end);

    // Use std::unique to move duplicates to the end
    auto last = std::unique(begin, end);

    // Erase the duplicates
    vec.erase(last, end);
  }

  //  Time.
  //////////////////////////////////////////

  TK_API float MillisecToSec(float ms);

  /**
   * @return Elapsed time from the ToolKit initialization.
   */
  TK_API float GetElapsedMilliSeconds();

  // Hash.
  //////////////////////////////////////////

  TK_API uint64 MurmurHash(uint64 x);

  TK_API void Xoroshiro128PlusSeed(uint64 s[2], uint64 seed);

  TK_API uint64 Xoroshiro128Plus(uint64 s[2]);

} // namespace ToolKit
