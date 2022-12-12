#include "assimp/DefaultLogger.hpp"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include <assert.h>
#include <rapidxml_ext.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <ToolKit.h>
#include <Util.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <rapidxml.hpp>

using std::cout;
using std::endl;
using std::fstream;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::string;
using std::to_string;
using std::unordered_map;
using std::vector;
namespace fs = std::filesystem;

char GetPathSeparator()
{
  static char sep    = '/';
  const wchar_t pref = fs::path::preferred_separator;
  wcstombs(&sep, &pref, 1);
  return sep;
}

void NormalizePath(string& path)
{
  fs::path patify = path;
  path            = patify.lexically_normal().u8string();
}

void TrunckToFileName(string& fullPath)
{
  fs::path patify = fullPath;
  fullPath        = patify.filename().u8string();
}

namespace ToolKit
{
  class BoneNode
  {
   public:
    BoneNode()
    {
    }

    BoneNode(aiNode* node, unsigned int index)
    {
      boneIndex = index;
      boneNode  = node;
    }

    aiNode* boneNode       = nullptr;
    aiBone* bone           = nullptr;
    unsigned int boneIndex = 0;
  };

  vector<string> g_usedFiles;
  bool IsUsed(string file)
  {
    return find(g_usedFiles.begin(), g_usedFiles.end(), file) ==
           g_usedFiles.end();
  }

  void AddToUsedFiles(string file)
  {
    // Add unique.
    if (IsUsed(file))
    {
      g_usedFiles.push_back(file);
    }
  }

  void ClearForbidden(std::string& str)
  {
    const std::string forbiddenChars = "\\/:?\"<>|";
    std::replace_if(
        str.begin(),
        str.end(),
        [&forbiddenChars](char c) {
          return std::string::npos != forbiddenChars.find(c);
        },
        ' ');
  }

  unordered_map<string, BoneNode> g_skeletonMap;
  SkeletonPtr g_skeleton;
  bool isSkeletonEntityCreated = false;
  const aiScene* g_scene       = nullptr;

  void Decompose(string fullPath, string& path, string& name)
  {
    NormalizePath(fullPath);
    path = "";

    size_t i = fullPath.find_last_of(GetPathSeparator());
    if (i != string::npos)
    {
      path = fullPath.substr(0, fullPath.find_last_of(GetPathSeparator()) + 1);
    }

    name = fullPath.substr(fullPath.find_last_of(GetPathSeparator()) + 1);
    name = name.substr(0, name.find_last_of('.'));
  }

  void DecomposeAssimpMatrix(aiMatrix4x4 transform,
                             Vec3* t,
                             Quaternion* r,
                             Vec3* s)
  {
    aiVector3D aiT, aiS;
    aiQuaternion aiR;
    transform.Decompose(aiS, aiR, aiT);

    *t = Vec3(aiT.x, aiT.y, aiT.z);
    *r = Quaternion(aiR.w, aiR.x, aiR.y, aiR.z);
    *s = Vec3(aiS.x, aiS.y, aiS.z);
  }

  string GetEmbeddedTextureName(const aiTexture* texture)
  {
    string name = texture->mFilename.C_Str();
    NormalizePath(name);
    name = name + "." + texture->achFormatHint;

    return name;
  }

  string GetMaterialName(aiMaterial* material, unsigned int indx)
  {
    string name = material->GetName().C_Str();
    if (name.empty())
    {
      name = "emb" + to_string(indx);
    }

    return name;
  }

  string GetMaterialName(aiMesh* mesh)
  {
    aiString matName;
    g_scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_NAME, matName);
    return GetMaterialName(g_scene->mMaterials[mesh->mMaterialIndex],
                           mesh->mMaterialIndex);
  }

  std::vector<MaterialPtr> tMaterials;
  template <typename T>
  void CreateFileAndSerializeObject(T* objectToSerialize,
                                    const String& filePath)
  {
    std::ofstream file;
    file.open(filePath.c_str(), std::ios::out);
    assert(file.is_open() && "File creation failed!");

    XmlDocument doc;
    objectToSerialize->Serialize(&doc, nullptr);
    std::string xml;
    rapidxml::print(std::back_inserter(xml), doc, 0);

    file << xml;
    file.close();
    doc.clear();
  }

  const float g_desiredFps = 30.0f;
  const float g_animEps    = 0.001f;
  String g_currentExt;

  // Interpolator functions Begin
  // Range checks added by OTSoftware.
  // https://github.com/triplepointfive/ogldev/blob/master/tutorial39/mesh.cpp

  bool LessEqual(float a, float b, float eps)
  {
    float diff = a - b;
    return diff < -eps || diff == 0.0f;
  }

  bool IsEqual(float a, float b, float eps)
  {
    return abs(a - b) < eps;
  }

  bool IsZero(float a, float eps)
  {
    return abs(a) < eps;
  }

  int GetMax(int a, int b)
  {
    return a > b ? a : b;
  }

  int GetMax(int a, int b, int c)
  {
    return GetMax(a, GetMax(b, c));
  }

  uint FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
  {
    for (uint i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
    {
      if (LessEqual(AnimationTime,
                    static_cast<float>(pNodeAnim->mPositionKeys[i + 1].mTime),
                    g_animEps))
      {
        return i;
      }
    }

    assert(0);

    return 0;
  }

  uint FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
  {
    assert(pNodeAnim->mNumRotationKeys > 0);

    for (uint i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++)
    {
      if (LessEqual(AnimationTime,
                    static_cast<float>(pNodeAnim->mRotationKeys[i + 1].mTime),
                    g_animEps))
      {
        return i;
      }
    }

    assert(0);

    return 0;
  }

  uint FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
  {
    assert(pNodeAnim->mNumScalingKeys > 0);

    for (uint i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++)
    {
      if (LessEqual(AnimationTime,
                    static_cast<float>(pNodeAnim->mScalingKeys[i + 1].mTime),
                    g_animEps))
      {
        return i;
      }
    }

    assert(0);

    return 0;
  }

  void CalcInterpolatedPosition(aiVector3D& Out,
                                float AnimationTime,
                                const aiNodeAnim* pNodeAnim)
  {
    if (pNodeAnim->mNumPositionKeys == 1)
    {
      Out = pNodeAnim->mPositionKeys[0].mValue;
      return;
    }

    uint PositionIndex     = FindPosition(AnimationTime, pNodeAnim);
    uint NextPositionIndex = (PositionIndex + 1);
    assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
    float DeltaTime =
        static_cast<float>(pNodeAnim->mPositionKeys[NextPositionIndex].mTime -
                           pNodeAnim->mPositionKeys[PositionIndex].mTime);

    float Factor =
        (AnimationTime -
         static_cast<float>(pNodeAnim->mPositionKeys[PositionIndex].mTime)) /
        DeltaTime;

    if (IsZero(Factor, 0.001f))
    {
      Factor = 0.0f;
    }

    if (IsEqual(Factor, 1.0f, 0.001f))
    {
      Factor = 1.0f;
    }

    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
    const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
    aiVector3D Delta      = End - Start;
    Out                   = Start + Factor * Delta;
  }

  void CalcInterpolatedRotation(aiQuaternion& Out,
                                float AnimationTime,
                                const aiNodeAnim* pNodeAnim)
  {
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumRotationKeys == 1)
    {
      Out = pNodeAnim->mRotationKeys[0].mValue;
      return;
    }

    uint RotationIndex     = FindRotation(AnimationTime, pNodeAnim);
    uint NextRotationIndex = (RotationIndex + 1);
    assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);

    float DeltaTime =
        static_cast<float>(pNodeAnim->mRotationKeys[NextRotationIndex].mTime -
                           pNodeAnim->mRotationKeys[RotationIndex].mTime);

    float Factor =
        (AnimationTime -
         static_cast<float>(pNodeAnim->mRotationKeys[RotationIndex].mTime)) /
        DeltaTime;

    if (IsZero(Factor, g_animEps))
    {
      Factor = 0.0f;
    }

    if (IsEqual(Factor, 1.0f, g_animEps))
    {
      Factor = 1.0f;
    }

    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiQuaternion& StartRotationQ =
        pNodeAnim->mRotationKeys[RotationIndex].mValue;

    const aiQuaternion& EndRotationQ =
        pNodeAnim->mRotationKeys[NextRotationIndex].mValue;

    aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
    Out = Out.Normalize();
  }

  void CalcInterpolatedScaling(aiVector3D& Out,
                               float AnimationTime,
                               const aiNodeAnim* pNodeAnim)
  {
    if (pNodeAnim->mNumScalingKeys == 1)
    {
      Out = pNodeAnim->mScalingKeys[0].mValue;
      return;
    }

    uint ScalingIndex     = FindScaling(AnimationTime, pNodeAnim);
    uint NextScalingIndex = (ScalingIndex + 1);
    assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);

    float DeltaTime =
        static_cast<float>(pNodeAnim->mScalingKeys[NextScalingIndex].mTime -
                           pNodeAnim->mScalingKeys[ScalingIndex].mTime);

    float Factor =
        (AnimationTime -
         static_cast<float>(pNodeAnim->mScalingKeys[ScalingIndex].mTime)) /
        DeltaTime;

    if (IsZero(Factor, 0.001f))
    {
      Factor = 0.0f;
    }

    if (IsEqual(Factor, 1.0f, 0.001f))
    {
      Factor = 1.0f;
    }

    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
    const aiVector3D& End   = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
    aiVector3D Delta        = End - Start;
    Out                     = Start + Factor * Delta;
  }

  // Interpolator functions END

  void ImportAnimation(string file)
  {
    if (!g_scene->HasAnimations())
    {
      return;
    }

    for (uint i = 0; i < g_scene->mNumAnimations; i++)
    {
      aiAnimation* anim = g_scene->mAnimations[i];
      std::string animName(anim->mName.C_Str());
      string animFilePath = file;
      replace(animName.begin(), animName.end(), '.', '_');
      replace(animName.begin(), animName.end(), '|', '_');
      animFilePath += animName + ".anim";
      AddToUsedFiles(animFilePath);
      AnimationPtr tAnim = std::make_shared<Animation>();

      double fps =
          anim->mTicksPerSecond == 0 ? g_desiredFps : anim->mTicksPerSecond;

      double duration = anim->mDuration / fps;
      uint frameCount = (uint) ceil(duration * g_desiredFps);

      // Used to normalize animation start time.
      int cr, ct, cs, cmax;
      cr = ct = cs = cmax = 0;

      for (unsigned int chIndx = 0; chIndx < anim->mNumChannels; chIndx++)
      {
        KeyArray keys;
        aiNodeAnim* nodeAnim = anim->mChannels[chIndx];
        for (uint frame = 1; frame < frameCount; frame++)
        {
          float timeInTicks = (frame / g_desiredFps) *
                              static_cast<float>(anim->mTicksPerSecond);

          aiVector3D t;
          if (
              // Timer is not yet reach the animation begin. Skip frames.
              // Happens when there aren't keys at the beginning of the
              // animation.
              LessEqual(timeInTicks,
                        static_cast<float>(nodeAnim->mPositionKeys[0].mTime),
                        0.001f))
          {
            continue;
          }
          else
          {
            CalcInterpolatedPosition(t, timeInTicks, nodeAnim);
            ct++;
          }

          aiQuaternion r;
          if (LessEqual(timeInTicks,
                        static_cast<float>(nodeAnim->mRotationKeys[0].mTime),
                        0.001f))
          {
            continue;
          }
          else
          {
            CalcInterpolatedRotation(r, timeInTicks, nodeAnim);
            cr++;
          }

          aiVector3D s;
          if (LessEqual(timeInTicks,
                        static_cast<float>(nodeAnim->mScalingKeys[0].mTime),
                        0.001f))
          {
            continue;
          }
          else
          {
            CalcInterpolatedScaling(s, timeInTicks, nodeAnim);
            cs++;
          }

          Key tKey;
          tKey.m_frame    = frame;
          tKey.m_position = Vec3(t.x, t.y, t.z);
          tKey.m_rotation = Quaternion(r.w, r.x, r.y, r.z);
          tKey.m_scale    = Vec3(s.x, s.y, s.z);
          keys.push_back(tKey);
        }

        cmax = GetMax(cr, ct, cs);
        cr = ct = cs = 0;
        tAnim->m_keys.insert(std::make_pair(nodeAnim->mNodeName.C_Str(), keys));
      }

      // Recalculate duration. May be misleading dueto shifted animations.
      tAnim->m_duration = static_cast<float>(cmax / g_desiredFps);
      tAnim->m_fps      = static_cast<float>(g_desiredFps);

      CreateFileAndSerializeObject(tAnim.get(), animFilePath);
    }
  }

  void ImportMaterial(string filePath, string origin)
  {
    fs::path pathOrg = fs::path(origin).parent_path();

    auto textureFindAndCreateFunc =
        [filePath, pathOrg](aiTextureType textureAssimpType,
                            aiMaterial* material) -> TexturePtr {
      int texCount = material->GetTextureCount(textureAssimpType);
      TexturePtr tTexture;
      if (texCount > 0)
      {
        aiString texture;
        material->GetTexture(textureAssimpType, 0, &texture);

        string tName  = texture.C_Str();
        bool embedded = false;
        if (!tName.empty() && tName[0] == '*') // Embedded texture.
        {
          embedded           = true;
          string indxPart    = tName.substr(1);
          unsigned int tIndx = atoi(indxPart.c_str());
          if (g_scene->mNumTextures > tIndx)
          {
            aiTexture* t = g_scene->mTextures[tIndx];
            tName        = GetEmbeddedTextureName(t);
          }
        }

        string fileName = tName;
        TrunckToFileName(fileName);
        string textPath =
            fs::path(filePath + fileName).lexically_normal().u8string();

        if (!embedded && !std::filesystem::exists(textPath))
        {
          // Try copying texture.
          fs::path fullPath = pathOrg;
          fullPath.append(tName);
          fullPath = fullPath.lexically_normal();

          ifstream isGoodFile;
          isGoodFile.open(fullPath, ios::binary | ios::in);
          if (isGoodFile.good())
          {
            fs::path target = fs::path(textPath);
            if (target.has_parent_path())
            {
              fs::path dir = target.parent_path();
              if (!fs::exists(dir))
              {
                fs::create_directories(dir);
              }
            }

            fs::copy(fullPath, target, fs::copy_options::overwrite_existing);
          }
          isGoodFile.close();
        }

        AddToUsedFiles(textPath);
        tTexture = std::make_shared<Texture>();
        tTexture->SetFile(textPath);
      }
      return tTexture;
    };

    for (unsigned int i = 0; i < g_scene->mNumMaterials; i++)
    {
      aiMaterial* material  = g_scene->mMaterials[i];
      string name           = GetMaterialName(material, i);
      string writePath      = filePath + name + MATERIAL;
      MaterialPtr tMaterial = std::make_shared<Material>();

      auto diffuse = textureFindAndCreateFunc(aiTextureType_DIFFUSE, material);
      if (diffuse)
      {
        tMaterial->m_diffuseTexture = diffuse;
      }

      tMaterial->SetFile(writePath);
      CreateFileAndSerializeObject(tMaterial.get(), writePath);
      AddToUsedFiles(writePath);
      tMaterials.push_back(tMaterial);
    }
  }

  // Creates a ToolKit mesh by reading the aiMesh
  // @param mainMesh: Pointer of the mesh
  template <typename convertType>
  void ConvertMesh(aiMesh* mesh, convertType tMesh)
  {
    assert(mesh->mNumVertices && "Mesh has no vertices!");

    // Skin data
    unordered_map<int, vector<std::pair<int, float>>> skinData;
    if constexpr (std::is_same<convertType, SkinMeshPtr>::value)
    {
      for (unsigned int i = 0; i < mesh->mNumBones; i++)
      {
        aiBone* bone = mesh->mBones[i];
        assert(g_skeletonMap.find(bone->mName.C_Str()) != g_skeletonMap.end());
        BoneNode bn = g_skeletonMap[bone->mName.C_Str()];
        for (unsigned int j = 0; j < bone->mNumWeights; j++)
        {
          aiVertexWeight vw = bone->mWeights[j];
          skinData[vw.mVertexId].push_back(
              std::pair<int, float>(bn.boneIndex, vw.mWeight));
        }
      }
      tMesh->m_skeleton = g_skeleton;
    }

    tMesh->m_clientSideVertices.resize(mesh->mNumVertices);
    for (unsigned int vIndex = 0; vIndex < mesh->mNumVertices; vIndex++)
    {
      auto& v = tMesh->m_clientSideVertices[vIndex];
      v.pos   = Vec3(mesh->mVertices[vIndex].x,
                   mesh->mVertices[vIndex].y,
                   mesh->mVertices[vIndex].z);

      if (mesh->HasNormals())
      {
        v.norm = Vec3(mesh->mNormals[vIndex].x,
                      mesh->mNormals[vIndex].y,
                      mesh->mNormals[vIndex].z);
      }

      // Does the mesh contain texture coordinates?
      if (mesh->HasTextureCoords(0))
      {
        v.tex.x = mesh->mTextureCoords[0][vIndex].x;
        v.tex.y = mesh->mTextureCoords[0][vIndex].y;
      }

      if (mesh->HasTangentsAndBitangents())
      {
        v.btan = Vec3(mesh->mBitangents[vIndex].x,
                      mesh->mBitangents[vIndex].y,
                      mesh->mBitangents[vIndex].z);
      }

      if constexpr (std::is_same<convertType, SkinMeshPtr>::value)
      {
        if (!skinData.empty() && skinData.find(vIndex) != skinData.end())
        {
          for (int j = 0; j < 4; j++)
          {
            if (j >= static_cast<int>(skinData[vIndex].size()))
            {
              skinData[vIndex].push_back(std::pair<int, float>(0, 0.0f));
            }
          }

          for (ubyte i = 0; i < 4; i++)
          {
            v.bones[i]   = (float) skinData[vIndex][i].first;
            v.weights[i] = skinData[vIndex][i].second;
          }
        }
      }
    }

    tMesh->m_clientSideIndices.resize(mesh->mNumFaces * 3);
    for (unsigned int face_i = 0; face_i < mesh->mNumFaces; face_i++)
    {
      aiFace face = mesh->mFaces[face_i];
      assert(face.mNumIndices == 3);
      for (ubyte i = 0; i < 3; i++)
      {
        tMesh->m_clientSideIndices[(face_i * 3) + i] = face.mIndices[i];
      }
    }

    tMesh->m_loaded      = true;
    tMesh->m_vertexCount = static_cast<int>(tMesh->m_clientSideVertices.size());
    tMesh->m_indexCount  = static_cast<int>(tMesh->m_clientSideIndices.size());
    tMesh->m_material    = tMaterials[mesh->mMaterialIndex];
    for (ubyte i = 0; i < 3; i++)
    {
      tMesh->m_aabb.min[i] = mesh->mAABB.mMin[i];
      tMesh->m_aabb.max[i] = mesh->mAABB.mMax[i];
    }
  }

  std::unordered_map<aiMesh*, MeshPtr> g_meshes;
  SkinMeshPtr mainSkinMesh;

  void ImportMeshes(string filePath)
  {
    string path, name;
    Decompose(filePath, path, name);
    mainSkinMesh = nullptr;

    // Skinned meshes will be merged because they're using the same skeleton
    // (Only one skeleton is imported)
    for (uint MeshIndx = 0; MeshIndx < g_scene->mNumMeshes; MeshIndx++)
    {
      aiMesh* aMesh = g_scene->mMeshes[MeshIndx];
      if (aMesh->HasBones())
      {
        SkinMeshPtr skinMesh = std::make_shared<SkinMesh>();
        ConvertMesh(aMesh, skinMesh);
        if (mainSkinMesh)
        {
          mainSkinMesh->m_subMeshes.push_back(skinMesh);
        }
        else
        {
          mainSkinMesh = skinMesh;
        }
      }
      else
      {
        MeshPtr mesh = std::make_shared<Mesh>();
        ConvertMesh(aMesh, mesh);

        // Better to use scene node name
        string fileName  = "";
        aiNode* meshNode = g_scene->mRootNode->FindNode(aMesh->mName);
        if (meshNode)
        {
          fileName = std::string(meshNode->mName.C_Str());
        }
        else
        {
          fileName = aMesh->mName.C_Str();
        }
        ClearForbidden(fileName);
        String meshPath = path + fileName + MESH;

        mesh->SetFile(meshPath);
        AddToUsedFiles(meshPath);
        g_meshes[aMesh] = mesh;
        CreateFileAndSerializeObject(mesh.get(), meshPath);
      }
    }
    if (mainSkinMesh)
    {
      ClearForbidden(name);
      String skinMeshPath = path + name + SKINMESH;
      mainSkinMesh->SetFile(skinMeshPath);

      AddToUsedFiles(skinMeshPath);
      CreateFileAndSerializeObject(mainSkinMesh.get(), skinMeshPath);
    }
  }

  EntityRawPtrArray deletedEntities;
  bool deleteEmptyEntitiesRecursively(Scene* tScene, Entity* ntt)
  {
    bool shouldDelete = true;
    if (ntt->GetComponentPtrArray().size())
    {
      shouldDelete = false;
    }
    VariantCategoryArray varCategories;
    ntt->m_localData.GetCategories(varCategories, true, false);
    if (varCategories.size() > 1)
    {
      shouldDelete = false;
    }

    for (Node* child : ntt->m_node->m_children)
    {
      if (!deleteEmptyEntitiesRecursively(tScene, child->m_entity))
      {
        shouldDelete = false;
      }
    }
    if (shouldDelete)
    {
      tScene->RemoveEntity(ntt->GetIdVal());
      deletedEntities.push_back(ntt);
    }
    return shouldDelete;
  }

  void TraverseScene(Scene* tScene, const aiNode* node, Entity* parent)
  {
    Entity* ntt                 = new Entity;
    ntt->m_node->m_inheritScale = true;
    Vec3 t, s;
    Quaternion rt;
    DecomposeAssimpMatrix(node->mTransformation, &t, &rt, &s);

    if (parent)
    {
      parent->m_node->AddChild(ntt->m_node);
    }
    for (uint meshIndx = 0; meshIndx < node->mNumMeshes; meshIndx++)
    {
      aiMesh* aMesh = g_scene->mMeshes[node->mMeshes[meshIndx]];
      if (aMesh->HasBones() && isSkeletonEntityCreated)
      {
        continue;
      }
      MeshComponentPtr meshComp = std::make_shared<MeshComponent>();
      ntt->AddComponent(meshComp);
      if (aMesh->HasBones())
      {
        meshComp->SetMeshVal(mainSkinMesh);
        SkeletonComponentPtr skelComp = std::make_shared<SkeletonComponent>();
        skelComp->SetSkeletonResourceVal(g_skeleton);
        ntt->AddComponent(skelComp);
        isSkeletonEntityCreated = true;
      }
      else
      {
        meshComp->SetMeshVal(g_meshes[aMesh]);
      }
      MultiMaterialPtr matComp = std::make_shared<MultiMaterialComponent>();
      ntt->AddComponent(matComp);
      matComp->UpdateMaterialList(meshComp);
    }

    for (uint childIndx = 0; childIndx < node->mNumChildren; childIndx++)
    {
      TraverseScene(tScene, node->mChildren[childIndx], ntt);
    }

    ntt->m_node->Translate(t);
    ntt->m_node->Rotate(rt);
    ntt->m_node->Scale(s);
    tScene->AddEntity(ntt);
  }

  void ImportScene(string filePath)
  {
    // Print Scene.
    string path, name;
    Decompose(filePath, path, name);

    string fullPath = path + name + SCENE;
    AddToUsedFiles(fullPath);
    Scene* tScene = new Scene;

    TraverseScene(tScene, g_scene->mRootNode, nullptr);
    // First entity is the root entity
    EntityRawPtrArray roots;
    GetRootEntities(tScene->GetEntities(), roots);
    for (Entity* r : roots)
    {
      deleteEmptyEntitiesRecursively(tScene, r);
    }
    for (Entity* ntt : deletedEntities)
    {
      SafeDel(ntt);
    }
    deletedEntities.clear();

    CreateFileAndSerializeObject(tScene, fullPath);
  }

  void ImportSkeleton(string filePath)
  {
    auto addBoneNodeFn = [](aiNode* node, aiBone* bone) -> void {
      BoneNode bn(node, 0);
      if (node->mName == bone->mName)
      {
        bn.bone = bone;
      }
      g_skeletonMap[node->mName.C_Str()] = bn;
    };

    // Collect skeleton parts
    vector<aiBone*> bones;
    for (unsigned int i = 0; i < g_scene->mNumMeshes; i++)
    {
      aiMesh* mesh     = g_scene->mMeshes[i];
      aiNode* meshNode = g_scene->mRootNode->FindNode(mesh->mName);
      for (unsigned int j = 0; j < mesh->mNumBones; j++)
      {
        aiBone* bone = mesh->mBones[j];
        bones.push_back(bone);
        aiNode* node = g_scene->mRootNode->FindNode(bone->mName);
        while (node) // Go Up
        {
          if (node == meshNode)
          {
            break;
          }

          if (meshNode != nullptr)
          {
            if (node == meshNode->mParent)
            {
              break;
            }
          }

          addBoneNodeFn(node, bone);
          node = node->mParent;
        }

        node = g_scene->mRootNode->FindNode(bone->mName);

        // Go Down
        std::function<void(aiNode*)> checkDownFn =
            [&checkDownFn, &bone, &addBoneNodeFn](aiNode* node) -> void {
          if (node == nullptr)
          {
            return;
          }

          addBoneNodeFn(node, bone);

          for (unsigned int i = 0; i < node->mNumChildren; i++)
          {
            checkDownFn(node->mChildren[i]);
          }
        };
        checkDownFn(node);
      }
    }

    for (auto& bone : bones)
    {
      if (g_skeletonMap.find(bone->mName.C_Str()) != g_skeletonMap.end())
      {
        g_skeletonMap[bone->mName.C_Str()].bone = bone;
      }
    }

    if (bones.size() == 0)
    {
      return;
    }

    // Assign indices
    std::function<void(aiNode*, unsigned int&)> assignBoneIndexFn =
        [&assignBoneIndexFn](aiNode* node, unsigned int& index) -> void {
      if (g_skeletonMap.find(node->mName.C_Str()) != g_skeletonMap.end())
      {
        g_skeletonMap[node->mName.C_Str()].boneIndex = index++;
      }

      for (unsigned int i = 0; i < node->mNumChildren; i++)
      {
        assignBoneIndexFn(node->mChildren[i], index);
      }
    };

    unsigned int boneIndex = 0;
    assignBoneIndexFn(g_scene->mRootNode, boneIndex);

    string name, path;
    Decompose(filePath, path, name);
    string fullPath = path + name + SKELETON;

    g_skeleton = std::make_shared<Skeleton>();
    g_skeleton->SetFile(fullPath);

    // Print
    std::function<void(aiNode * node, DynamicBoneMap::DynamicBone*)>
        setBoneHierarchyFn =
            [&setBoneHierarchyFn](
                aiNode* node, DynamicBoneMap::DynamicBone* parentBone) -> void {
      DynamicBoneMap::DynamicBone* searchDBone = parentBone;
      if (g_skeletonMap.find(node->mName.C_Str()) != g_skeletonMap.end())
      {
        assert(node->mName.length);
        g_skeleton->m_Tpose.boneList.insert(std::make_pair(
            String(node->mName.C_Str()), DynamicBoneMap::DynamicBone()));
        searchDBone =
            &g_skeleton->m_Tpose.boneList.find(node->mName.C_Str())->second;
        searchDBone->node                 = new Node();
        searchDBone->node->m_inheritScale = true;
        searchDBone->boneIndx             = uint(g_skeleton->m_bones.size());
        g_skeleton->m_Tpose.AddDynamicBone(
            node->mName.C_Str(), *searchDBone, parentBone);

        StaticBone* sBone = new StaticBone(node->mName.C_Str());
        g_skeleton->m_bones.push_back(sBone);
      }
      for (unsigned int i = 0; i < node->mNumChildren; i++)
      {
        setBoneHierarchyFn(node->mChildren[i], searchDBone);
      }
    };

    std::function<void(aiNode * node)> setTransformationsFn =
        [&setTransformationsFn](aiNode* node) -> void {
      if (g_skeletonMap.find(node->mName.C_Str()) != g_skeletonMap.end())
      {
        StaticBone* sBone = g_skeleton->GetBone(node->mName.C_Str());

        // Set bone node transformation
        {
          DynamicBoneMap::DynamicBone& dBone =
              g_skeleton->m_Tpose.boneList[node->mName.C_Str()];
          Vec3 t, s;
          Quaternion r;
          DecomposeAssimpMatrix(node->mTransformation, &t, &r, &s);

          dBone.node->SetTranslation(t);
          dBone.node->SetOrientation(r);
          dBone.node->SetScale(s);
        }

        // Set bind pose transformation
        {
          aiBone* bone = g_skeletonMap[node->mName.C_Str()].bone;

          if (bone)
          {
            Vec3 t, s;
            Quaternion r;
            DecomposeAssimpMatrix(bone->mOffsetMatrix, &t, &r, &s);

            Mat4 tMat, rMat, sMat;
            tMat                        = glm::translate(tMat, t);
            rMat                        = glm::toMat4(r);
            sMat                        = glm::scale(sMat, s);
            sBone->m_inverseWorldMatrix = tMat * rMat * sMat;
          }
        }
      }

      for (unsigned int i = 0; i < node->mNumChildren; i++)
      {
        setTransformationsFn(node->mChildren[i]);
      }
    };

    setBoneHierarchyFn(g_scene->mRootNode, nullptr);
    setTransformationsFn(g_scene->mRootNode);

    CreateFileAndSerializeObject(g_skeleton.get(), fullPath);
    AddToUsedFiles(fullPath);
  }

  void ImportTextures(string filePath)
  {
    // Embedded textures.
    if (g_scene->HasTextures())
    {
      for (unsigned int i = 0; i < g_scene->mNumTextures; i++)
      {
        TexturePtr tTexture = std::make_shared<Texture>();
        aiTexture* texture  = g_scene->mTextures[i];
        string embId        = GetEmbeddedTextureName(texture);

        // Compressed.
        if (texture->mHeight == 0)
        {
          ofstream file(filePath + embId, fstream::out | std::fstream::binary);
          assert(file.good());

          file.write((const char*) g_scene->mTextures[i]->pcData,
                     g_scene->mTextures[i]->mWidth);
        }
        else
        {
          unsigned char* buffer = (unsigned char*) texture->pcData;
          stbi_write_png(filePath.c_str(),
                         texture->mWidth,
                         texture->mHeight,
                         4,
                         buffer,
                         texture->mWidth * 4);
        }
      }
    }
  }

  int ToolKitMain(int argc, char* argv[])
  {
    try
    {
      if (argc < 2)
      {
        cout << "usage: Import 'fileToImport.format' <op> -t 'importTo' <op> "
                "-s 1.0";
        throw(-1);
      }

      Assimp::Importer importer;
      importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE,
                                  aiPrimitiveType_LINE | aiPrimitiveType_POINT);

      string dest, file = argv[1];
      Assimp::DefaultLogger::create("Assimplog.txt", Assimp::Logger::VERBOSE);
      for (int i = 0; i < argc; i++)
      {
        string arg = argv[i];
        Assimp::DefaultLogger::get()->info(arg);

        if (arg == "-t")
        {
          dest = fs::path(argv[i + 1]).append("").u8string();
        }

        if (arg == "-s")
        {
          float scale = static_cast<float>(std::atof(argv[i + 1]));
          importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, scale);
        }
      }

      dest = fs::path(dest).lexically_normal().u8string();
      if (!dest.empty())
      {
        fs::create_directories(dest);
      }

      string ext = file.substr(file.find_last_of("."));
      std::vector<string> files;
      if (ext == ".txt")
      {
        fstream fList;
        fList.open(file, ios::in);
        if (fList.is_open())
        {
          string fileStr;
          while (getline(fList, fileStr))
          {
            files.push_back(fileStr);
          }
          fList.close();
        }
      }
      else
      {
        files.push_back(file);
      }

      // Initialize ToolKit to serialize resources
      Main* g_proxy = new Main();
      Main::SetProxy(g_proxy);
      g_proxy->m_entityFactory = new EntityFactory();

      for (int i = 0; i < static_cast<int>(files.size()); i++)
      {
        file = files[i];
        // Clear global materials for each scene to prevent wrong referencing
        tMaterials.clear();

        const aiScene* scene = importer.ReadFile(
            file,
            aiProcess_Triangulate | aiProcess_SortByPType |
                aiProcess_FindDegenerates | aiProcess_CalcTangentSpace |
                aiProcess_FlipUVs | aiProcess_LimitBoneWeights |
                aiProcess_GenSmoothNormals | aiProcess_GlobalScale |
                aiProcess_FindInvalidData | aiProcess_GenBoundingBoxes |
                aiProcessPreset_TargetRealtime_MaxQuality |
                aiProcess_PopulateArmatureData);

        if (scene == nullptr)
        {
          assert(
              0 &&
              "Assimp failed to import the file. Probably file is corrupted!");
          throw(-1);
        }
        g_scene                 = scene;
        isSkeletonEntityCreated = false;

        String fileName;
        DecomposePath(file, nullptr, &fileName, &g_currentExt);
        string destFile = dest + fileName;

        // DON'T BREAK THE CALLING ORDER!

        ImportAnimation(dest);
        // Create Textures to reference in Materials
        ImportTextures(dest);
        // Create Materials to reference in Meshes
        ImportMaterial(dest, file);
        // Create a Skeleton to reference in Meshes
        ImportSkeleton(destFile);

        // Add Meshes.
        ImportMeshes(destFile);

        // Create Meshes & Scene
        ImportScene(destFile);
      }

      // Report all in use files.
      fstream inUse("out.txt", ios::out);
      for (string fs : g_usedFiles)
      {
        inUse << fs << endl;
      }
      inUse.close();

      delete g_proxy;
    }
    catch (int code)
    {
      Assimp::DefaultLogger::get()->error("Import failed");
      Assimp::DefaultLogger::kill();
      return code;
    }

    Assimp::DefaultLogger::get()->info("Import success");
    Assimp::DefaultLogger::kill();

    return 0;
  }
} // namespace ToolKit

int main(int argc, char* argv[])
{
  return ToolKit::ToolKitMain(argc, argv);
}
