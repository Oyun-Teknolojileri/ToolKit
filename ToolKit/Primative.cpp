/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Primative.h"

#include "Camera.h"
#include "DirectionComponent.h"
#include "Material.h"
#include "MathUtil.h"
#include "Mesh.h"
#include "Node.h"
#include "ResourceComponent.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

  UIHint FloatHint = {false, true, 0.0f, 100.0f, 0.25f, false};

  TKDefineClass(Billboard, Entity);

  Billboard::Billboard() {}

  Billboard::Billboard(const Settings& settings) : m_settings(settings) {}

  void Billboard::NativeConstruct()
  {
    Super::NativeConstruct();
    AddComponent<MeshComponent>();
  }

  void Billboard::LookAt(CameraPtr cam, float scale)
  {
    // Billboard placement.
    if (m_settings.distanceToCamera > 0.0f)
    {
      if (cam->IsOrtographic())
      {
        m_node->SetTranslation(m_worldLocation);
        if (m_settings.heightInScreenSpace > 0.0f)
        {
          // Compensate shrinkage due to height changes.
          m_node->SetScale(Vec3(m_settings.heightInScreenSpace * scale));
        }
      }
      else
      {
        Vec3 cdir                    = cam->GetComponent<DirectionComponent>()->GetDirection();
        Vec3 camWorldPos             = cam->m_node->GetTranslation();
        Vec3 dir                     = glm::normalize(m_worldLocation - camWorldPos);

        // Always place at the same distance from the near plane.
        float radialToPlanarDistance = 1.0f / glm::dot(cdir, dir);
        if (radialToPlanarDistance < 0)
        {
          return;
        }

        Vec3 billWorldPos = camWorldPos + dir * m_settings.distanceToCamera * radialToPlanarDistance;

        m_node->SetTranslation(billWorldPos);
        if (m_settings.heightInScreenSpace > 0.0f)
        {
          // Compensate shrinkage due to height changes.
          float magicScale = 6.0f;
          m_node->SetScale(Vec3(magicScale * m_settings.heightInScreenSpace / scale));
        }

        // Compensate shrinkage due to fov changes.
        float initialFovRadians    = glm::quarter_pi<float>(); // Initial field of view in radians
        float newFovRadians        = cam->Fov();               // New field of view in radians

        // Calculate scaling factor
        float initialFrustumHeight = tan(initialFovRadians / 2.0f);
        float newFrustumHeight     = tan(newFovRadians / 2.0f);
        float scaleFactor          = newFrustumHeight / initialFrustumHeight;

        m_node->Scale(Vec3(scaleFactor));
      }
    }
    else
    {
      m_node->SetTranslation(m_worldLocation);
    }

    if (m_settings.lookAtCamera)
    {
      Quaternion camOrientation = cam->m_node->GetOrientation();
      m_node->SetOrientation(camOrientation);
    }
  }

  Entity* Billboard::CopyTo(Entity* copyTo) const
  {
    Entity::CopyTo(copyTo);
    Billboard* ntt       = static_cast<Billboard*>(copyTo);
    ntt->m_settings      = m_settings;
    ntt->m_worldLocation = m_worldLocation;
    return ntt;
  }

  XmlNode* Billboard::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

  TKDefineClass(Cube, Entity);

  Cube::Cube() {}

  Entity* Cube::CopyTo(Entity* copyTo) const { return Entity::CopyTo(copyTo); }

  void Cube::NativeConstruct()
  {
    Super::NativeConstruct();

    AddComponent<MeshComponent>();
    AddComponent<MaterialComponent>();

    Generate();
  }

  void Cube::Generate()
  {
    MeshComponentPtr meshCmp = GetMeshComponent();
    Vec3 scl                 = GetCubeScaleVal();
    Generate(meshCmp, scl);
  }

  void Cube::ParameterConstructor()
  {
    Super::ParameterConstructor();
    CubeScale_Define(Vec3(1.0f), "Geometry", 90, true, true);
  }

  void Cube::ParameterEventConstructor()
  {
    Super::ParameterEventConstructor();

    ValueUpdateFn upVal = [this](Value& old, Value& val) -> void { Generate(); };
    ParamCubeScale().m_onValueChangedFn.push_back(upVal);
  }

  XmlNode* Cube::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    XmlNode* nttNode = Super::DeSerializeImp(info, parent);
    Generate();

    return nttNode->first_node(StaticClass()->Name.c_str());
  }

  XmlNode* Cube::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

  void Cube::Generate(MeshComponentPtr meshComp, const Vec3& scale)
  {
    VertexArray vertices;
    vertices.resize(36);

    Vec3 corners[8] {
        Vec3(-0.5f, 0.5f, 0.5f) * scale,   // FTL.
        Vec3(-0.5f, -0.5f, 0.5f) * scale,  // FBL.
        Vec3(0.5f, -0.5f, 0.5f) * scale,   // FBR.
        Vec3(0.5f, 0.5f, 0.5f) * scale,    // FTR.
        Vec3(-0.5f, 0.5f, -0.5f) * scale,  // BTL.
        Vec3(-0.5f, -0.5f, -0.5f) * scale, // BBL.
        Vec3(0.5f, -0.5f, -0.5f) * scale,  // BBR.
        Vec3(0.5f, 0.5f, -0.5f) * scale,   // BTR.
    };

    // Front
    vertices[0].pos   = corners[0];
    vertices[0].tex   = Vec2(0.0f, 1.0f);
    vertices[0].norm  = Vec3(0.0f, 0.0f, 1.0f);
    vertices[1].pos   = corners[1];
    vertices[1].tex   = Vec2(0.0f, 0.0f);
    vertices[1].norm  = Vec3(0.0f, 0.0f, 1.0f);
    vertices[2].pos   = corners[2];
    vertices[2].tex   = Vec2(1.0f, 0.0f);
    vertices[2].norm  = Vec3(0.0f, 0.0f, 1.0f);

    vertices[3].pos   = corners[0];
    vertices[3].tex   = Vec2(0.0f, 1.0f);
    vertices[3].norm  = Vec3(0.0f, 0.0f, 1.0f);
    vertices[4].pos   = corners[2];
    vertices[4].tex   = Vec2(1.0f, 0.0f);
    vertices[4].norm  = Vec3(0.0f, 0.0f, 1.0f);
    vertices[5].pos   = corners[3];
    vertices[5].tex   = Vec2(1.0f, 1.0f);
    vertices[5].norm  = Vec3(0.0f, 0.0f, 1.0f);

    // Right
    vertices[6].pos   = corners[3];
    vertices[6].tex   = Vec2(0.0f, 1.0f);
    vertices[6].norm  = Vec3(1.0f, 0.0f, 0.0f);
    vertices[7].pos   = corners[2];
    vertices[7].tex   = Vec2(0.0f, 0.0f);
    vertices[7].norm  = Vec3(1.0f, 0.0f, 0.0f);
    vertices[8].pos   = corners[6];
    vertices[8].tex   = Vec2(1.0f, 0.0f);
    vertices[8].norm  = Vec3(1.0f, 0.0f, 0.0f);

    vertices[9].pos   = corners[3];
    vertices[9].tex   = Vec2(0.0f, 1.0f);
    vertices[9].norm  = Vec3(1.0f, 0.0f, 0.0f);
    vertices[10].pos  = corners[6];
    vertices[10].tex  = Vec2(1.0f, 0.0f);
    vertices[10].norm = Vec3(1.0f, 0.0f, 0.0f);
    vertices[11].pos  = corners[7];
    vertices[11].tex  = Vec2(1.0f, 1.0f);
    vertices[11].norm = Vec3(1.0f, 0.0f, 0.0f);

    // Top
    vertices[12].pos  = corners[0];
    vertices[12].tex  = Vec2(0.0f, 0.0f);
    vertices[12].norm = Vec3(0.0f, 1.0f, 0.0f);
    vertices[13].pos  = corners[3];
    vertices[13].tex  = Vec2(1.0f, 0.0f);
    vertices[13].norm = Vec3(0.0f, 1.0f, 0.0f);
    vertices[14].pos  = corners[7];
    vertices[14].tex  = Vec2(1.0f, 1.0f);
    vertices[14].norm = Vec3(0.0f, 1.0f, 0.0f);

    vertices[15].pos  = corners[0];
    vertices[15].tex  = Vec2(0.0f, 0.0f);
    vertices[15].norm = Vec3(0.0f, 1.0f, 0.0f);
    vertices[16].pos  = corners[7];
    vertices[16].tex  = Vec2(1.0f, 1.0f);
    vertices[16].norm = Vec3(0.0f, 1.0f, 0.0f);
    vertices[17].pos  = corners[4];
    vertices[17].tex  = Vec2(0.0f, 1.0f);
    vertices[17].norm = Vec3(0.0f, 1.0f, 0.0f);

    // Back
    vertices[18].pos  = corners[4];
    vertices[18].tex  = Vec2(0.0f, 1.0f);
    vertices[18].norm = Vec3(0.0f, 0.0f, -1.0f);
    vertices[19].pos  = corners[6];
    vertices[19].tex  = Vec2(1.0f, 0.0f);
    vertices[19].norm = Vec3(0.0f, 0.0f, -1.0f);
    vertices[20].pos  = corners[5];
    vertices[20].tex  = Vec2(0.0f, 0.0f);
    vertices[20].norm = Vec3(0.0f, 0.0f, -1.0f);

    vertices[21].pos  = corners[4];
    vertices[21].tex  = Vec2(0.0f, 1.0f);
    vertices[21].norm = Vec3(0.0f, 0.0f, -1.0f);
    vertices[22].pos  = corners[7];
    vertices[22].tex  = Vec2(1.0f, 1.0f);
    vertices[22].norm = Vec3(0.0f, 0.0f, -1.0f);
    vertices[23].pos  = corners[6];
    vertices[23].tex  = Vec2(1.0f, 0.0f);
    vertices[23].norm = Vec3(0.0f, 0.0f, -1.0f);

    // Left
    vertices[24].pos  = corners[0];
    vertices[24].tex  = Vec2(0.0f, 1.0f);
    vertices[24].norm = Vec3(-1.0f, 0.0f, 0.0f);
    vertices[25].pos  = corners[5];
    vertices[25].tex  = Vec2(1.0f, 0.0f);
    vertices[25].norm = Vec3(-1.0f, 0.0f, 0.0f);
    vertices[26].pos  = corners[1];
    vertices[26].tex  = Vec2(0.0f, 0.0f);
    vertices[26].norm = Vec3(-1.0f, 0.0f, 0.0f);

    vertices[27].pos  = corners[0];
    vertices[27].tex  = Vec2(0.0f, 1.0f);
    vertices[27].norm = Vec3(-1.0f, 0.0f, 0.0f);
    vertices[28].pos  = corners[4];
    vertices[28].tex  = Vec2(1.0f, 1.0f);
    vertices[28].norm = Vec3(-1.0f, 0.0f, 0.0f);
    vertices[29].pos  = corners[5];
    vertices[29].tex  = Vec2(1.0f, 0.0f);
    vertices[29].norm = Vec3(-1.0f, 0.0f, 0.0f);

    // Bottom
    vertices[30].pos  = corners[1];
    vertices[30].tex  = Vec2(0.0f, 1.0f);
    vertices[30].norm = Vec3(0.0f, -1.0f, 0.0f);
    vertices[31].pos  = corners[6];
    vertices[31].tex  = Vec2(1.0f, 0.0f);
    vertices[31].norm = Vec3(0.0f, -1.0f, 0.0f);
    vertices[32].pos  = corners[2];
    vertices[32].tex  = Vec2(0.0f, 0.0f);
    vertices[32].norm = Vec3(0.0f, -1.0f, 0.0f);

    vertices[33].pos  = corners[1];
    vertices[33].tex  = Vec2(0.0f, 1.0f);
    vertices[33].norm = Vec3(0.0f, -1.0f, 0.0f);
    vertices[34].pos  = corners[5];
    vertices[34].tex  = Vec2(1.0f, 1.0f);
    vertices[34].norm = Vec3(0.0f, -1.0f, 0.0f);
    vertices[35].pos  = corners[6];
    vertices[35].tex  = Vec2(1.0f, 0.0f);
    vertices[35].norm = Vec3(0.0f, -1.0f, 0.0f);

    MeshPtr mesh      = meshComp->GetMeshVal();
    mesh->UnInit();
    mesh->m_vertexCount        = (uint) vertices.size();
    mesh->m_clientSideVertices = vertices;
    mesh->m_clientSideIndices  = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
                                  18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35};

    mesh->m_indexCount         = (uint) mesh->m_clientSideIndices.size();
    mesh->m_material           = GetMaterialManager()->GetCopyOfDefaultMaterial(false);

    mesh->CalculateAABB();
    mesh->ConstructFaces();
  }

  TKDefineClass(Quad, Entity);

  Quad::Quad() {}

  void Quad::NativeConstruct()
  {
    Super::NativeConstruct();

    AddComponent<MeshComponent>();
    AddComponent<MaterialComponent>();

    Generate();
  }

  Entity* Quad::CopyTo(Entity* copyTo) const { return Entity::CopyTo(copyTo); }

  XmlNode* Quad::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

  XmlNode* Quad::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    XmlNode* nttNode = Super::DeSerializeImp(info, parent);
    Generate();

    return nttNode->first_node(StaticClass()->Name.c_str());
  }

  void Quad::Generate()
  {
    VertexArray vertices;
    vertices.resize(4);

    // Front
    vertices[0].pos            = Vec3(-0.5f, 0.5f, 0.0f);
    vertices[0].tex            = Vec2(0.0f, 0.0f);
    vertices[0].norm           = Vec3(0.0f, 0.0f, 1.0f);
    vertices[0].btan           = Vec3(0.0f, 1.0f, 0.0f);

    vertices[1].pos            = Vec3(-0.5f, -0.5f, 0.0f);
    vertices[1].tex            = Vec2(0.0f, 1.0f);
    vertices[1].norm           = Vec3(0.0f, 0.0f, 1.0f);
    vertices[1].btan           = Vec3(0.0f, 1.0f, 0.0f);

    vertices[2].pos            = Vec3(0.5f, -0.5f, 0.0f);
    vertices[2].tex            = Vec2(1.0f, 1.0f);
    vertices[2].norm           = Vec3(0.0f, 0.0f, 1.0f);
    vertices[2].btan           = Vec3(0.0f, 1.0f, 0.0f);

    vertices[3].pos            = Vec3(0.5f, 0.5f, 0.0f);
    vertices[3].tex            = Vec2(1.0f, 0.0f);
    vertices[3].norm           = Vec3(0.0f, 0.0f, 1.0f);
    vertices[3].btan           = Vec3(0.0f, 1.0f, 0.0f);

    MeshPtr mesh               = GetMeshComponent()->GetMeshVal();
    mesh->m_vertexCount        = (uint) vertices.size();
    mesh->m_clientSideVertices = vertices;
    mesh->m_indexCount         = 6;
    mesh->m_clientSideIndices  = {0, 1, 2, 0, 2, 3};
    mesh->m_material           = GetMaterialManager()->GetCopyOfDefaultMaterial(false);

    mesh->CalculateAABB();
    mesh->ConstructFaces();
  }

  TKDefineClass(Sphere, Entity);

  Sphere::Sphere() {}

  void Sphere::NativeConstruct()
  {
    Super::NativeConstruct();

    AddComponent<MeshComponent>();
    AddComponent<MaterialComponent>();

    Generate(GetMeshComponent(), GetRadiusVal(), GetNumRingVal(), GetNumSegVal());
  }

  Entity* Sphere::CopyTo(Entity* copyTo) const
  {
    Entity::CopyTo(copyTo);
    Sphere* ntt = static_cast<Sphere*>(copyTo);
    return ntt;
  }

  XmlNode* Sphere::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    XmlNode* nttNode = Super::DeSerializeImp(info, parent);
    Generate(GetMeshComponent(), GetRadiusVal(), GetNumRingVal(), GetNumSegVal());

    return nttNode->first_node(StaticClass()->Name.c_str());
  }

  XmlNode* Sphere::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

  void Sphere::ParameterConstructor()
  {
    Super::ParameterConstructor();
    Radius_Define(1.0f, "Geometry", 90, true, true, FloatHint);
    NumRing_Define(32, "Geometry", 90, true, true);
    NumSeg_Define(32, "Geometry", 90, true, true);
  }

  void Sphere::ParameterEventConstructor()
  {
    Super::ParameterEventConstructor();

    auto genFn = [this]() -> void { Generate(GetMeshComponent(), GetRadiusVal(), GetNumRingVal(), GetNumSegVal()); };
    ParamRadius().m_onValueChangedFn.push_back([=](Value& oldVal, Value& newVal) -> void { genFn(); });
    ParamNumRing().m_onValueChangedFn.push_back([=](Value& oldVal, Value& newVal) -> void { genFn(); });
    ParamNumSeg().m_onValueChangedFn.push_back([=](Value& oldVal, Value& newVal) -> void { genFn(); });
  }

  void Sphere::Generate(MeshComponentPtr meshComp, float r, int numRing, int numSeg)
  {
    int nRings    = numRing;
    int nSegments = numSeg;

    VertexArray vertices;
    std::vector<uint> indices;

    float fDeltaRingAngle = (glm::pi<float>() / nRings);
    float fDeltaSegAngle  = (glm::two_pi<float>() / nSegments);
    int wVerticeIndex     = 0;

    // Generate the group of rings for the sphere
    for (int ring = 0; ring <= nRings; ring++)
    {
      float r0 = r * sinf(ring * fDeltaRingAngle);
      float y0 = r * cosf(ring * fDeltaRingAngle);

      // Generate the group of segments for the current ring
      for (int seg = 0; seg <= nSegments; seg++)
      {
        float x0 = r0 * sinf(seg * fDeltaSegAngle);
        float z0 = r0 * cosf(seg * fDeltaSegAngle);

        // Add one vertex to the strip which makes up the sphere
        Vertex v;
        v.pos  = Vec3(x0, y0, z0);
        v.norm = Vec3(x0, y0, z0);
        v.tex  = Vec2((float) seg / (float) nSegments, (float) ring / (float) nRings);

        float r2, zenith, azimuth;
        ToSpherical(v.pos, r2, zenith, azimuth);
        v.btan = Vec3(r * glm::cos(zenith) * glm::sin(azimuth),
                      -r * glm::sin(zenith),
                      r * glm::cos(zenith) * glm::cos(azimuth));

        vertices.push_back(v);

        if (ring != nRings)
        {
          // each vertex (except the last) has six indices pointing to it
          indices.push_back(wVerticeIndex + nSegments + 1);
          indices.push_back(wVerticeIndex);
          indices.push_back(wVerticeIndex + nSegments);
          indices.push_back(wVerticeIndex + nSegments + 1);
          indices.push_back(wVerticeIndex + 1);
          indices.push_back(wVerticeIndex);
          wVerticeIndex++;
        }
      } // end for seg
    }   // end for ring

    MeshPtr mesh = meshComp->GetMeshVal();
    mesh->UnInit();
    mesh->m_vertexCount        = (uint) vertices.size();
    mesh->m_clientSideVertices = vertices;
    mesh->m_indexCount         = (uint) indices.size();
    mesh->m_clientSideIndices  = indices;
    mesh->m_material           = GetMaterialManager()->GetCopyOfDefaultMaterial(false);

    mesh->CalculateAABB();
    mesh->ConstructFaces();
  }

  TKDefineClass(Cone, Entity);

  Cone::Cone() {}

  void Cone::NativeConstruct()
  {
    Super::NativeConstruct();

    AddComponent<MaterialComponent>();
    AddComponent<MeshComponent>();

    Generate();
  }

  // https://github.com/OGRECave/ogre-procedural/blob/master/library/src/ProceduralConeGenerator.cpp
  void Cone::Generate()
  {
    VertexArray vertices;
    UIntArray indices;

    float height      = GetHeightVal();
    float radius      = GetRadiusVal();
    int nSegBase      = GetSegBaseVal();
    int nSegHeight    = GetSegHeightVal();

    float deltaAngle  = glm::two_pi<float>() / (float) nSegBase;
    float deltaHeight = height / nSegHeight;
    int offset        = 0;

    for (int i = 0; i <= nSegHeight; i++)
    {
      float r0 = radius * (1 - i / (float) nSegHeight);
      for (int j = 0; j <= nSegBase; j++)
      {
        float x0 = r0 * glm::cos(j * deltaAngle);
        float z0 = r0 * glm::sin(j * deltaAngle);
        Vec3 p   = Vec3(x0, i * deltaHeight, z0);

        Vertex v {
            p,
            glm::normalize(p),
            Vec2(j / (float) (nSegBase), i / (float) nSegHeight),
            ZERO // btan missing.
        };

        vertices.push_back(v);

        if (i != nSegHeight && j != nSegBase)
        {
          indices.push_back(offset + nSegBase + 2);
          indices.push_back(offset);
          indices.push_back(offset + nSegBase + 1);
          indices.push_back(offset + nSegBase + 2);
          indices.push_back(offset + 1);
          indices.push_back(offset);
        }

        offset++;
      }
    }

    // low cap
    int centerIndex = offset;

    Vertex v {
        ZERO,
        -Y_AXIS,
        Y_AXIS,
        ZERO // btan missing.
    };
    vertices.push_back(v);

    offset++;
    for (int j = 0; j <= nSegBase; j++)
    {
      float x0 = radius * glm::cos(j * deltaAngle);
      float z0 = radius * glm::sin(j * deltaAngle);

      Vertex v {
          Vec3(x0, 0.0f, z0),
          -Y_AXIS,
          Vec2(j / (float) nSegBase, 0.0f),
          ZERO // btan missing.
      };
      vertices.push_back(v);

      if (j != nSegBase)
      {
        indices.push_back(centerIndex);
        indices.push_back(offset);
        indices.push_back(offset + 1);
      }
      offset++;
    }

    MeshPtr mesh = GetComponent<MeshComponent>()->GetMeshVal();
    mesh->UnInit();
    mesh->m_vertexCount        = (uint) vertices.size();
    mesh->m_clientSideVertices = vertices;
    mesh->m_indexCount         = (uint) indices.size();
    mesh->m_clientSideIndices  = indices;
    mesh->m_material           = GetMaterialManager()->GetCopyOfDefaultMaterial(false);

    mesh->CalculateAABB();
    mesh->ConstructFaces();
  }

  Entity* Cone::CopyTo(Entity* copyTo) const
  {
    Entity::CopyTo(copyTo);
    Cone* ntt = static_cast<Cone*>(copyTo);
    return ntt;
  }

  void Cone::Generate(float height, float radius, int segBase, int segHeight)
  {
    // Specifically doing it this way to prevent ParameterEventConstructor's to regenerate
    // at each Set()
    ParamHeight().GetVar<float>()  = height;
    ParamRadius().GetVar<float>()  = radius;
    ParamSegBase().GetVar<int>()   = segBase;
    ParamSegHeight().GetVar<int>() = segHeight;

    Generate();
  }

  void Cone::ParameterConstructor()
  {
    Super::ParameterConstructor();

    UIHint hint;
    hint.increment      = 0.25f;
    hint.isRangeLimited = true;
    hint.rangeMin       = 0.0f;
    hint.rangeMax       = 100.0f;

    Height_Define(1.0f, "Geometry", 90, true, true, hint);
    Radius_Define(1.0f, "Geometry", 90, true, true, hint);
    SegBase_Define(30, "Geometry", 90, true, true);
    SegHeight_Define(20, "Geometry", 90, true, true);
  }

  void Cone::ParameterEventConstructor()
  {
    Super::ParameterEventConstructor();

    auto regenFn = [this](const Value& old, const Value& val) -> void { Generate(); };
    ParamHeight().m_onValueChangedFn.push_back(regenFn);
    ParamRadius().m_onValueChangedFn.push_back(regenFn);
    ParamSegBase().m_onValueChangedFn.push_back(regenFn);
    ParamSegHeight().m_onValueChangedFn.push_back(regenFn);
  }

  XmlNode* Cone::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

  XmlNode* Cone::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    XmlNode* nttNode = Super::DeSerializeImp(info, parent);
    Generate();

    return nttNode->first_node(StaticClass()->Name.c_str());
  }

  TKDefineClass(Arrow2d, Entity);

  Arrow2d::Arrow2d() { m_label = AxisLabel::X; }

  void Arrow2d::NativeConstruct()
  {
    Super::NativeConstruct();

    AddComponent<MaterialComponent>();
    AddComponent<MeshComponent>();

    Generate(AxisLabel::Y);
  }

  Entity* Arrow2d::CopyTo(Entity* copyTo) const
  {
    Entity::CopyTo(copyTo);
    Arrow2d* ntt = static_cast<Arrow2d*>(copyTo);
    ntt->m_label = m_label;

    return ntt;
  }

  XmlNode* Arrow2d::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

  void Arrow2d::Generate(AxisLabel axis)
  {
    m_label = axis;

    VertexArray vertices;
    vertices.resize(8);

    // Line
    vertices[0].pos                    = Vec3(0.0f, 0.0f, 0.0f);
    vertices[1].pos                    = Vec3(0.8f, 0.0f, 0.0f);

    // Triangle
    vertices[2].pos                    = Vec3(0.8f, -0.2f, 0.0f);
    vertices[3].pos                    = Vec3(0.8f, 0.2f, 0.0f);
    vertices[4].pos                    = Vec3(0.8f, 0.2f, 0.0f);
    vertices[5].pos                    = Vec3(1.0f, 0.0f, 0.0f);
    vertices[6].pos                    = Vec3(1.0f, 0.0f, 0.0f);
    vertices[7].pos                    = Vec3(0.8f, -0.2f, 0.0f);

    MaterialPtr newMat                 = GetMaterialManager()->GetCopyOfUnlitColorMaterial();
    newMat->GetRenderState()->drawType = DrawType::Line;
    newMat->m_color                    = Vec3(0.89f, 0.239f, 0.341f);

    Quaternion rotation;
    if (m_label == AxisLabel::Y)
    {
      newMat->m_color = Vec3(0.537f, 0.831f, 0.07f);
      rotation        = glm::angleAxis(glm::half_pi<float>(), Z_AXIS);
    }

    if (m_label == AxisLabel::Z)
    {
      newMat->m_color = Vec3(0.196f, 0.541f, 0.905f);
      rotation        = glm::angleAxis(-glm::half_pi<float>(), Y_AXIS);
    }

    for (size_t i = 0; i < vertices.size(); i++)
    {
      vertices[i].pos = rotation * vertices[i].pos;
    }

    MeshComponentPtr mesh                    = GetComponent<MeshComponent>();
    mesh->GetMeshVal()->m_vertexCount        = (uint) vertices.size();
    mesh->GetMeshVal()->m_clientSideVertices = vertices;
    mesh->GetMeshVal()->m_material           = newMat;

    mesh->GetMeshVal()->CalculateAABB();
    mesh->GetMeshVal()->ConstructFaces();
  }

  TKDefineClass(LineBatch, Entity);

  LineBatch::LineBatch() {}

  void LineBatch::NativeConstruct()
  {
    Super::NativeConstruct();
    AddComponent<MeshComponent>();
  }

  Entity* LineBatch::CopyTo(Entity* copyTo) const { return Entity::CopyTo(copyTo); }

  XmlNode* LineBatch::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

  void LineBatch::Generate(const Vec3Array& linePnts, const Vec3& color, DrawType t, float lineWidth)
  {
    VertexArray vertices;
    vertices.resize(linePnts.size());

    MeshPtr mesh = GetComponent<MeshComponent>()->GetMeshVal();
    mesh->UnInit();
    mesh->m_material                             = GetMaterialManager()->GetCopyOfUnlitColorMaterial(false);
    mesh->m_material->GetRenderState()->drawType = t;

    for (size_t i = 0; i < linePnts.size(); i++)
    {
      vertices[i].pos = linePnts[i];
    }

    mesh->m_vertexCount                           = (uint) vertices.size();
    mesh->m_clientSideVertices                    = vertices;
    mesh->m_material->m_color                     = color;
    mesh->m_material->GetRenderState()->lineWidth = lineWidth;

    mesh->CalculateAABB();
  }

  void MeshGenerator::GenerateCircleMesh(MeshPtr mesh, int numSegments, float radius)
  {
    const float step    = glm::two_pi<float>() / (float) (numSegments);
    mesh->m_vertexCount = numSegments + 1;
    mesh->m_indexCount  = numSegments * 3;
    mesh->m_clientSideVertices.resize(numSegments + 2);
    mesh->m_clientSideIndices.resize((numSegments + 1) * 3);
    mesh->m_clientSideVertices[0] = {
        // center point
        Vec3(0.0f),
        Vec3(0.0f, 0.0f, -1.0f), // normal
        Vec2(0.5f, 0.5f),        // tex coord
        Vec3(0.0f, 1.0f, 0.0f)   // btan
    };
    mesh->m_boundingBox = BoundingBox(Vec3(-radius), Vec3(radius));

    // create vertices
    for (int i = 0; i < numSegments; i++)
    {
      float f = step * (float) i;
      float s = glm::sin(f), c = glm::cos(f);
      float snorm                       = s * 0.5f + 0.5f;
      float cnorm                       = c * 0.5f + 0.5f;

      const float sqrt2                 = 1.41421356f; // square root of 2
      float strechS                     = sqrt2 * glm::abs(s);
      float strechC                     = sqrt2 * glm::abs(c);

      mesh->m_clientSideVertices[i + 1] = {
          Vec3(c * radius, s * radius, 0.0f),
          Vec3(0.0f, 0.0f, -1.0f),                // normal
          Vec2(cnorm * strechS, snorm * strechC), // tex coord
          Vec3(c, s, 0.0f)                        // btan
      };
    }

    // create indices
    for (int i = 0; i < numSegments; i++)
    {
      mesh->m_clientSideIndices[i * 3 + 0] = 0;
      mesh->m_clientSideIndices[i * 3 + 1] = i + 1; // jump center point
      mesh->m_clientSideIndices[i * 3 + 2] = i + 2; // next vertex pos
    }
    mesh->m_clientSideIndices[numSegments * 3 + 0] = 0;
    mesh->m_clientSideIndices[numSegments * 3 + 1] = numSegments; // jump center point
    mesh->m_clientSideIndices[numSegments * 3 + 2] = 1;           // next vertex pos

    mesh->ConstructFaces();
  }

  void MeshGenerator::GenerateConeMesh(MeshPtr mesh, float height, int numSegments, float outerAngle)
  {
    mesh->UnInit();

    // Middle line.
    Vec3 dir = Vec3(0.0f, 0.0f, -1.0f) * height;
    Vec3 per = Vec3(1.0f, 0.0f, 0.0f);

    mesh->m_clientSideVertices.clear();
    mesh->m_clientSideIndices.clear();
    mesh->m_clientSideVertices.reserve(numSegments + 2);
    mesh->m_clientSideVertices.push_back({ZERO});
    mesh->m_clientSideVertices.push_back({dir});
    // Calculating circles.
    // 0.62 = 0.5 + 0.12 for slightly bigger cone
    float outerCircleRadius = height * glm::tan(glm::radians(outerAngle * 0.62f));
    Vec3 outStartPoint      = dir + per * outerCircleRadius;

    mesh->m_clientSideVertices.push_back({outStartPoint});
    float deltaAngle = glm::two_pi<float>() / (numSegments);
    Mat4 identity    = Mat4();
    for (int i = 1; i <= numSegments; i++)
    {
      // Outer circle vertices.
      Mat4 m_rot    = glm::rotate(identity, deltaAngle, dir);
      outStartPoint = Vec3(m_rot * Vec4(outStartPoint, 1.0f));
      mesh->m_clientSideVertices.push_back({outStartPoint});
    }

    // Cone
    for (int i = 0; i < numSegments; ++i)
    {
      mesh->m_clientSideIndices.push_back(0);
      mesh->m_clientSideIndices.push_back(i + 3);
      mesh->m_clientSideIndices.push_back(i + 2);
    }

    // circle
    for (int i = 0; i < numSegments; ++i)
    {
      mesh->m_clientSideIndices.push_back(1);
      mesh->m_clientSideIndices.push_back(i + 2);
      mesh->m_clientSideIndices.push_back(i + 3);
    }

    mesh->m_vertexCount = (uint) mesh->m_clientSideVertices.size();
    mesh->m_indexCount  = (uint) mesh->m_clientSideIndices.size();
    mesh->CalculateAABB();
  }

} // namespace ToolKit
