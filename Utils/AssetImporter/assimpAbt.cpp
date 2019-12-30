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

using namespace std;

class BoneNode;

unordered_map<string, BoneNode> g_skeletonMap;
const aiScene* g_scene = nullptr;

string GenUniqueMaterialName(aiMesh* mesh)
{
  aiString matName;
  g_scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_NAME, matName);

  string newName = string(matName.C_Str());
  replace(newName.begin(), newName.end(), '|', '_');
  replace(newName.begin(), newName.end(), '/', '_');

  return newName;
}

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

void PrintAnims_(const aiScene* scene, string file)
{
  if (!scene->HasAnimations())
    return;

  for (unsigned int i = 0; i < scene->mNumAnimations; i++)
  {
    aiAnimation* anim = scene->mAnimations[i];
    string path = file + "_";
    std::string animName(anim->mName.C_Str());
    replace(animName.begin(), animName.end(), '.', '_');
    replace(animName.begin(), animName.end(), '|', '_');
    path += animName + ".anim";

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

void PrintMaterial_(aiMesh* mesh, const aiScene* scene, string filePath)
{
  string fileName = filePath.substr(filePath.find_last_of("\\") + 1) + "_";
  filePath = filePath.substr(0, filePath.find_last_of("\\") + 1);
  filePath += fileName + GenUniqueMaterialName(mesh);

  if (mesh->mMaterialIndex >= 0)
  {
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    ofstream file(filePath + ".material", ios::out);
    assert(file.good());

    file << "<material>\n";
    int diffTexCnt = material->GetTextureCount(aiTextureType_DIFFUSE);
    if (diffTexCnt > 0)
    {
      aiString texture;
      material->GetTexture(aiTextureType_DIFFUSE, 0, &texture);
      string sText = texture.C_Str();
      sText = sText.substr(sText.find_last_of("\\") + 1);
      file << "  <diffuseTexture name = \"" + sText + "\"/>\n";
    }
    file << "</material>\n";
    file.close();
  }
}

void AppendMesh_(aiMesh* mesh, ofstream& file, string fileName)
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
    tag = "mesh";
  
  file << "  <" + tag + ">\n";
  file << "    <material name=\"" + fileName + GenUniqueMaterialName(mesh) + ".material\"/>\n";
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
            skinData[i].push_back(pair<int, float>(0, 0.0f));
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

void PrintMesh_(const aiScene* scene, string filePath)
{
  string fileName = filePath.substr(filePath.find_last_of("\\") + 1) + "_";

  string tag = "mesh";
  if (!g_skeletonMap.empty())
    tag = "skinMesh";

  ofstream file(filePath + "." + tag, ios::out);
  file << "<meshContainer>\n";

  function<void(aiNode*)> searchMeshFn = [&searchMeshFn, &scene, &file, &filePath, &fileName](aiNode* node) -> void
  {
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
      aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
      AppendMesh_(mesh, file, fileName);
      PrintMaterial_(mesh, scene, filePath);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
      searchMeshFn(node->mChildren[i]);
  };

  searchMeshFn(scene->mRootNode);

  file << "</meshContainer>\n";
  file.close();
}

void PrintSkeleton_(const aiScene* scene, string filePath)
{
  auto addBoneNodeFn = [](aiNode* node, aiBone* bone) -> void
  {
    BoneNode bn(node, 0);
    if (node->mName == bone->mName)
      bn.bone = bone;
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
          break;

        if (meshNode != nullptr)
          if (node == meshNode->mParent)
            break;

        addBoneNodeFn(node, bone);

        node = node->mParent;
      }
      node = scene->mRootNode->FindNode(bone->mName);
      function<void(aiNode*)> checkDownFn = [&checkDownFn, &bone, &addBoneNodeFn](aiNode* node) -> void // Go Down
      {
        if (node == nullptr)
          return;

        addBoneNodeFn(node, bone);

        for (unsigned int i = 0; i < node->mNumChildren; i++)
          checkDownFn(node->mChildren[i]);
      };
      checkDownFn(node);
    }
  }

  for (auto& bone : bones)
  {
    if (g_skeletonMap.find(bone->mName.C_Str()) != g_skeletonMap.end())
      g_skeletonMap[bone->mName.C_Str()].bone = bone;
  }

  // Assign indices
  function<void(aiNode*, unsigned int&)> assignBoneIndexFn = [&assignBoneIndexFn](aiNode* node, unsigned int& index) -> void
  {
    if (g_skeletonMap.find(node->mName.C_Str()) != g_skeletonMap.end())
      g_skeletonMap[node->mName.C_Str()].boneIndex = index++;

    for (unsigned int i = 0; i < node->mNumChildren; i++)
      assignBoneIndexFn(node->mChildren[i], index);
  };

  unsigned int boneIndex = 0;
  assignBoneIndexFn(scene->mRootNode, boneIndex);

  // Print
  function<void(aiNode*, ofstream&, string)> writeFn = [&writeFn](aiNode* node, ofstream& file, string tabSpace) -> void
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
      writeFn(node->mChildren[i], file, tabSpace + "  ");

    if (bonePrinted)
      file << tabSpace << "</bone>\n";
  };

  ofstream file(filePath + ".skeleton", ios::out);
  assert(file.good());

  file << "<skeleton>\n";
  writeFn(scene->mRootNode, file, "  ");
  file << "</skeleton>\n";
  file.close();
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    cout << "usage: AssimpAbt \"fileToImport.format\"\n";
    return -1;
  }

  string file = argv[1];
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_LimitBoneWeights | aiProcess_GenNormals);
  if (scene == nullptr)
    return -1;
  g_scene = scene;
  
  string pathPart = file.substr(0, file.find_last_of("."));

  PrintSkeleton_(scene, pathPart);
  PrintMesh_(scene, pathPart);
  PrintAnims_(scene, pathPart);

  return 0;
}
