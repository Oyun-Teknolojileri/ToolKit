#include "stdafx.h"
#include "Primative.h"
#include "Mesh.h"
#include "ToolKit.h"
#include "MathUtil.h"
#include "Directional.h"
#include "Node.h"
#include "DebugNew.h"

ToolKit::Billboard::Billboard(const Settings& settings)
	: m_settings(settings)
{
}

ToolKit::Billboard::~Billboard()
{
}

ToolKit::EntityType ToolKit::Billboard::GetType() const
{
  return EntityType::Entity_Billboard;
}

void ToolKit::Billboard::LookAt(Camera* cam)
{
	// SetTranslation in given space is not provided in Node class.
	// Therefore all objects must be in the worldSpace.
	assert(m_node->m_parent == nullptr);
	assert(cam->m_node->m_parent == nullptr);

	Camera::CamData data = cam->GetData();

	// Billboard placement.
	if (m_settings.distanceToCamera > 0.0f)
	{
		glm::vec3 cdir, dir;
		cam->GetLocalAxis(cdir, dir, dir);
		dir = glm::normalize(m_worldLocation - cam->m_node->m_translation);

		float radialToPlanarDistance = 1.0f / glm::dot(cdir, dir); // Always place at the same distance from the near plane.
		if (radialToPlanarDistance < 0)
		{
			return;
		}

		m_node->m_translation = cam->m_node->m_translation + dir * m_settings.distanceToCamera * radialToPlanarDistance;
	}

	if (m_settings.heightInScreenSpace > 0.0f)
	{
		m_node->m_scale = glm::vec3(m_settings.heightInScreenSpace / data.height); // Compensate shrinkage due to height changes.
	}

	if (m_settings.lookAtCamera)
	{
		m_node->m_orientation = cam->m_node->m_orientation;
	}
}

ToolKit::Cube::Cube()
{
  std::vector<ToolKit::Vertex> vertices;
  vertices.resize(36);

  // Front
  vertices[0].pos = glm::vec3(-0.5f, 0.5f, 0.5f);
  vertices[0].tex = glm::vec2(0.0f, 1.0f);
  vertices[1].pos = glm::vec3(-0.5f, -0.5f, 0.5f);
  vertices[1].tex = glm::vec2(0.0f, 0.0f);
  vertices[2].pos = glm::vec3(0.5f, -0.5f, 0.5f);
  vertices[2].tex = glm::vec2(1.0f, 0.0f);

  vertices[3].pos = glm::vec3(-0.5f, 0.5f, 0.5f);
  vertices[3].tex = glm::vec2(0.0f, 1.0f);
  vertices[4].pos = glm::vec3(0.5f, -0.5f, 0.5f);
  vertices[4].tex = glm::vec2(1.0f, 0.0f);
  vertices[5].pos = glm::vec3(0.5f, 0.5f, 0.5f);
  vertices[5].tex = glm::vec2(1.0f, 1.0f);

  // Right
  vertices[6].pos = glm::vec3(0.5f, 0.5f, 0.5f);
  vertices[6].tex = glm::vec2(0.0f, 1.0f);
  vertices[7].pos = glm::vec3(0.5f, -0.5f, 0.5f);
  vertices[7].tex = glm::vec2(0.0f, 0.0f);
  vertices[8].pos = glm::vec3(0.5f, -0.5f, -0.5f);
  vertices[8].tex = glm::vec2(1.0f, 0.0f);

  vertices[9].pos = glm::vec3(0.5f, 0.5f, 0.5f);
  vertices[9].tex = glm::vec2(0.0f, 1.0f);
  vertices[10].pos = glm::vec3(0.5f, -0.5f, -0.5f);
  vertices[10].tex = glm::vec2(1.0f, 0.0f);
  vertices[11].pos = glm::vec3(0.5f, 0.5f, -0.5f);
  vertices[11].tex = glm::vec2(1.0f, 1.0f);

  // Top
  vertices[12].pos = glm::vec3(-0.5f, 0.5f, 0.5f);
  vertices[12].tex = glm::vec2(0.0f, 0.0f);
  vertices[13].pos = glm::vec3(0.5f, 0.5f, 0.5f);
  vertices[13].tex = glm::vec2(1.0f, 0.0f);
  vertices[14].pos = glm::vec3(0.5f, 0.5f, -0.5f);
  vertices[14].tex = glm::vec2(1.0f, 1.0f);

  vertices[15].pos = glm::vec3(-0.5f, 0.5f, 0.5f);
  vertices[15].tex = glm::vec2(0.0f, 0.0f);
  vertices[16].pos = glm::vec3(0.5f, 0.5f, -0.5f);
  vertices[16].tex = glm::vec2(1.0f, 1.0f);
  vertices[17].pos = glm::vec3(-0.5f, 0.5f, -0.5f);
  vertices[17].tex = glm::vec2(0.0f, 1.0f);

  // Back
  vertices[18].pos = glm::vec3(-0.5f, 0.5f, -0.5f);
  vertices[18].tex = glm::vec2(0.0f, 1.0f);
  vertices[19].pos = glm::vec3(0.5f, -0.5f, -0.5f);
  vertices[19].tex = glm::vec2(1.0f, 0.0f);
  vertices[20].pos = glm::vec3(-0.5f, -0.5f, -0.5f);
  vertices[20].tex = glm::vec2(0.0f, 0.0f);

  vertices[21].pos = glm::vec3(-0.5f, 0.5f, -0.5f);
  vertices[21].tex = glm::vec2(0.0f, 1.0f);
  vertices[22].pos = glm::vec3(0.5f, 0.5f, -0.5f);
  vertices[22].tex = glm::vec2(1.0f, 1.0f);
  vertices[23].pos = glm::vec3(0.5f, -0.5f, -0.5f);
  vertices[23].tex = glm::vec2(1.0f, 0.0f);

  // Left
  vertices[24].pos = glm::vec3(-0.5f, 0.5f, 0.5f);
  vertices[24].tex = glm::vec2(0.0f, 1.0f);
  vertices[25].pos = glm::vec3(-0.5f, -0.5f, -0.5f);
  vertices[25].tex = glm::vec2(1.0f, 0.0f);
  vertices[26].pos = glm::vec3(-0.5f, -0.5f, 0.5f);
  vertices[26].tex = glm::vec2(0.0f, 0.0f);

  vertices[27].pos = glm::vec3(-0.5f, 0.5f, 0.5f);
  vertices[27].tex = glm::vec2(0.0f, 1.0f);
  vertices[28].pos = glm::vec3(-0.5f, 0.5f, -0.5f);
  vertices[28].tex = glm::vec2(1.0f, 1.0f);
  vertices[29].pos = glm::vec3(-0.5f, -0.5f, -0.5f);
  vertices[29].tex = glm::vec2(1.0f, 0.0f);

  // Bottom
  vertices[30].pos = glm::vec3(-0.5f, -0.5f, 0.5f);
  vertices[30].tex = glm::vec2(0.0f, 1.0f);
  vertices[31].pos = glm::vec3(0.5f, -0.5f, -0.5f);
  vertices[31].tex = glm::vec2(1.0f, 0.0f);
  vertices[32].pos = glm::vec3(0.5f, -0.5f, 0.5f);
  vertices[32].tex = glm::vec2(0.0f, 0.0f);

  vertices[33].pos = glm::vec3(-0.5f, -0.5f, 0.5f);
  vertices[33].tex = glm::vec2(0.0f, 1.0f);
  vertices[34].pos = glm::vec3(-0.5f, -0.5f, -0.5f);
  vertices[34].tex = glm::vec2(1.0f, 1.0f);
  vertices[35].pos = glm::vec3(0.5f, -0.5f, -0.5f);
  vertices[35].tex = glm::vec2(1.0f, 0.0f);

  m_mesh->m_vertexCount = (uint)vertices.size();
  m_mesh->m_clientSideVertices = vertices;
  m_mesh->m_clientSideIndices = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35 };
  m_mesh->m_indexCount = (uint)m_mesh->m_clientSideIndices.size();
  m_mesh->m_material = Main::GetInstance()->m_materialManager.Create(MaterialPath("default.material"));

	m_mesh->CalculateAABoundingBox();
}

ToolKit::Cube::~Cube()
{
}

ToolKit::EntityType ToolKit::Cube::GetType() const
{
  return EntityType::Entity_Cube;
}

ToolKit::Quad::Quad()
{
  std::vector<ToolKit::Vertex> vertices;
  vertices.resize(4);

  // Front
  vertices[0].pos = glm::vec3(-0.5f, 0.5f, 0.0f);
  vertices[0].tex = glm::vec2(0.0f, 1.0f);
  vertices[0].norm = glm::vec3(0.0f, 0.0f, 1.0f);
  vertices[0].btan = glm::vec3(0.0f, 1.0f, 0.0f);

  vertices[1].pos = glm::vec3(-0.5f, -0.5f, 0.0f);
  vertices[1].tex = glm::vec2(0.0f, 0.0f);
  vertices[1].norm = glm::vec3(0.0f, 0.0f, 1.0f);
  vertices[1].btan = glm::vec3(0.0f, 1.0f, 0.0f);

  vertices[2].pos = glm::vec3(0.5f, -0.5f, 0.0f);
  vertices[2].tex = glm::vec2(1.0f, 0.0f);
  vertices[2].norm = glm::vec3(0.0f, 0.0f, 1.0f);
  vertices[2].btan = glm::vec3(0.0f, 1.0f, 0.0f);

  vertices[3].pos = glm::vec3(0.5f, 0.5f, 0.0f);
  vertices[3].tex = glm::vec2(1.0f, 1.0f);
  vertices[3].norm = glm::vec3(0.0f, 0.0f, 1.0f);
  vertices[3].btan = glm::vec3(0.0f, 1.0f, 0.0f);

  m_mesh->m_vertexCount = (uint)vertices.size();
  m_mesh->m_clientSideVertices = vertices;
  m_mesh->m_indexCount = 6;
  m_mesh->m_clientSideIndices = { 0,1,2,0,2,3 };
  m_mesh->m_material = Main::GetInstance()->m_materialManager.Create(MaterialPath("default.material"));

	m_mesh->CalculateAABoundingBox();
}

ToolKit::Quad::~Quad()
{
}

ToolKit::EntityType ToolKit::Quad::GetType() const
{
  return EntityType::Entity_Quad;
}

ToolKit::Sphere::Sphere()
{
  const float r = 1.0f;
  const int nRings = 32;
  const int nSegments = 32;

  std::vector<Vertex> vertices;
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
      v.pos = glm::vec3(x0, y0, z0);
      v.norm = glm::vec3(x0, y0, z0);
      v.tex = glm::vec2((float)seg / (float)nSegments, (float)ring / (float)nRings);

      float r2, zenith, azimuth;
      ToSpherical(v.pos, r2, zenith, azimuth);
      v.btan = glm::vec3(r * glm::cos(zenith) * glm::sin(azimuth), -r * glm::sin(zenith), r * glm::cos(zenith) * glm::cos(azimuth));

      vertices.push_back(v);

      if (ring != nRings) 
      {
        // each vertex (except the last) has six indicies pointing to it
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

  m_mesh->m_vertexCount = (uint)vertices.size();
  m_mesh->m_clientSideVertices = vertices;
  m_mesh->m_indexCount = (uint)indices.size();
  m_mesh->m_clientSideIndices = indices;
  m_mesh->m_material = Main::GetInstance()->m_materialManager.Create(MaterialPath("default.material"));

	m_mesh->CalculateAABoundingBox();
}

  ToolKit::Sphere::~Sphere()
{
}

ToolKit::EntityType ToolKit::Sphere::GetType() const
{
  return EntityType::Entity_Sphere;
}

ToolKit::Arrow2d::Arrow2d()
{
	Generate(m_arrowType);
}

ToolKit::Arrow2d::Arrow2d(ArrowType t)
	: m_arrowType(t)
{
	Generate(m_arrowType);
}

ToolKit::Arrow2d::~Arrow2d()
{
}

ToolKit::EntityType ToolKit::Arrow2d::GetType() const
{
  return EntityType::Etity_Arrow;
}

void ToolKit::Arrow2d::Generate(ToolKit::Arrow2d::ArrowType t)
{
	std::vector<ToolKit::Vertex> vertices;
	vertices.resize(8);

	// Line
	vertices[0].pos = glm::vec3(0.0f, 0.0f, 0.0f);
	vertices[1].pos = glm::vec3(0.8f, 0.0f, 0.0f);

	// Triangle
	vertices[2].pos = glm::vec3(0.8f, -0.2f, 0.0f);
	vertices[3].pos = glm::vec3(0.8f, 0.2f, 0.0f);
	vertices[4].pos = glm::vec3(0.8f, 0.2f, 0.0f);
	vertices[5].pos = glm::vec3(1.0f, 0.0f, 0.0f);
	vertices[6].pos = glm::vec3(1.0f, 0.0f, 0.0f);
	vertices[7].pos = glm::vec3(0.8f, -0.2f, 0.0f);

	Material* newMaterial = Main::GetInstance()->m_materialManager.Create(MaterialPath("LineColor.material"))->GetCopy();
	newMaterial->m_color = glm::vec3(0.89f, 0.239f, 0.341f);

	glm::quat rotation;
	if (m_arrowType == ArrowType::Y)
	{
		newMaterial->m_color = glm::vec3(0.537f, 0.831f, 0.07f);
		rotation = glm::angleAxis(glm::half_pi<float>(), Z_AXIS);
	}

	if (m_arrowType == ArrowType::Z)
	{
		newMaterial->m_color = glm::vec3(0.196f, 0.541f, 0.905f);
		rotation = glm::angleAxis(-glm::half_pi<float>(), Y_AXIS);
	}

	for (size_t i = 0; i < vertices.size(); i++)
	{
		vertices[i].pos = rotation * vertices[i].pos;
	}

  m_mesh->m_vertexCount = (uint)vertices.size();
	m_mesh->m_clientSideVertices = vertices;
	m_mesh->m_material = std::shared_ptr<Material>(newMaterial);

	m_mesh->CalculateAABoundingBox();
}

ToolKit::LineBatch::LineBatch(const std::vector<glm::vec3>& linePnts, glm::vec3 color, DrawType t)
{
	Material* newMaterial = Main::GetInstance()->m_materialManager.Create(MaterialPath("LineColor.material"))->GetCopy();
	m_mesh->m_material = std::shared_ptr<Material>(newMaterial);

  Generate(linePnts, color, t);
}

ToolKit::LineBatch::~LineBatch()
{
}

ToolKit::EntityType ToolKit::LineBatch::GetType() const
{
  return EntityType::Entity_LineBatch;
}

void ToolKit::LineBatch::Generate(const std::vector<glm::vec3>& linePnts, glm::vec3 color, DrawType t)
{
	std::vector<ToolKit::Vertex> vertices;
	vertices.resize(linePnts.size());

  m_mesh->UnInit();

	for (size_t i = 0; i < linePnts.size(); i++)
	{
		vertices[i].pos = linePnts[i];
	}

	m_mesh->m_vertexCount = (uint)vertices.size();
	m_mesh->m_clientSideVertices = vertices;
	m_mesh->m_material->m_color = color;
  m_mesh->m_material->GetRenderState()->drawType = t;

	m_mesh->CalculateAABoundingBox();
}
