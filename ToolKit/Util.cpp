#include "Util.h"

#include <string>
#include <cstdarg>
#include <fstream>
#include <filesystem>
#include <algorithm>

#include "rapidxml.hpp"
#include "Primative.h"
#include "ToolKit.h"
#include "DebugNew.h"

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
        val[i] = static_cast<int> (std::atoi(attr->value()));
      }
      else if constexpr (std::is_floating_point_v<typename T::value_type>)
      {
        val[i] = static_cast<float> (std::atof(attr->value()));
      }
    }
  }

  template TK_API void ReadVec(XmlNode* node, Vec2& val);
  template TK_API void ReadVec(XmlNode* node, Vec3& val);
  template TK_API void ReadVec(XmlNode* node, glm::ivec3& val);
  template TK_API void ReadVec(XmlNode* node, Quaternion& val);
  template TK_API void ReadVec(XmlNode* node, Vec4& val);
  template TK_API void ReadVec(XmlNode* node, UVec4& val);

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

  template TK_API void WriteVec
  (
    XmlNode* node, XmlDocument* doc, const Vec2& val
  );
  template TK_API void WriteVec
  (
    XmlNode* node, XmlDocument* doc, const Vec3& val
  );
  template TK_API void WriteVec
  (
    XmlNode* node, XmlDocument* doc, const Vec4& val
  );
  template TK_API void WriteVec
  (
    XmlNode* node, XmlDocument* doc, const Quaternion& val
  );

  void WriteAttr
  (
    XmlNode* node,
    XmlDocument* doc,
    const String& name,
    const String& val
  )
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
        return static_cast<int> (std::atoi(attr->value()));
      }
      else if constexpr (std::is_floating_point_v<T>)
      {
        return static_cast<float> (std::atof(attr->value()));
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

  void ReadAttr(XmlNode* node, const String& name, ULongID& val)
  {
    val = ReadVal<ULongID>(node, name);
  }

  void ReadAttr(XmlNode* node, const String& name, byte& val)
  {
    val = ReadVal<byte>(node, name);
  }

  void ReadAttr(XmlNode* node, const String& name, ubyte& val)
  {
    val = ReadVal<ubyte>(node, name);
  }

  void ReadAttr(XmlNode* node, const String& name, String& val)
  {
    if (XmlAttribute* attr = node->first_attribute(name.c_str()))
    {
      val = attr->value();
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

  bool UpdateAttribute
  (
    XmlDocument* doc,
    const StringArray& path,
    const String& attribute,
    const String& val
  )
  {
    if (XmlNode* node = Query(doc, path))
    {
      if (XmlAttribute* attr = node->first_attribute(attribute.c_str()))
      {
        node->value(val.c_str());
      }
      else
      {
        XmlAttribute* newAttr = doc->allocate_attribute
        (
          attribute.c_str(),
          val.c_str()
        );
        node->append_attribute(newAttr);
      }

      return true;
    }

    return false;
  }

  XmlNode* CreateXmlNode(XmlDocument* doc, const String& name, XmlNode* parent)
  {
    assert(doc);

    char* str = doc->allocate_string(name.c_str());
    XmlNode* node = doc->allocate_node
    (
      rapidxml::node_type::node_element,
      str
    );

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
    XmlNode* material = doc->allocate_node
    (
      rapidxml::node_type::node_element,
      "material"
    );
    parent->append_node(material);

    String matPath = GetRelativeResourcePath(file);
    if (matPath.empty())
    {
      matPath = MaterialPath("default.material", true);
    }

    XmlAttribute* nameAttr = doc->allocate_attribute
    (
      "name",
      doc->allocate_string(matPath.c_str())
    );
    material->append_attribute(nameAttr);
  }

  MaterialPtr ReadMaterial(XmlNode* parent)
  {
    if (XmlNode* materialNode = parent->first_node("material"))
    {
      String path = materialNode->first_attribute("name")->value();
      NormalizePath(path);
      String matFile = MaterialPath(path);
      return GetMaterialManager()->Create<Material>(matFile);
    }

    return nullptr;
  }

  bool CheckFile(const String& path)
  {
    return GetFileManager()->CheckFileFromResources(path);
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
          cpyPath = ConcatPaths
          (
            { path, name + "_copy(" + std::to_string(i++) + ")" + ext }
          );
        } while (CheckFile(cpyPath));
      }
    }

    return cpyPath;
  }

  void DecomposePath
  (
    const String& fullPath,
    String* path,
    String* name,
    String* ext
  )
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
#ifdef __EMSCRIPTEN__
    UnixifyPath(path);
#else
    DosifyPath(path);
#endif
  }

  void UnixifyPath(String& path)
  {
    ReplaceCharInPlace(path, '\\', '/');
  }

  void DosifyPath(String& path)
  {
    ReplaceCharInPlace(path, '/', '\\');
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

  String GetRelativeResourcePath(const String& path)
  {
    // Check workspace relativity.
    String root = Main::GetInstance()->m_resourceRoot;
    size_t exist = path.find(root);

    // Check install path relativity.
    bool toolKit = false;
    if (exist == String::npos)
    {
      root = ResourcePath(true);
      exist = path.find(root, 0);
      toolKit = true;
    }

    if (exist == String::npos)
    {
      // ToolKit resource absolute path
      root = std::filesystem::absolute(root).string();
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
        exist = rel.find(GetPathSeparator());
        if (exist != String::npos)
        {
          rel = rel.substr(exist + 1);
        }

        if (toolKit)
        {
          //  Any relative path starting with ToolKit root directory
          //  will be search in the default path.
          rel = ConcatPaths({ "ToolKit", rel });
        }

        return rel;
      }
    }


    return path;
  }

  String CreatePathFromResourceType(const String& file, ResourceType type)
  {
    return GetResourcePath(type) + file;
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

    if (ext == HDR)
    {
      return ResourceType::Hdri;
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
    case ResourceType::Skeleton:
      return "Skeleton";
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

  char GetPathSeparator()
  {
#ifndef __EMSCRIPTEN__
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
    if (ext.empty())
    {
      return false;
    }

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
  // https://stackoverflow.com/questions/53849/how-do-i-tokenize-a-string-in-c?page=2&tab=votes#tab-top // NOLINT
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
  void ReplaceStringInPlace
  (
    String& subject,
    const String& search,
    const String& replace
  )
  {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos)
    {
      subject.replace(pos, search.length(), replace);
      pos += replace.length();
    }
  }

  void ReplaceFirstStringInPlace
  (
    String& subject,
    const String& search,
    const String& replace
  )
  {
    size_t pos = 0;
    if ((pos = subject.find(search, pos)) != std::string::npos)
    {
      subject.replace(pos, search.length(), replace);
    }
  }

  void ReplaceCharInPlace
  (
    String& subject,
    const char search,
    const char replace
  )
  {
    for (char& ch : subject)
    {
      if (ch == search)
      {
        ch = replace;
      }
    }
  }

  String ToLower(const String& str)
  {
    String lwr = str;
    transform(lwr.begin(), lwr.end(), lwr.begin(), ::tolower);
    return lwr;
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
      return "";  // no content
    }

    const size_t strEnd = str.find_last_not_of(whitespace);
    const size_t strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
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

  LineBatch* CreateBoundingBoxDebugObject
  (
    const BoundingBox& box,
    const Vec3& color,
    float size,
    const Mat4* transform
  )
  {
    Vec3Array corners;
    GetCorners(box, corners);

    std::vector<Vec3> vertices =
    {
      corners[0],  // FTL
      corners[3],  // FBL
      corners[2],  // FBR
      corners[1],  // FTR
      corners[0],  // FTL
      corners[4],  // BTL
      corners[7],  // BBL
      corners[6],  // BBR
      corners[5],  // BTR
      corners[4],  // BTL
      corners[5],  // BTR
      corners[1],  // FTR
      corners[2],  // FBR
      corners[6],  // BBR
      corners[7],  // BBL
      corners[3]  // FBL
    };

    if (transform != nullptr)
    {
      for (Vec3& v : vertices)
      {
        v = *transform * Vec4(v, 1.0f);
      }
    }

    LineBatch* lineForm = new LineBatch
    (
      vertices,
      color,
      DrawType::LineStrip,
      size
    );
    return lineForm;
  }

  void ToEntityIdArray
  (
    EntityIdArray& idArray,
    const EntityRawPtrArray& ptrArray
  )
  {
    idArray.reserve(ptrArray.size());
    for (Entity* ntt : ptrArray)
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

  void RootsOnly
  (
    const EntityRawPtrArray& entities,
    EntityRawPtrArray& roots,
    Entity* child
  )
  {
    auto AddUnique = [&roots](Entity* e) -> void
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
      if
      (
        std::find
        (
          entities.begin(),
        entities.end(),
        parentEntity
        ) != entities.end()
      )
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

  void GetRootEntities
  (
    const EntityRawPtrArray& entities,
    EntityRawPtrArray& roots
  )
  {
    for (Entity* e : entities)
    {
      RootsOnly(entities, roots, e);
    }
  }

  void GetParents(const Entity* ntt, EntityRawPtrArray& parents)
  {
    if (Node* pNode = ntt->m_node->m_parent)
    {
      if (Entity* parent = pNode->m_entity)
      {
        parents.push_back(parent);
        GetParents(parent, parents);
      }
    }
  }

  void GetChildren(const Entity* ntt, EntityRawPtrArray& children)
  {
    if (ntt == nullptr)
    {
      return;
    }

    for (Node* childNode : ntt->m_node->m_children)
    {
      Entity* child = childNode->m_entity;
      if (child)
      {
        children.push_back(child);
        GetChildren(child, children);
      }
    }
  }

  Entity* DeepCopy(Entity* root, EntityRawPtrArray& copies)
  {
    Entity* cpy = root->Copy();
    copies.push_back(cpy);

    for (Node* node : root->m_node->m_children)
    {
      if (node->m_entity)
      {
        if (Entity* sub = DeepCopy(node->m_entity, copies))
        {
          cpy->m_node->AddChild(sub->m_node);
        }
      }
    }

    return cpy;
  }

  Entity* DeepInstantiate(Entity* root, EntityRawPtrArray& instances)
  {
    Entity* cpy = root->Instantiate();
    instances.push_back(cpy);

    for (Node* node : root->m_node->m_children)
    {
      if (node->m_entity)
      {
        if (Entity* sub = DeepInstantiate(node->m_entity, instances))
        {
          cpy->m_node->AddChild(sub->m_node);
        }
      }
    }

    return cpy;
  }

  TK_API void StableSortByDistanceToCamera
  (
    EntityRawPtrArray& entities,
    const Camera* cam
  )
  {
    std::function<bool(Entity*, Entity*)> sortFn =
      [cam](Entity* ntt1, Entity* ntt2) -> bool
    {
      Vec3 camLoc = cam->m_node->GetTranslation
      (
        TransformationSpace::TS_WORLD
      );

      BoundingBox bb1 = ntt1->GetAABB(true);
      float first = glm::length2(bb1.GetCenter() - camLoc);

      BoundingBox bb2 = ntt2->GetAABB(true);
      float second = glm::length2(bb2.GetCenter() - camLoc);

      return second < first;
    };

    if (cam->IsOrtographic())
    {
      sortFn = [cam](Entity* ntt1, Entity* ntt2) -> bool
      {
        float first = ntt1->m_node->GetTranslation
        (
          TransformationSpace::TS_WORLD
        ).z;

        float second = ntt2->m_node->GetTranslation
        (
          TransformationSpace::TS_WORLD
        ).z;

        return first < second;
      };
    }

    std::stable_sort(entities.begin(), entities.end(), sortFn);
  }

  TK_API void StableSortByMaterialPriority(EntityRawPtrArray& entities)
  {
    std::stable_sort
    (
      entities.begin(),
      entities.end(),
      [](Entity* a, Entity* b) -> bool
      {
        MaterialComponentPtr matA = a->GetMaterialComponent();
        MaterialComponentPtr matB = b->GetMaterialComponent();
        if (matA && matB)
        {
          int pA = matA->GetMaterialVal()->GetRenderState()->priority;
          int pB = matB->GetMaterialVal()->GetRenderState()->priority;
          return pA > pB;
        }

        return false;
      }
    );
  }

  TK_API MaterialPtr GetRenderMaterial(Entity* entity)
  {
    MaterialPtr renderMat = nullptr;
    if (MaterialComponentPtr matCom = entity->GetMaterialComponent())
    {
      renderMat = matCom->GetMaterialVal();
    }
    else if (MeshComponentPtr meshCom = entity->GetMeshComponent())
    {
      renderMat = meshCom->GetMeshVal()->m_material;
    }

    return renderMat;
  }

  void* TKMalloc(size_t sz)
  {
    return malloc(sz);
  }

  void TKFree(void* m)
  {
    free(m);
  }

  int IndexOf(Entity* ntt, const EntityRawPtrArray& entities)
  {
    EntityRawPtrArray::const_iterator it = std::find
    (
      entities.begin(),
      entities.end(),
      ntt
    );

    if (it != entities.end())
    {
      return static_cast<int> (it - entities.begin());
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

  float MillisecToSec(float ms)
  {
    return ms / 1000.0f;
  }

  float GetElapsedMilliSeconds()
  {
    namespace ch = std::chrono;
    static ch::high_resolution_clock::time_point t1 =
      ch::high_resolution_clock::now();

    ch::high_resolution_clock::time_point t2 =
      ch::high_resolution_clock::now();

    ch::duration<double> timeSpan =
      ch::duration_cast<ch::duration<double>> (t2 - t1);

    return static_cast<float>(timeSpan.count() * 1000.0);
  }

}  //  namespace ToolKit
