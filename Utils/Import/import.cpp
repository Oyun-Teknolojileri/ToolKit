#include <iostream>
#include <fstream>
#include <vector>
#include <functional>
#include <unordered_map>
#include <utility>
#include <algorithm>
#include <string>
#include <assert.h>
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/DefaultLogger.hpp"
#include "lodepng.h"
#include <filesystem>

using namespace std;

struct Pnt3D
{
  float x = 0;
  float y = 0;
  float z = 0;
};

struct Pnt2D
{
  float x = 0;
  float y = 0;
};

class BoneNode
{
public:
  BoneNode() {}

  BoneNode(aiNode* node, unsigned int index)
  {
    boneIndex = index;
    boneNode = node;
  }

  aiNode* boneNode = nullptr;
  aiBone* bone = nullptr;
  unsigned int boneIndex = 0;
};

vector<string> g_usedFiles;
bool IsUsed(string file)
{
  return find(g_usedFiles.begin(), g_usedFiles.end(), file) == g_usedFiles.end();
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
  std::replace_if(str.begin(), str.end(), [&forbiddenChars](char c) { return std::string::npos != forbiddenChars.find(c); }, ' ');
}

unordered_map<string, BoneNode> g_skeletonMap;
const aiScene* g_scene = nullptr;
static unsigned int g_lastId = 1;

void TrunckToFileName(string& fullPath)
{
  std::replace(fullPath.begin(), fullPath.end(), '/', '\\');

  size_t i = fullPath.find_last_of('\\');
  if (i != string::npos)
  {
    fullPath = fullPath.substr(i + 1);
  }
}

void Decompose(string fullPath, string& path, string& name)
{
  std::replace(fullPath.begin(), fullPath.end(), '/', '\\');
  path = "";

  size_t i = fullPath.find_last_of('\\');
  if (i != string::npos)
  {
    path = fullPath.substr(0, fullPath.find_last_of('\\') + 1);
  }

  name = fullPath.substr(fullPath.find_last_of('\\') + 1);
  name = name.substr(0, name.find_last_of('.'));
}

string GetTextureName(const aiTexture* texture, unsigned int i)
{
  string name = texture->mFilename.C_Str();
  std::replace(name.begin(), name.end(), '/', '\\');

  if (name.empty() || name[0] == '*')
  {
    name = "emb" + to_string(i) + "." + texture->achFormatHint;
  }
  else
  {
    TrunckToFileName(name);
  }

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
  return GetMaterialName(g_scene->mMaterials[mesh->mMaterialIndex], mesh->mMaterialIndex);
}

void PrintAnims_(const aiScene* scene, string file)
{
  if (!scene->HasAnimations())
    return;

  for (unsigned int i = 0; i < scene->mNumAnimations; i++)
  {
    aiAnimation* anim = scene->mAnimations[i];
    string path = file;
    std::string animName(anim->mName.C_Str());
    replace(animName.begin(), animName.end(), '.', '_');
    replace(animName.begin(), animName.end(), '|', '_');
    path += animName + ".anim";

    AddToUsedFiles(path);

    ofstream oFile;
    oFile.open(path, ios::out);
    assert(oFile.good());

    double fps = anim->mTicksPerSecond == 0 ? 24.0 : anim->mTicksPerSecond;
    oFile << "<anim fps=\"" + to_string(fps) + "\" duration=\"" + to_string(anim->mDuration / fps) + "\">\n";

    for (unsigned int j = 0; j < anim->mNumChannels; j++)
    {
      aiNodeAnim* nodeAnim = anim->mChannels[j];
      oFile << "  <node name=\"" + string(nodeAnim->mNodeName.C_Str()) + "\">\n";
      for (unsigned int k = 0; k < nodeAnim->mNumPositionKeys; k++)
      {
        oFile << "    <key frame=\"" + to_string((int)(nodeAnim->mPositionKeys[k].mTime)) + "\">\n";
        aiVector3D t = nodeAnim->mPositionKeys[k].mValue;
        oFile << "      <translation x=\"" + to_string(t.x) + "\" y=\"" + to_string(t.y) + "\" z=\"" + to_string(t.z) + "\"/>\n";
        aiQuaternion r = nodeAnim->mRotationKeys[k].mValue;
        oFile << "      <rotation w=\"" + to_string(r.w) + "\" x=\"" + to_string(r.x) + "\" y=\"" + to_string(r.y) + "\" z=\"" + to_string(r.z) + "\"/>\n";
        aiVector3D s = nodeAnim->mScalingKeys[k].mValue;
        oFile << "      <scale x=\"" + to_string(s.x) + "\" y=\"" + to_string(s.y) + "\" z=\"" + to_string(s.z) + "\"/>\n";
        oFile << "    </key>\n";
      }
      oFile << "  </node>\n";
    }

    oFile << "</anim>\n";
    oFile.close();
  }
}

void PrintMaterial_(const aiScene* scene, string filePath, string origin)
{
  filesystem::path pathOrg = filesystem::path(origin).parent_path();

  for (unsigned int i = 0; i < scene->mNumMaterials; i++)
  {
    aiMaterial* material = scene->mMaterials[i];
    string name = GetMaterialName(material, i);
    string writePath = filePath + name + ".material";
    if (IsUsed(writePath))
    {
      // Unused material.
      continue;
    }

    ofstream file(writePath, ios::out);
    assert(file.good());

    file << "<material>\n";
    int diffTexCnt = material->GetTextureCount(aiTextureType_DIFFUSE);
    if (diffTexCnt > 0)
    {
      aiString texture;
      material->GetTexture(aiTextureType_DIFFUSE, 0, &texture);

      string tName = texture.C_Str();
      filesystem::path nextPath = pathOrg;
      tName = filesystem::canonical(nextPath.append(tName)).u8string();
      string outName = filesystem::path(tName).filename().u8string();
      string textPath = filePath + outName;
      if (!tName.empty() && tName[0] == '*') // Embedded texture.
      {
        string indxPart = tName.substr(1);
        unsigned int tIndx = atoi(indxPart.c_str());
        if (scene->mNumTextures > tIndx)
        {
          aiTexture* t = scene->mTextures[tIndx];
          tName = GetTextureName(t, tIndx);
        }
      }
      else
      {
        // Try copying texture.
        ifstream isGoodFile;
        isGoodFile.open(tName, ios::binary | ios::in);
        if (isGoodFile.good())
        {
          filesystem::copy(tName, textPath, std::filesystem::copy_options::overwrite_existing);
        }
        isGoodFile.close();
      }

      AddToUsedFiles(textPath);

      file << "  <diffuseTexture name = \"" + textPath + "\"/>\n";
    }
    file << "</material>\n";
    file.close();
  }
}

void AppendMesh_(aiMesh* mesh, ofstream& file, string filePath)
{
  // Skin data
  unordered_map<int, vector<pair<int, float> > > skinData;
  for (unsigned int i = 0; i < mesh->mNumBones; i++)
  {
    aiBone* bone = mesh->mBones[i];
    assert(g_skeletonMap.find(bone->mName.C_Str()) != g_skeletonMap.end());
    BoneNode bn = g_skeletonMap[bone->mName.C_Str()];
    for (unsigned int j = 0; j < bone->mNumWeights; j++)
    {
      aiVertexWeight vw = bone->mWeights[j];
      skinData[vw.mVertexId].push_back(pair<int, float>(bn.boneIndex, vw.mWeight));
    }
  }

  // Mesh
  assert(file.good());
  string tag = "skinMesh";
  if (skinData.empty())
  {
    tag = "mesh";
  }

  string path, name;
  Decompose(filePath, path, name);

  string fullMaterialPath = path + GetMaterialName(mesh) + ".material";
  AddToUsedFiles(fullMaterialPath);

  file << "  <" + tag + ">\n";
  file << "    <material name=\"" + fullMaterialPath + "\"/>\n";
  file << "    <vertices>\n";
  for (unsigned int i = 0; i < mesh->mNumVertices; i++)
  {
    file << "      <v>\n";

    file << "        <p x=\"" + to_string(mesh->mVertices[i].x) + "\" y=\"" + to_string(mesh->mVertices[i].y) + "\" z=\"" + to_string(mesh->mVertices[i].z) + "\"/>\n";
    file << "        <n x=\"" + to_string(mesh->mNormals[i].x) + "\" y=\"" + to_string(mesh->mNormals[i].y) + "\" z=\"" + to_string(mesh->mNormals[i].z) + "\"/>\n";

    Pnt2D vec;
    if (mesh->HasTextureCoords(0)) // Does the mesh contain texture coordinates?
    {
      vec.x = mesh->mTextureCoords[0][i].x;
      vec.y = mesh->mTextureCoords[0][i].y;
    }

    file << "        <t x=\"" + to_string(vec.x) + "\" y=\"" + to_string(vec.y) + "\"/>\n";

    Pnt3D tangent;
    if (mesh->HasTangentsAndBitangents())
    {
      tangent.x = mesh->mBitangents[i].x;
      tangent.y = mesh->mBitangents[i].y;
      tangent.z = mesh->mBitangents[i].z;
    }

    file << "        <bt x=\"" + to_string(tangent.x) + "\" y=\"" + to_string(tangent.y) + "\" z=\"" + to_string(tangent.z) + "\"/>\n";

    if (!skinData.empty())
    {
      if (skinData.find(i) != skinData.end())
      {
        for (int j = 0; j < 4; j++)
        {
          if (j >= (int)skinData[i].size())
          {
            skinData[i].push_back(pair<int, float>(0, 0.0f));
          }
        }
        file << "        <b w=\"" + to_string(skinData[i][0].first) + "\" x=\"" + to_string(skinData[i][1].first) + "\" y=\"" + to_string(skinData[i][2].first) + "\" z=\"" + to_string(skinData[i][3].first) + "\"/>\n";
        file << "        <w w=\"" + to_string(skinData[i][0].second) + "\" x=\"" + to_string(skinData[i][1].second) + "\" y=\"" + to_string(skinData[i][2].second) + "\" z=\"" + to_string(skinData[i][3].second) + "\"/>\n";
      }
    }

    file << "      </v>\n";
  }
  file << "    </vertices>\n";

  file << "    <faces>\n";
  for (unsigned int i = 0; i < mesh->mNumFaces; i++)
  {
    aiFace face = mesh->mFaces[i];
    assert(face.mNumIndices == 3);
    file << "      <f x=\"" + to_string(face.mIndices[0]) + "\" y=\"" + to_string(face.mIndices[1]) + "\" z=\"" + to_string(face.mIndices[2]) + "\"/>\n";
  }
  file << "    </faces>\n";

  file << "  </" + tag + ">\n";
}

void SearchMesh(ofstream& sceneFile, const aiScene* scene, string filePath, aiNode* node, int parentId)
{
  int thisId = g_lastId++;

  // Write meshes.
  string meshPath;
  if (node->mNumMeshes > 0)
  {
    string path, name;
    Decompose(filePath, path, name);

    string tag = "mesh";
    if (g_skeletonMap.find(node->mName.C_Str()) != g_skeletonMap.end())
    {
      tag = "skinMesh";
    }

    string fileName = std::string(node->mName.C_Str());
    ClearForbidden(fileName);
    meshPath = path + name + "_" + fileName + "." + tag;
    AddToUsedFiles(meshPath);

    ofstream file(meshPath, ios::out);
    file << "<meshContainer>\n";

    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
      aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
      AppendMesh_(mesh, file, filePath);
    }

    file << "</meshContainer>\n";
    file.close();
  }

  // Write Entity.
  if (parentId != -1)
  {
    sceneFile << "  <E i=\"" + std::to_string(thisId) + "\" pi=\"" + std::to_string(parentId) + "\" t=\"9\">\n";
  }
  else
  {
    sceneFile << "  <E i=\"" + std::to_string(thisId) + "\" t=\"9\">\n";
  }

  sceneFile << "    <N>\n";

  aiQuaternion rt;
  aiVector3D ts, scl;
  node->mTransformation.Decompose(scl, rt, ts);

  sceneFile << "      <T x=\"" + std::to_string(ts.x) + "\" y=\"" + std::to_string(ts.y) + "\" z=\"" + std::to_string(ts.z) + "\" />\n";
  sceneFile << "      <R x=\"" + std::to_string(rt.x) + "\" y=\"" + std::to_string(rt.y) + "\" z=\"" + std::to_string(rt.z) + "\" w=\"" + std::to_string(rt.w) + "\" />\n";
  sceneFile << "      <S x=\"" + std::to_string(scl.x) + "\" y=\"" + std::to_string(scl.y) + "\" z=\"" + std::to_string(scl.z) + "\" />\n";

  sceneFile << "    </N>\n";

  if (!meshPath.empty())
  {
    sceneFile << "    <M f=\"" + meshPath + "\"/>\n";
  }

  sceneFile << "  </E>\n";

  for (unsigned int j = 0; j < node->mNumChildren; j++)
  {
    SearchMesh(sceneFile, scene, filePath, node->mChildren[j], thisId);
  }
}

void PrintMesh_(const aiScene* scene, string filePath)
{
  // Print Scene.
  string path, name;
  Decompose(filePath, path, name);

  string fullPath = path + name + ".scene";
  AddToUsedFiles(fullPath);

  ofstream file(fullPath, ios::out);
  file << "<S>\n";

  // Add Meshes.
  SearchMesh(file, scene, filePath, scene->mRootNode, -1);

  file << "</S>";
  file.close();
}

void PrintSkeleton_(const aiScene* scene, string filePath)
{
  auto addBoneNodeFn = [](aiNode* node, aiBone* bone) -> void
  {
    BoneNode bn(node, 0);
    if (node->mName == bone->mName)
    {
      bn.bone = bone;
    }
    g_skeletonMap[node->mName.C_Str()] = bn;
  };

  // Collect skeleton parts
  vector<aiBone*> bones;
  for (unsigned int i = 0; i < scene->mNumMeshes; i++)
  {
    aiMesh* mesh = scene->mMeshes[i];
    aiNode* meshNode = scene->mRootNode->FindNode(mesh->mName);
    for (unsigned int j = 0; j < mesh->mNumBones; j++)
    {
      aiBone* bone = mesh->mBones[j];
      bones.push_back(bone);
      aiNode* node = scene->mRootNode->FindNode(bone->mName);
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

      node = scene->mRootNode->FindNode(bone->mName);
      function<void(aiNode*)> checkDownFn = [&checkDownFn, &bone, &addBoneNodeFn](aiNode* node) -> void // Go Down
      {
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

  // Assign indices
  function<void(aiNode*, unsigned int&)> assignBoneIndexFn = [&assignBoneIndexFn](aiNode* node, unsigned int& index) -> void
  {
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
  assignBoneIndexFn(scene->mRootNode, boneIndex);

  // Print
  bool anythingPrinted = false;
  function<void(aiNode*, ofstream&, string)> writeFn = [&writeFn, &anythingPrinted](aiNode* node, ofstream& file, string tabSpace) -> void
  {
    bool bonePrinted = false;
    if (g_skeletonMap.find(node->mName.C_Str()) != g_skeletonMap.end())
    {
      assert(node->mName.length);
      file << tabSpace << "<bone name = \"" + string(node->mName.C_Str()) + "\">\n";
      aiVector3D t, s;
      aiQuaternion r;
      node->mTransformation.Decompose(s, r, t);
      file << tabSpace << "  <translation x=\"" + to_string(t.x) + "\" y=\"" + to_string(t.y) + "\" z=\"" + to_string(t.z) + "\"/>\n";
      file << tabSpace << "  <rotation w=\"" + to_string(r.w) + "\" x=\"" + to_string(r.x) + "\" y=\"" + to_string(r.y) + "\" z=\"" + to_string(r.z) + "\"/>\n";
      file << tabSpace << "  <scale x=\"" + to_string(s.x) + "\" y=\"" + to_string(s.y) + "\" z=\"" + to_string(s.z) + "\"/>\n";

      aiBone* bone = g_skeletonMap[node->mName.C_Str()].bone;
      if (bone != nullptr)
      {
        file << tabSpace << "  <bindPose>\n";
        bone->mOffsetMatrix.Decompose(s, r, t);

        file << tabSpace << "    <translation x=\"" + to_string(t.x) + "\" y=\"" + to_string(t.y) + "\" z=\"" + to_string(t.z) + "\"/>\n";
        file << tabSpace << "    <rotation w=\"" + to_string(r.w) + "\" x=\"" + to_string(r.x) + "\" y=\"" + to_string(r.y) + "\" z=\"" + to_string(r.z) + "\"/>\n";
        file << tabSpace << "    <scale x=\"" + to_string(s.x) + "\" y=\"" + to_string(s.y) + "\" z=\"" + to_string(s.z) + "\"/>\n";

        file << tabSpace << "  </bindPose>\n";
      }

      bonePrinted = true;
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      writeFn(node->mChildren[i], file, tabSpace + "  ");
    }

    if (bonePrinted)
    {
      anythingPrinted = true;
      file << tabSpace << "</bone>\n";
    }
  };

  string name, path;
  Decompose(filePath, path, name);

  string fullPath = path + name + ".skeleton";
  ofstream file(fullPath, ios::out);
  assert(file.good());

  file << "<skeleton>\n";
  writeFn(scene->mRootNode, file, "  ");
  file << "</skeleton>\n";
  file.close();

  if (!anythingPrinted)
  {
    std::remove(fullPath.c_str());
  }
  else
  {
    AddToUsedFiles(fullPath);
  }
}

void PrintTextures_(const aiScene* scene, string filePath)
{
  // Embedded textures.
  if (scene->HasTextures())
  {
    for (unsigned int i = 0; i < scene->mNumTextures; i++)
    {
      aiTexture* t = scene->mTextures[i];
      string embId = GetTextureName(t, i);

      // Compressed.
      if (scene->mTextures[i]->mHeight == 0)
      {
        ofstream file(filePath + embId, fstream::out | std::fstream::binary);
        assert(file.good());

        file.write((const char*)scene->mTextures[i]->pcData, scene->mTextures[i]->mWidth);
      }
      else
      {
        unsigned char* buffer = (unsigned char*)scene->mTextures[i]->pcData;
        lodepng::encode(filePath, buffer, scene->mTextures[i]->mWidth, scene->mTextures[i]->mHeight);
      }
    }
  }
}

int main(int argc, char* argv[])
{
  try
  {
    if (argc < 2)
    {
      cout << "usage: Import 'fileToImport.format' <op> -t 'importTo' <op> -s 1.0\n";
      throw (-1);
    }

    Assimp::Importer importer;
    string dest, file = argv[1];
    Assimp::DefaultLogger::create("log.txt", Assimp::Logger::VERBOSE);
    for (int i = 0; i < argc; i++)
    {
      string arg = argv[i];
      Assimp::DefaultLogger::get()->info(arg);

      if (arg == "-t")
      {
        dest = filesystem::path(argv[i + 1]).append("").u8string();
      }

      if (arg == "-s")
      {
        float scale = (float)std::atof(argv[i + 1]);
        importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, scale);
      }
    }

    if (!dest.empty())
    {
      std::filesystem::create_directories(dest);
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

    for (int i = 0; i < (int)files.size(); i++)
    {
      file = files[i];
      const aiScene* scene = importer.ReadFile
      (
        file,
        aiProcess_Triangulate
        | aiProcess_CalcTangentSpace
        | aiProcess_FlipUVs
        | aiProcess_LimitBoneWeights
        | aiProcess_GenNormals
        | aiProcess_GlobalScale
      );

      if (scene == nullptr)
      {
        throw (-1);
      }
      g_scene = scene;

      filesystem::path pathToProcess = file;
      string fileName = pathToProcess.filename().u8string();
      string destFile = dest + fileName;

      PrintSkeleton_(scene, destFile);
      PrintMesh_(scene, destFile);
      PrintAnims_(scene, dest);
      PrintMaterial_(scene, dest, file);
      PrintTextures_(scene, dest);
    }

    // Report all in use files.
    fstream inUse("out.txt", ios::out);
    for (string fs : g_usedFiles)
    {
      inUse << fs << endl;
    }
    inUse.close();
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
