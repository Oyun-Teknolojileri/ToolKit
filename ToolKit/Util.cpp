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

#include "Util.h"

#include "Audio.h"
#include "Common/utf8.h"
#include "FileManager.h"
#include "Material.h"
#include "MathUtil.h"
#include "Mesh.h"
#include "Primative.h"
#include "Scene.h"
#include "Shader.h"
#include "SpriteSheet.h"
#include "ToolKit.h"
#include <filesystem>
#include <unordered_set>

#include "DebugNew.h"

namespace ToolKit
{

  template <typename T>
  void ReadVec(XmlNode* node, T& val)
  {
    static const char letters[] = {'x', 'y', 'z', 'w'};
    int limit                   = glm::min(val.length(), 4);
    for (int i = 0; i < limit; i++)
    {
      XmlAttribute* attr = node->first_attribute(letters + i, 1);
      if constexpr (std::is_integral_v<typename T::value_type>)
      {
        val[i] = static_cast<int>(std::atoi(attr->value()));
      }
      else if constexpr (std::is_floating_point_v<typename T::value_type>)
      {
        val[i] = static_cast<float>(std::atof(attr->value()));
      }
    }
  }

  template TK_API void ReadVec(XmlNode* node, UVec2& val);
  template TK_API void ReadVec(XmlNode* node, IVec2& val);
  template TK_API void ReadVec(XmlNode* node, Vec2& val);
  template TK_API void ReadVec(XmlNode* node, Vec3& val);
  template TK_API void ReadVec(XmlNode* node, glm::ivec3& val);
  template TK_API void ReadVec(XmlNode* node, Quaternion& val);
  template TK_API void ReadVec(XmlNode* node, Vec4& val);
  template TK_API void ReadVec(XmlNode* node, UVec4& val);

  template <typename T>
  void WriteVec(XmlNode* node, XmlDocument* doc, const T& val)
  {
    static const String letters[] = {"x", "y", "z", "w"};
    int limit                     = glm::min(val.length(), 4);
    for (int i = 0; i < limit; i++)
    {
      WriteAttr(node, doc, letters[i], std::to_string(val[i]));
    }
  }

  template TK_API void WriteVec(XmlNode* node, XmlDocument* doc, const UVec2& val);
  template TK_API void WriteVec(XmlNode* node, XmlDocument* doc, const IVec2& val);
  template TK_API void WriteVec(XmlNode* node, XmlDocument* doc, const Vec2& val);
  template TK_API void WriteVec(XmlNode* node, XmlDocument* doc, const Vec3& val);
  template TK_API void WriteVec(XmlNode* node, XmlDocument* doc, const Vec4& val);
  template TK_API void WriteVec(XmlNode* node, XmlDocument* doc, const UVec4& val);
  template TK_API void WriteVec(XmlNode* node, XmlDocument* doc, const Quaternion& val);

  void WriteAttr(XmlNode* node, XmlDocument* doc, const StringView& name, const StringView& val)
  {
    node->append_attribute(
        doc->allocate_attribute(doc->allocate_string(name.data(), 0), doc->allocate_string(val.data(), 0)));
  }

  template <typename T>
  T ReadVal(XmlNode* node, const String& name)
  {
    if (XmlAttribute* attr = node->first_attribute(name.c_str()))
    {
      if constexpr (std::is_integral_v<T>)
      {
        if constexpr (std::is_unsigned_v<T>)
        {
          return static_cast<T>(std::stoull(attr->value()));
        }
        else
        {
          return static_cast<T>(std::atoi(attr->value()));
        }
      }
      else if constexpr (std::is_floating_point_v<T>)
      {
        return static_cast<float>(std::atof(attr->value()));
      }
    }

    if constexpr (std::is_same_v<T, bool>)
    {
      return 1;
    }

    return 0;
  }

  void ReadAttr(XmlNode* node, const String& name, bool& val) { val = ReadVal<bool>(node, name); }

  void ReadAttr(XmlNode* node, const String& name, float& val) { val = ReadVal<float>(node, name); }

  void ReadAttr(XmlNode* node, const String& name, int& val) { val = ReadVal<int>(node, name); }

  void ReadAttr(XmlNode* node, const String& name, uint& val) { val = ReadVal<uint>(node, name); }

  void ReadAttr(XmlNode* node, const String& name, ULongID& val) { val = ReadVal<ULongID>(node, name); }

  void ReadAttr(XmlNode* node, const String& name, byte& val) { val = ReadVal<byte>(node, name); }

  void ReadAttr(XmlNode* node, const String& name, ubyte& val) { val = ReadVal<ubyte>(node, name); }

  void ReadAttr(XmlNode* node, const String& name, String& val, StringView defaultVal)
  {
    if (XmlAttribute* attr = node->first_attribute(name.c_str()))
    {
      val = attr->value();
    }
    else
    {
      val = defaultVal;
    }
  }

  XmlNode* Query(XmlDocument* doc, const StringArray& path)
  {
    XmlNode* node = doc->first_node(path.front().c_str());
    for (size_t i = 1; i < path.size(); i++)
    {
      if (node == nullptr)
      {
        return nullptr;
      }

      node = node->first_node(path[i].c_str());
    }

    return node;
  }

  bool UpdateAttribute(XmlDocument* doc, const StringArray& path, const String& attribute, const String& val)
  {
    if (XmlNode* node = Query(doc, path))
    {
      if (XmlAttribute* attr = node->first_attribute(attribute.c_str()))
      {
        node->value(val.c_str());
      }
      else
      {
        XmlAttribute* newAttr = doc->allocate_attribute(attribute.c_str(), val.c_str());
        node->append_attribute(newAttr);
      }

      return true;
    }

    return false;
  }

  XmlNode* CreateXmlNode(XmlDocument* doc, const StringView& name, XmlNode* parent)
  {
    assert(doc);

    char* str     = doc->allocate_string(name.data());
    XmlNode* node = doc->allocate_node(rapidxml::node_type::node_element, str);

    if (parent)
    {
      parent->append_node(node);
    }
    else
    {
      doc->append_node(node);
    }

    return node;
  }

  void WriteMaterial(XmlNode* parent, XmlDocument* doc, const String& file)
  {
    XmlNode* material = doc->allocate_node(rapidxml::node_type::node_element, "material");
    parent->append_node(material);

    String matPath = GetRelativeResourcePath(file);
    if (matPath.empty())
    {
      matPath = MaterialPath("default.material", true);
    }

    XmlAttribute* nameAttr = doc->allocate_attribute("name", doc->allocate_string(matPath.c_str()));
    material->append_attribute(nameAttr);
  }

  MaterialPtr ReadMaterial(XmlNode* parent)
  {
    if (XmlNode* materialNode = parent->first_node("material"))
    {
      String path = materialNode->first_attribute("name")->value();
      NormalizePathInplace(path);
      String matFile = MaterialPath(path);
      return GetMaterialManager()->Create<Material>(matFile);
    }

    return nullptr;
  }

  bool CheckSystemFile(StringView path) { return std::filesystem::exists(path); }

  bool CheckFile(const String& path) { return GetFileManager()->CheckFileFromResources(path); }

  String CreateCopyFileFullPath(const String& fullPath)
  {
    String cpyPath;
    if (!fullPath.empty())
    {
      String path, name, ext;
      DecomposePath(fullPath, &path, &name, &ext);
      cpyPath = ConcatPaths({path, name + "_copy" + ext});
      if (CheckFile(cpyPath))
      {
        int i = 1;
        do
        {
          cpyPath = ConcatPaths({path, name + "_copy(" + std::to_string(i++) + ")" + ext});
        } while (CheckFile(cpyPath));
      }
    }

    return cpyPath;
  }

  void DecomposePath(const String& fullPath, String* path, String* name, String* ext)
  {
    String normal = NormalizePath(fullPath);

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

  void NormalizePathInplace(String& path)
  {
#if __clang__
    UnixifyPath(path);
#else
    DosifyPath(path);
#endif
  }
  
  String NormalizePath(String path)
  {
#if __clang__
    UnixifyPath(path);
#else
    DosifyPath(path);
#endif
    return path;
  }

  int RunPipe(const String& command, std::function<void(int)> afterFn);
  {
#ifdef _WIN32
    FILE* fp = _popen(command.c_str(), "r");
#else
    FILE* fp = popen(command.c_str(), "r");
#endif
    if (fp == nullptr)
    {
      TK_ERR("pipe run failed! command: %s", command.c_str());
      afterFn(1);
      return 0;
    }
    char path[512] {};
    while (fgets(path, sizeof(path), fp) != NULL)
    {
      TK_LOG("%s", path);
    }

    int res = fclose(fp);
    if (afterFn) 
    {
      afterFn(res);
    }
    return res;
  }

  void UnixifyPath(String& path) { ReplaceCharInPlace(path, '\\', '/'); }

  void DosifyPath(String& path) { ReplaceCharInPlace(path, '/', '\\'); }

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

  String GetRelativeResourcePath(const String& path, String* rootFolder)
  {
    // Check workspace relativity.
    String root  = Main::GetInstance()->m_resourceRoot;
    size_t exist = path.find(root);

    // Check install path relativity.
    bool toolKit = false;
    if (exist == String::npos)
    {
      root    = ResourcePath(true);
      exist   = path.find(root, 0);
      toolKit = true;
    }

    if (exist == String::npos)
    {
      // ToolKit resource absolute path
      root  = std::filesystem::absolute(root).string();
      exist = path.find(root, 0);
    }

    if (exist != String::npos)
    {
      // If path isn't absolute
      if (!root.length())
      {
        return path;
      }
      else
      {
        String rel = path.substr(root.length() + 1);
        // Extract the root layer. Mesh, Texture ect...
        exist      = rel.find(GetPathSeparator());
        if (exist != String::npos)
        {
          if (rootFolder != nullptr)
          {
            *rootFolder = rel.substr(0, exist);
          }
          rel = rel.substr(exist + 1);
        }

        if (toolKit)
        {
          //  Any relative path starting with ToolKit root directory
          //  will be search in the default path.
          rel = ConcatPaths({"ToolKit", rel});
        }

        return rel;
      }
    }

    return path;
  }

  bool IsDefaultResource(const String& path)
  {
    if (HasToolKitRoot(path))
    {
      return true;
    }

    static const String defPath = DefaultPath();
    if (StartsWith(path, defPath))
    {
      return true;
    }

    return false;
  }

  bool HasToolKitRoot(const String& path) 
  {
    return StartsWith(path, "ToolKit\\") || StartsWith(path, "ToolKit/"); 
  }

  String GetFileName(const String& path)
  {
    char sep = GetPathSeparator();
    int i    = (int) path.find_last_of(sep) + 1;
    return path.substr(i);
  }

  String CreatePathFromResourceType(const String& file, TKClass* Class) { return GetResourcePath(Class) + file; }

  TKClass* GetResourceType(const String& ext)
  {
    if (ext == MESH || ext == SKINMESH || SupportedMeshFormat(ext))
    {
      return Mesh::StaticClass();
    }

    if (ext == ANIM)
    {
      return Animation::StaticClass();
    }

    if (ext == MATERIAL)
    {
      return Material::StaticClass();
    }

    if (SupportedImageFormat(ext))
    {
      if (ext == HDR)
      {
        return Hdri::StaticClass();
      }

      return Texture::StaticClass();
    }

    if (ext == SHADER)
    {
      return Shader::StaticClass();
    }

    if (ext == AUDIO)
    {
      return Audio::StaticClass();
    }

    if (ext == SCENE)
    {
      return Scene::StaticClass();
    }

    if (ext == SKELETON)
    {
      return Skeleton::StaticClass();
    }

    assert(false && "Extension does not map to a Class.");
    return nullptr;
  }

  String GetExtFromType(TKClass* Class)
  {
    if (Class == Animation::StaticClass())
    {
      return ANIM;
    }
    if (Class == Audio::StaticClass())
    {
      return AUDIO;
  }
    if (Class == Material::StaticClass())
  {
      return MATERIAL;
    }
    if (Class == Mesh::StaticClass())
    {
      return MESH;
    }
    if (Class == SkinMesh::StaticClass())
    {
      return SKINMESH;
    }
    if (Class == Shader::StaticClass())
    {
      return SHADER;
    }
    if (Class == Scene::StaticClass())
    {
      return SCENE;
    }

    assert(false && "Resource type does not have a corresponding extension.");
    return String();
  }

  String GetResourcePath(TKClass* Class)
  {
    if (Class == Animation::StaticClass())
    {
      return AnimationPath("");
    }
    if (Class == Audio::StaticClass())
    {
      return AudioPath("");
    }
    if (Class == Material::StaticClass())
    {
      return MaterialPath("");
    }
    if (Class->IsSublcassOf(Mesh::StaticClass()))
    {
      return MeshPath("");
    }
    if (Class == Shader::StaticClass())
    {
      return ShaderPath("");
    }
    if (Class == SpriteSheet::StaticClass())
    {
      return SpritePath("");
    }
    if (Class->IsSublcassOf(Texture::StaticClass()))
    {
      return TexturePath("");
    }
    if (Class == Scene::StaticClass())
    {
      return ScenePath("");
    }

    assert(false && "Resource type does not have a dedicated folder.");

    return String();
  }

  char GetPathSeparator()
  {
#ifndef __clang__
    return '\\';
#else
    return '/';
#endif
  }

  String GetPathSeparatorAsStr() { return String() + GetPathSeparator(); }

  bool SupportedImageFormat(const String& ext)
  {
    if (ext.empty())
    {
      return false;
    }

    static String supportedFormats(PNG + JPG + JPEG + TGA + BMP + PSD + HDR);
    return supportedFormats.find(ToLower(ext)) != String::npos;
  }

  bool SupportedMeshFormat(const String& ext)
  {
    static String supportedFormats(FBX + GLB + GLTF + OBJ);
    return supportedFormats.find(ToLower(ext)) != String::npos;
  }

  bool IsLayer(const String& file) { return file.find(ToLower(LAYER)) != String::npos; }

  // split a string into multiple sub strings, based on a separator string
  // for example, if separator="::",
  // s = "abc::def xy::st:" -> "abc", "def xy" and "st:",
  // https://stackoverflow.com/questions/53849/how-do-i-tokenize-a-string-in-c?page=2&tab=votes#tab-top
  // // NOLINT
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
    while ((pos = subject.find(search, pos)) != std::string::npos)
    {
      subject.replace(pos, search.length(), replace);
      pos += replace.length();
    }
  }

  void ReplaceFirstStringInPlace(String& subject, const String& search, const String& replace)
  {
    size_t pos = 0;
    if ((pos = subject.find(search, pos)) != std::string::npos)
    {
      subject.replace(pos, search.length(), replace);
    }
  }

  void ReplaceCharInPlace(String& subject, const char search, const char replace)
  {
    for (char& ch : subject)
    {
      if (ch == search)
      {
        ch = replace;
      }
    }
  }

  int CountChar(const String& str, const char chr)
  {
    int cnt = 0;
    for (char c : str)
    {
      if (c == chr)
      {
        cnt++;
      }
    }

    return cnt;
  }

  String ToLower(const String& str)
  {
    String lwr = str;
    transform(lwr.begin(), lwr.end(), lwr.begin(), ::tolower);
    return lwr;
  }

  bool Utf8CaseInsensitiveSearch(const String& text, const String& search)
  {
    char* findPoint = utf8casestr(text.c_str(), search.c_str());
    return findPoint && utf8size_lazy(findPoint) != 0;
  }

  String Format(const char* msg, ...)
  {
    va_list args;
    va_start(args, msg);

    static char buff[2048];
    vsprintf(buff, msg, args);
    String msgFormatted(buff);

    va_end(args);

    return msgFormatted;
  }

  String Trim(const std::string& str, const String& whitespace)
  {
    const size_t strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
    {
      return ""; // no content
    }

    const size_t strEnd   = str.find_last_not_of(whitespace);
    const size_t strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
  }

  bool StartsWith(const String& str, const String& prefix) { return str.find(prefix) == 0; }

  bool EndsWith(const String& str, const String& suffix)
  {
    return str.rfind(suffix) == glm::abs(str.size() - suffix.size());
  }

  LineBatchPtr CreatePlaneDebugObject(PlaneEquation plane, float size)
  {
    // Searching perpendicular axes on the plane.
    Vec3 z = plane.normal;
    Vec3 x = z + Vec3(1.0f);
    Vec3 y = glm::cross(z, x);
    x      = glm::normalize(glm::cross(y, z));
    y      = glm::normalize(glm::cross(z, x));

    NormalizePlaneEquation(plane);
    Vec3 o      = plane.normal * plane.d;

    float hSize = size * 0.5f;
    Vec3Array corners {o + x * hSize + y * hSize,
                       o - x * hSize + y * hSize,
                       o - x * hSize - y * hSize,
                       o + x * hSize - y * hSize};

    LineBatchPtr obj = MakeNewPtr<LineBatch>();
    obj->Generate(corners, X_AXIS, DrawType::LineLoop, 5.0f);

    return obj;
  }

  LineBatchPtr CreateLineDebugObject(const Vec3Array& corners)
  {
    LineBatchPtr obj = MakeNewPtr<LineBatch>();
    obj->Generate(corners, X_AXIS, DrawType::LineLoop, 5.0f);

    return obj;
  }

  LineBatchPtr CreateBoundingBoxDebugObject(const BoundingBox& box,
                                            const Vec3& color,
                                            float size,
                                            const Mat4* transform)
  {
    Vec3Array corners;
    GetCorners(box, corners);

    std::vector<Vec3> vertices = {
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
        corners[3]  // FBL
    };

    if (transform != nullptr)
    {
      for (Vec3& v : vertices)
      {
        v = *transform * Vec4(v, 1.0f);
      }
    }

    LineBatchPtr lineForm = MakeNewPtr<LineBatch>();
    lineForm->Generate(vertices, color, DrawType::LineStrip, size);

    return lineForm;
  }

  void ToEntityIdArray(EntityIdArray& idArray, const EntityPtrArray& ptrArray)
  {
    idArray.reserve(ptrArray.size());
    for (EntityPtr ntt : ptrArray)
    {
      idArray.push_back(ntt->GetIdVal());
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

  void RootsOnly(const EntityPtrArray& entities, EntityPtrArray& roots, EntityPtr child)
  {
    auto AddUnique = [&roots](EntityPtr ntt) -> void
    {
      assert(ntt != nullptr);
      bool unique = std::find(roots.begin(), roots.end(), ntt) == roots.end();
      if (unique)
      {
        roots.push_back(ntt);
      }
    };

    Node* parent = child->m_node->m_parent;
    if (parent != nullptr)
    {
      EntityPtr parentEntity = parent->m_entity;
      if (contains(entities, parentEntity))
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

  void GetRootEntities(const EntityPtrArray& entities, EntityPtrArray& roots)
  {
    for (EntityPtr ntt : entities)
    {
      RootsOnly(entities, roots, ntt);
    }
  }

  void GetParents(const EntityPtr ntt, EntityPtrArray& parents)
  {
    if (Node* pNode = ntt->m_node->m_parent)
    {
      if (EntityPtr parent = pNode->m_entity)
      {
        parents.push_back(parent);
        GetParents(parent, parents);
      }
    }
  }

  void GetChildren(const EntityPtr ntt, EntityPtrArray& children)
  {
    if (ntt == nullptr)
    {
      return;
    }

    for (Node* childNode : ntt->m_node->m_children)
    {
      EntityPtr child = childNode->m_entity;
      if (child)
      {
        children.push_back(child);
        GetChildren(child, children);
      }
    }
  }

  EntityPtr DeepCopy(EntityPtr root, EntityPtrArray& copies)
  {
    EntityPtr cpy = std::static_pointer_cast<Entity>(root->Copy());
    copies.push_back(cpy);

    for (Node* node : root->m_node->m_children)
    {
      if (node->m_entity)
      {
        if (EntityPtr sub = DeepCopy(node->m_entity, copies))
        {
          cpy->m_node->AddChild(sub->m_node);
        }
      }
    }

    return cpy;
  }

  static void RecursiveCopyDirectoryWithSet(const String& source,
                                            const String& destination,
                                            const std::unordered_set<uint64>& ignoredExtSet) 
  {
    using namespace std::filesystem;
    String ext;
    DecomposePath(source, nullptr, nullptr, &ext);
    if (ignoredExtSet.count(std::hash<String> {}(ext)) != 0)
    {
      return;
    }

    // Create the destination directory if it doesn't exist
    if (!exists(destination))
    {
      create_directories(destination);
    }

    for (const auto& entry : directory_iterator(source))
    {
      const path current_path = entry.path();
      const path new_path     = destination / current_path.filename();

      String ext;
      DecomposePath(current_path.u8string(), nullptr, nullptr, &ext);

      if (ignoredExtSet.count(std::hash<String> {}(ext)) != 0) 
      {
        continue;
      }

      if (is_directory(current_path))
      {
        RecursiveCopyDirectoryWithSet(current_path.generic_u8string(), new_path.generic_u8string(), ignoredExtSet);
      }
      else if (is_regular_file(current_path))
      {
        copy_file(current_path, new_path, copy_options::overwrite_existing);
      }
    }
  }

  void RecursiveCopyDirectory(const String& source,
                              const String& destination,
                              const StringArray& ignoredExtensions)
  {
    std::unordered_set<uint64> ignoredSet;
    for (int i = 0; i < ignoredExtensions.size(); i++) 
    {
      ignoredSet.insert(std::hash<String> {}(ignoredExtensions[i]));
    }
    RecursiveCopyDirectoryWithSet(source, destination, ignoredSet);
  }

  void* TKMalloc(size_t sz) { return malloc(sz); }

  void TKFree(void* m) { free(m); }

  int IndexOf(EntityPtr ntt, const EntityPtrArray& entities)
  {
    auto it = std::find(entities.begin(), entities.end(), ntt);

    if (it != entities.end())
    {
      return static_cast<int>(it - entities.begin());
    }

    return -1;
  }

  bool Exist(const IntArray& vec, int val)
  {
    for (int v : vec)
    {
      if (v == val)
      {
        return true;
      }
    }

    return false;
  }

  float MillisecToSec(float ms) { return ms / 1000.0f; }

  float GetElapsedMilliSeconds()
  {
    namespace ch                                    = std::chrono;
    static ch::high_resolution_clock::time_point t1 = ch::high_resolution_clock::now();
    ch::high_resolution_clock::time_point t2        = ch::high_resolution_clock::now();
    ch::duration<double> timeSpan                   = ch::duration_cast<ch::duration<double>>(t2 - t1);
    return static_cast<float>(timeSpan.count() * 1000.0);
  }

} //  namespace ToolKit
