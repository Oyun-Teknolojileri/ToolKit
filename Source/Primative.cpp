#include "stdafx.h"
#include "Primative.h"
#include "Mesh.h"
#include "ToolKit.h"
#include "MathUtil.h"
#include "Directional.h"
#include "Node.h"
#include "DebugNew.h"

namespace ToolKit
{

  Billboard::Billboard(const Settings& settings)
    : m_settings(settings)
  {
  }

  void Billboard::LookAt(Camera* cam, float scale)
  {
    Camera::CamData data = cam->GetData();

    // Billboard placement.
    if (m_settings.distanceToCamera > 0.0f)
    {
      if (cam->IsOrtographic())
      {
        m_node->SetTranslation(m_worldLocation, TransformationSpace::TS_WORLD);

        if (m_settings.heightInScreenSpace > 0.0f)
        {
          m_node->SetScale(Vec3(m_settings.heightInScreenSpace * scale));
        }
      }
      else
      {
        Vec3 cdir = cam->GetDir();
        Vec3 camWorldPos = cam->m_node->GetTranslation(TransformationSpace::TS_WORLD);
        Vec3 dir = glm::normalize(m_worldLocation - camWorldPos);

        float radialToPlanarDistance = 1.0f / glm::dot(cdir, dir); // Always place at the same distance from the near plane.
        if (radialToPlanarDistance < 0)
        {
          return;
        }

        Vec3 billWorldPos = camWorldPos + dir * m_settings.distanceToCamera * radialToPlanarDistance;
        m_node->SetTranslation(billWorldPos, TransformationSpace::TS_WORLD);

        if (m_settings.heightInScreenSpace > 0.0f)
        {
          float magicScale = 6.0f;
          m_node->SetScale(Vec3(magicScale * m_settings.heightInScreenSpace / data.height)); // Compensate shrinkage due to height changes.
        }
      }

      if (m_settings.lookAtCamera)
      {
        Quaternion camOrientation = cam->m_node->GetOrientation(TransformationSpace::TS_WORLD);
        m_node->SetOrientation(camOrientation, TransformationSpace::TS_WORLD);
      }
    }
  }

  Entity* Billboard::GetCopy(Entity* copyTo) const
  {
    Drawable::GetCopy(copyTo);
    Billboard* ntt = static_cast<Billboard*> (copyTo);
    ntt->m_settings = m_settings;
    ntt->m_worldLocation = m_worldLocation;
    return ntt;
  }

  Entity* Billboard::GetInstance(Entity* copyTo) const
  {
    Drawable::GetInstance(copyTo);
    Billboard* instance = static_cast<Billboard*> (copyTo);
    instance->m_settings = m_settings;
    instance->m_worldLocation = m_worldLocation;
    return nullptr;
  }

  EntityType Billboard::GetType() const
  {
    return EntityType::Entity_Billboard;
  }

  Cube::Cube(bool genDef)
  {
    if (genDef)
    {
      Generate();
    }
  }

  Cube::Cube(const Params& params)
    : m_params(params)
  {
    Generate();
  }

  Entity* Cube::GetCopy(Entity* copyTo) const
  {
    return Drawable::GetCopy(copyTo);
  }

  EntityType Cube::GetType() const
  {
    return EntityType::Entity_Cube;
  }

  void Cube::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    Entity::Serialize(doc, parent);
    m_params.Serialize(doc, parent->last_node());
  }

  void Cube::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Entity::DeSerialize(doc, parent);
    m_params.DeSerialize(doc, parent);
    Generate();
  }

  Entity* Cube::GetInstance(Entity* copyTo) const
  {
    Drawable::GetInstance(copyTo);
    Cube* instance = static_cast<Cube*> (copyTo);
    instance->m_params = m_params;

    return instance;
  }

  void Cube::Generate()
  {
    VertexArray vertices;
    vertices.resize(36);

    const Vec3& scale = m_params.m_variants[0].GetVar<Vec3>();

    Vec3 corners[8]
    {
      Vec3(-0.5f, 0.5f, 0.5f) * scale, // FTL.
      Vec3(-0.5f, -0.5f, 0.5f) * scale, // FBL.
      Vec3(0.5f, -0.5f, 0.5f) * scale, // FBR.
      Vec3(0.5f, 0.5f, 0.5f) * scale, // FTR.
      Vec3(-0.5f, 0.5f, -0.5f) * scale, // BTL.
      Vec3(-0.5f, -0.5f, -0.5f) * scale, // BBL.
      Vec3(0.5f, -0.5f, -0.5f) * scale, // BBR.
      Vec3(0.5f, 0.5f, -0.5f) * scale, // BTR.
    };

    // Front
    vertices[0].pos = corners[0];
    vertices[0].tex = Vec2(0.0f, 1.0f);
    vertices[0].norm = Vec3(0.0f, 0.0f, 1.0f);
    vertices[1].pos = corners[1];
    vertices[1].tex = Vec2(0.0f, 0.0f);
    vertices[1].norm = Vec3(0.0f, 0.0f, 1.0f);
    vertices[2].pos = corners[2];
    vertices[2].tex = Vec2(1.0f, 0.0f);
    vertices[2].norm = Vec3(0.0f, 0.0f, 1.0f);

    vertices[3].pos = corners[0];
    vertices[3].tex = Vec2(0.0f, 1.0f);
    vertices[3].norm = Vec3(0.0f, 0.0f, 1.0f);
    vertices[4].pos = corners[2];
    vertices[4].tex = Vec2(1.0f, 0.0f);
    vertices[4].norm = Vec3(0.0f, 0.0f, 1.0f);
    vertices[5].pos = corners[3];
    vertices[5].tex = Vec2(1.0f, 1.0f);
    vertices[5].norm = Vec3(0.0f, 0.0f, 1.0f);

    // Right
    vertices[6].pos = corners[3];
    vertices[6].tex = Vec2(0.0f, 1.0f);
    vertices[6].norm = Vec3(1.0f, 0.0f, 0.0f);
    vertices[7].pos = corners[2];
    vertices[7].tex = Vec2(0.0f, 0.0f);
    vertices[7].norm = Vec3(1.0f, 0.0f, 0.0f);
    vertices[8].pos = corners[6];
    vertices[8].tex = Vec2(1.0f, 0.0f);
    vertices[8].norm = Vec3(1.0f, 0.0f, 0.0f);

    vertices[9].pos = corners[3];
    vertices[9].tex = Vec2(0.0f, 1.0f);
    vertices[9].norm = Vec3(1.0f, 0.0f, 0.0f);
    vertices[10].pos = corners[6];
    vertices[10].tex = Vec2(1.0f, 0.0f);
    vertices[10].norm = Vec3(1.0f, 0.0f, 0.0f);
    vertices[11].pos = corners[7];
    vertices[11].tex = Vec2(1.0f, 1.0f);
    vertices[11].norm = Vec3(1.0f, 0.0f, 0.0f);

    // Top
    vertices[12].pos = corners[0];
    vertices[12].tex = Vec2(0.0f, 0.0f);
    vertices[12].norm = Vec3(0.0f, 1.0f, 0.0f);
    vertices[13].pos = corners[3];
    vertices[13].tex = Vec2(1.0f, 0.0f);
    vertices[13].norm = Vec3(0.0f, 1.0f, 0.0f);
    vertices[14].pos = corners[7];
    vertices[14].tex = Vec2(1.0f, 1.0f);
    vertices[14].norm = Vec3(0.0f, 1.0f, 0.0f);

    vertices[15].pos = corners[0];
    vertices[15].tex = Vec2(0.0f, 0.0f);
    vertices[15].norm = Vec3(0.0f, 1.0f, 0.0f);
    vertices[16].pos = corners[7];
    vertices[16].tex = Vec2(1.0f, 1.0f);
    vertices[16].norm = Vec3(0.0f, 1.0f, 0.0f);
    vertices[17].pos = corners[4];
    vertices[17].tex = Vec2(0.0f, 1.0f);
    vertices[17].norm = Vec3(0.0f, 1.0f, 0.0f);

    // Back
    vertices[18].pos = corners[4];
    vertices[18].tex = Vec2(0.0f, 1.0f);
    vertices[18].norm = Vec3(0.0f, 0.0f, -1.0f);
    vertices[19].pos = corners[6];
    vertices[19].tex = Vec2(1.0f, 0.0f);
    vertices[19].norm = Vec3(0.0f, 0.0f, -1.0f);
    vertices[20].pos = corners[5];
    vertices[20].tex = Vec2(0.0f, 0.0f);
    vertices[20].norm = Vec3(0.0f, 0.0f, -1.0f);

    vertices[21].pos = corners[4];
    vertices[21].tex = Vec2(0.0f, 1.0f);
    vertices[21].norm = Vec3(0.0f, 0.0f, -1.0f);
    vertices[22].pos = corners[7];
    vertices[22].tex = Vec2(1.0f, 1.0f);
    vertices[22].norm = Vec3(0.0f, 0.0f, -1.0f);
    vertices[23].pos = corners[6];
    vertices[23].tex = Vec2(1.0f, 0.0f);
    vertices[23].norm = Vec3(0.0f, 0.0f, -1.0f);

    // Left
    vertices[24].pos = corners[0];
    vertices[24].tex = Vec2(0.0f, 1.0f);
    vertices[24].norm = Vec3(-1.0f, 0.0f, 0.0f);
    vertices[25].pos = corners[5];
    vertices[25].tex = Vec2(1.0f, 0.0f);
    vertices[25].norm = Vec3(-1.0f, 0.0f, 0.0f);
    vertices[26].pos = corners[1];
    vertices[26].tex = Vec2(0.0f, 0.0f);
    vertices[26].norm = Vec3(-1.0f, 0.0f, 0.0f);

    vertices[27].pos = corners[0];
    vertices[27].tex = Vec2(0.0f, 1.0f);
    vertices[27].norm = Vec3(-1.0f, 0.0f, 0.0f);
    vertices[28].pos = corners[4];
    vertices[28].tex = Vec2(1.0f, 1.0f);
    vertices[28].norm = Vec3(-1.0f, 0.0f, 0.0f);
    vertices[29].pos = corners[5];
    vertices[29].tex = Vec2(1.0f, 0.0f);
    vertices[29].norm = Vec3(-1.0f, 0.0f, 0.0f);

    // Bottom
    vertices[30].pos = corners[1];
    vertices[30].tex = Vec2(0.0f, 1.0f);
    vertices[30].norm = Vec3(0.0f, -1.0f, 0.0f);
    vertices[31].pos = corners[6];
    vertices[31].tex = Vec2(1.0f, 0.0f);
    vertices[31].norm = Vec3(0.0f, -1.0f, 0.0f);
    vertices[32].pos = corners[2];
    vertices[32].tex = Vec2(0.0f, 0.0f);
    vertices[32].norm = Vec3(0.0f, -1.0f, 0.0f);

    vertices[33].pos = corners[1];
    vertices[33].tex = Vec2(0.0f, 1.0f);
    vertices[33].norm = Vec3(0.0f, -1.0f, 0.0f);
    vertices[34].pos = corners[5];
    vertices[34].tex = Vec2(1.0f, 1.0f);
    vertices[34].norm = Vec3(0.0f, -1.0f, 0.0f);
    vertices[35].pos = corners[6];
    vertices[35].tex = Vec2(1.0f, 0.0f);
    vertices[35].norm = Vec3(0.0f, -1.0f, 0.0f);

    MeshPtr mesh = GetMesh();
    mesh->m_vertexCount = (uint)vertices.size();
    mesh->m_clientSideVertices = vertices;
    mesh->m_clientSideIndices = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35 };
    mesh->m_indexCount = (uint)mesh->m_clientSideIndices.size();
    mesh->m_material = GetMaterialManager()->GetCopyOfDefaultMaterial();

    mesh->CalculateAABB();
    mesh->ConstructFaces();
  }

  Quad::Quad(bool genDef)
  {
    if (genDef)
    {
      Generate();
    }
  }

  Entity* Quad::GetCopy(Entity* copyTo) const
  {
    return Drawable::GetCopy(copyTo);
  }

  EntityType Quad::GetType() const
  {
    return EntityType::Entity_Quad;
  }

  Entity* Quad::GetInstance(Entity* copyTo) const
  {
    Drawable::GetInstance(copyTo);
    Quad* instance = static_cast<Quad*> (copyTo);

    return instance;
  }

  void Quad::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    Entity::Serialize(doc, parent);
  }

  void Quad::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Entity::DeSerialize(doc, parent);
    Generate();
  }

  void Quad::Generate()
  {
    VertexArray vertices;
    vertices.resize(4);

    // Front
    vertices[0].pos = Vec3(-0.5f, 0.5f, 0.0f);
    vertices[0].tex = Vec2(0.0f, 1.0f);
    vertices[0].norm = Vec3(0.0f, 0.0f, 1.0f);
    vertices[0].btan = Vec3(0.0f, 1.0f, 0.0f);

    vertices[1].pos = Vec3(-0.5f, -0.5f, 0.0f);
    vertices[1].tex = Vec2(0.0f, 0.0f);
    vertices[1].norm = Vec3(0.0f, 0.0f, 1.0f);
    vertices[1].btan = Vec3(0.0f, 1.0f, 0.0f);

    vertices[2].pos = Vec3(0.5f, -0.5f, 0.0f);
    vertices[2].tex = Vec2(1.0f, 0.0f);
    vertices[2].norm = Vec3(0.0f, 0.0f, 1.0f);
    vertices[2].btan = Vec3(0.0f, 1.0f, 0.0f);

    vertices[3].pos = Vec3(0.5f, 0.5f, 0.0f);
    vertices[3].tex = Vec2(1.0f, 1.0f);
    vertices[3].norm = Vec3(0.0f, 0.0f, 1.0f);
    vertices[3].btan = Vec3(0.0f, 1.0f, 0.0f);

    MeshPtr mesh = GetMesh();
    mesh->m_vertexCount = (uint)vertices.size();
    mesh->m_clientSideVertices = vertices;
    mesh->m_indexCount = 6;
    mesh->m_clientSideIndices = { 0,1,2,0,2,3 };
    mesh->m_material = GetMaterialManager()->GetCopyOfDefaultMaterial();

    mesh->CalculateAABB();
    mesh->ConstructFaces();
  }

  Sphere::Sphere(bool genDef)
  {
    if (genDef)
    {
      Generate();
    }
  }

  Sphere::Sphere(const Params& params)
    : m_params(params)
  {
    Generate();
  }

  Entity* Sphere::GetCopy(Entity* copyTo) const
  {
    Drawable::GetCopy(copyTo);
    Sphere* ntt = static_cast<Sphere*> (copyTo);
    ntt->m_params = m_params;
    return ntt;
  }

  EntityType Sphere::GetType() const
  {
    return EntityType::Entity_Sphere;
  }

  void Sphere::Generate()
  {
    const float r = m_params[0].GetVar<float>();
    const int nRings = 32;
    const int nSegments = 32;

    VertexArray vertices;
    std::vector<uint> indices;

    constexpr float fDeltaRingAngle = (glm::pi<float>() / nRings);
    constexpr float fDeltaSegAngle = (glm::two_pi<float>() / nSegments);
    unsigned short wVerticeIndex = 0;

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
        v.pos = Vec3(x0, y0, z0);
        v.norm = Vec3(x0, y0, z0);
        v.tex = Vec2((float)seg / (float)nSegments, (float)ring / (float)nRings);

        float r2, zenith, azimuth;
        ToSpherical(v.pos, r2, zenith, azimuth);
        v.btan = Vec3(r * glm::cos(zenith) * glm::sin(azimuth), -r * glm::sin(zenith), r * glm::cos(zenith) * glm::cos(azimuth));

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
    } // end for ring

    MeshPtr mesh = GetMesh();
    mesh->m_vertexCount = (uint)vertices.size();
    mesh->m_clientSideVertices = vertices;
    mesh->m_indexCount = (uint)indices.size();
    mesh->m_clientSideIndices = indices;
    mesh->m_material = GetMaterialManager()->GetCopyOfDefaultMaterial();

    mesh->CalculateAABB();
    mesh->ConstructFaces();
  }

  void Sphere::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    Entity::Serialize(doc, parent);
    m_params.Serialize(doc, parent->last_node());
  }

  void Sphere::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Entity::DeSerialize(doc, parent);
    m_params.DeSerialize(doc, parent);
    Generate();
  }

  Entity* Sphere::GetInstance(Entity* copyTo) const
  {
    Drawable::GetInstance(copyTo);
    Sphere* instance = static_cast<Sphere*> (copyTo);
    instance->m_params = m_params;
    return instance;
  }

  Cone::Cone(bool genDef)
  {
    if (genDef)
    {
      Generate();
    }
  }

  Cone::Cone(const Params& params)
    : m_params(params)
  {
    Generate();
  }

  // https://github.com/OGRECave/ogre-procedural/blob/master/library/src/ProceduralConeGenerator.cpp
  void Cone::Generate()
  {
    VertexArray vertices;
    std::vector<uint> indices;

    float height = m_params[0].GetVar<float>();
    float radius = m_params[1].GetVar<float>();
    int nSegBase = m_params[2].GetVar<int>();
    int nSegHeight = m_params[3].GetVar<int>();

    float deltaAngle = (glm::two_pi<float>() / nSegBase);
    float deltaHeight = height / nSegHeight;
    int offset = 0;

    Vec3 refNormal = glm::normalize(Vec3(radius, height, 0.0f));
    Quaternion q;

    for (int i = 0; i <= nSegHeight; i++)
    {
      float r0 = radius * (1 - i / (float)nSegHeight);
      for (int j = 0; j <= nSegBase; j++)
      {
        float x0 = r0 * glm::cos(j * deltaAngle);
        float z0 = r0 * glm::sin(j * deltaAngle);

        q = glm::angleAxis(glm::radians(-deltaAngle * j), Y_AXIS);

        Vertex v
        {
          Vec3(x0, i * deltaHeight, z0),
          q * refNormal,
          Vec2(j / (float)nSegBase, i / (float)nSegHeight),
          Vec3() // btan missing.
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

    //low cap
    int centerIndex = offset;

    Vertex v
    {
      Vec3(),
      -Y_AXIS,
      Y_AXIS,
      Vec3() // btan missing.
    };
    vertices.push_back(v);

    offset++;
    for (int j = 0; j <= nSegBase; j++)
    {
      float x0 = radius * glm::cos(j * deltaAngle);
      float z0 = radius * glm::sin(j * deltaAngle);

      Vertex v
      {
        Vec3(x0, 0.0f, z0),
        -Y_AXIS,
        Vec2(j / (float)nSegBase, 0.0f),
        Vec3() // btan missing.
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

    MeshPtr mesh = GetMesh();
    mesh->m_vertexCount = (uint)vertices.size();
    mesh->m_clientSideVertices = vertices;
    mesh->m_indexCount = (uint)indices.size();
    mesh->m_clientSideIndices = indices;
    mesh->m_material = GetMaterialManager()->GetCopyOfDefaultMaterial();

    mesh->CalculateAABB();
    mesh->ConstructFaces();
  }

  Entity* Cone::GetCopy(Entity* copyTo) const
  {
    Drawable::GetCopy(copyTo);
    Cone* ntt = static_cast<Cone*> (copyTo);
    ntt->m_params = m_params;
    return ntt;
  }

  EntityType Cone::GetType() const
  {
    return EntityType::Entity_Cone;
  }

  void Cone::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    Entity::Serialize(doc, parent);
    m_params.Serialize(doc, parent->last_node());
  }

  void Cone::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Entity::DeSerialize(doc, parent);
    m_params.DeSerialize(doc, parent);
    Generate();
  }

  Entity* Cone::GetInstance(Entity* copyTo) const
  {
    Drawable::GetInstance(copyTo);
    Cone* instance = static_cast<Cone*> (copyTo);
    instance->m_params = m_params;
    return instance;
  }

  Arrow2d::Arrow2d(bool genDef)
  {
    m_label = AxisLabel::X;

    if (genDef)
    {
      Generate();
    }
  }

  Arrow2d::Arrow2d(AxisLabel label)
    : m_label(label)
  {
    Generate();
  }

  Entity* Arrow2d::GetCopy(Entity* copyTo) const
  {
    Drawable::GetCopy(copyTo);
    Arrow2d* ntt = static_cast<Arrow2d*> (copyTo);
    ntt->m_label = m_label;

    return ntt;
  }

  Entity* Arrow2d::GetInstance(Entity* copyTo) const
  {
    Drawable::GetInstance(copyTo);
    Arrow2d* instance = static_cast<Arrow2d*> (copyTo);
    instance->m_label = m_label;
    return instance;
  }

  EntityType Arrow2d::GetType() const
  {
    return EntityType::Etity_Arrow;
  }

  void Arrow2d::Generate()
  {
    VertexArray vertices;
    vertices.resize(8);

    // Line
    vertices[0].pos = Vec3(0.0f, 0.0f, 0.0f);
    vertices[1].pos = Vec3(0.8f, 0.0f, 0.0f);

    // Triangle
    vertices[2].pos = Vec3(0.8f, -0.2f, 0.0f);
    vertices[3].pos = Vec3(0.8f, 0.2f, 0.0f);
    vertices[4].pos = Vec3(0.8f, 0.2f, 0.0f);
    vertices[5].pos = Vec3(1.0f, 0.0f, 0.0f);
    vertices[6].pos = Vec3(1.0f, 0.0f, 0.0f);
    vertices[7].pos = Vec3(0.8f, -0.2f, 0.0f);

    MaterialPtr newMaterial = GetMaterialManager()->GetCopyOfUnlitColorMaterial();
    newMaterial->GetRenderState()->drawType = DrawType::Line;
    newMaterial->m_color = Vec3(0.89f, 0.239f, 0.341f);

    Quaternion rotation;
    if (m_label == AxisLabel::Y)
    {
      newMaterial->m_color = Vec3(0.537f, 0.831f, 0.07f);
      rotation = glm::angleAxis(glm::half_pi<float>(), Z_AXIS);
    }

    if (m_label == AxisLabel::Z)
    {
      newMaterial->m_color = Vec3(0.196f, 0.541f, 0.905f);
      rotation = glm::angleAxis(-glm::half_pi<float>(), Y_AXIS);
    }

    for (size_t i = 0; i < vertices.size(); i++)
    {
      vertices[i].pos = rotation * vertices[i].pos;
    }

    MeshPtr mesh = GetMesh();
    mesh->m_vertexCount = (uint)vertices.size();
    mesh->m_clientSideVertices = vertices;
    mesh->m_material = newMaterial;

    mesh->CalculateAABB();
    mesh->ConstructFaces();
  }

  LineBatch::LineBatch(const Vec3Array& linePnts, const Vec3& color, DrawType t, float lineWidth)
  {
    Generate(linePnts, color, t, lineWidth);
  }

  LineBatch::LineBatch()
  {
  }

  Entity* LineBatch::GetCopy(Entity* copyTo) const
  {
    return Drawable::GetCopy(copyTo);
  }

  EntityType LineBatch::GetType() const
  {
    return EntityType::Entity_LineBatch;
  }

  void LineBatch::Generate(const Vec3Array& linePnts, const Vec3& color, DrawType t, float lineWidth)
  {
    VertexArray vertices;
    vertices.resize(linePnts.size());

    MeshPtr mesh = GetMesh();
    mesh->UnInit();
    mesh->m_material = GetMaterialManager()->GetCopyOfUnlitColorMaterial();
    mesh->m_material->GetRenderState()->drawType = t;

    for (size_t i = 0; i < linePnts.size(); i++)
    {
      vertices[i].pos = linePnts[i];
    }

    mesh->m_vertexCount = (uint)vertices.size();
    mesh->m_clientSideVertices = vertices;
    mesh->m_material->m_color = color;
    mesh->m_material->GetRenderState()->lineWidth = lineWidth;

    mesh->CalculateAABB();
  }

}
