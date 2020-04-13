#include "stdafx.h"

#include "Gizmo.h"
#include "ToolKit.h"
#include "Node.h"
#include "Surface.h"
#include "Directional.h"
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"
#include "RenderState.h"
#include "Material.h"
#include "Primative.h"
#include "GlobalDef.h"
#include "Viewport.h"
#include "DebugNew.h"

namespace ToolKit
{
	namespace Editor
	{

		Cursor::Cursor()
			: Billboard({ true, 10.0f, 400.0f })
		{
			Generate();
		}

		void Cursor::Generate()
		{
			m_mesh->UnInit();

			// Billboard
			Quad quad;
			std::shared_ptr<Mesh> meshPtr = quad.m_mesh;

			meshPtr->m_material = MaterialPtr(meshPtr->m_material->GetCopy());
			meshPtr->m_material->UnInit();
			meshPtr->m_material->m_diffuseTexture = Main::GetInstance()->m_textureMan.Create(TexturePath("Icons/cursor4k.png"));
			meshPtr->m_material->GetRenderState()->blendFunction = BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA;
			meshPtr->m_material->Init();

			meshPtr->m_material->GetRenderState()->depthTestEnabled = false;
			m_mesh->m_subMeshes.push_back(meshPtr);

			// Lines
			std::vector<Vertex> vertices;
			vertices.resize(12);

			vertices[0].pos.z = -0.3f;
			vertices[1].pos.z = -0.7f;

			vertices[2].pos.z = 0.3f;
			vertices[3].pos.z = 0.7f;

			vertices[4].pos.x = 0.3f;
			vertices[5].pos.x = 0.7f;

			vertices[6].pos.x = -0.3f;
			vertices[7].pos.x = -0.7f;

			vertices[8].pos.y = 0.3f;
			vertices[9].pos.y = 0.7f;

			vertices[10].pos.y = -0.3f;
			vertices[11].pos.y = -0.7f;

			MaterialPtr newMaterial = GetMaterialManager()->GetCopyOfSolidMaterial();
			newMaterial->m_color = Vec3(0.1f, 0.1f, 0.1f);
			newMaterial->GetRenderState()->drawType = DrawType::Line;
			newMaterial->GetRenderState()->depthTestEnabled = false;

			m_mesh->m_clientSideVertices = vertices;
			m_mesh->m_material = newMaterial;

			m_mesh->CalculateAABoundingBox();
		}

		Axis3d::Axis3d()
			: Billboard({ false, 10.0f, 400.0f })
		{
			Generate();
		}

		void Axis3d::Generate()
		{
			for (int i = 0; i < 3; i++)
			{
				AxisLabel t;
				switch (i)
				{
				case 0:
					t = AxisLabel::X;
					break;
				case 1:
					t = AxisLabel::Y;
					break;
				case 2:
					t = AxisLabel::Z;
					break;
				}

				Arrow2d arrow(t);
				arrow.m_mesh->m_material->GetRenderState()->depthTestEnabled = false;
				if (i == 0)
				{
					m_mesh = arrow.m_mesh;
				}
				else
				{
					m_mesh->m_subMeshes.push_back(arrow.m_mesh);
				}
			}
		}

		MoveGizmo::MoveGizmo()
			: Billboard({ false, 10.0f, 400.0f })
		{
			m_inAccessable = AxisLabel::None;
			m_grabbed = AxisLabel::None;

			// Mesh.
			Generate();
		}

		AxisLabel MoveGizmo::HitTest(const Ray& ray)
		{
			Mat4 invMat = m_node->GetTransform(TransformationSpace::TS_WORLD);
			Mat4 tpsMat = glm::transpose(invMat);
			invMat = glm::inverse(invMat);

			Ray rayInObjectSpace = ray;
			rayInObjectSpace.position = invMat * Vec4(rayInObjectSpace.position, 1.0f);
			rayInObjectSpace.direction = tpsMat * Vec4(rayInObjectSpace.direction, 1.0f);

			float d, t = std::numeric_limits<float>::infinity();
			int minIndx = -1;
			for (int i = 0; i < 3; i++)
			{
				bool line = RayBoxIntersection(rayInObjectSpace, m_hitBox[i], d);
				bool cone = RayBoxIntersection(rayInObjectSpace, m_hitBox[i + 3], d);
				if (line || cone)
				{
					if (d < t)
					{
						t = d;
						minIndx = i;
					}
				}
			}

			AxisLabel hit = (AxisLabel)minIndx;
			if (m_grabbed != AxisLabel::None)
			{
				hit = m_grabbed; // If grabbed, always highlight.
			}

			return hit;
		}

		void MoveGizmo::Update(float deltaTime)
		{
			Viewport* vp = g_app->GetActiveViewport();
			if (vp == nullptr)
			{
				return;
			}

			std::vector<Mesh*> allMeshes;
			m_mesh->GetAllMeshes(allMeshes);
			assert(allMeshes.size() <= 6 && "Max expected size is 6");

			for (Mesh* mesh : allMeshes)
			{
				mesh->m_subMeshes.clear();
			}

			AxisLabel hitRes = HitTest(vp->RayFromMousePosition());

			bool firstFilled = false;
			for (int i = 0; i < 3; i++)
			{
				m_lines[i]->m_material->m_color = g_gizmoColor[i];
				m_solids[i]->m_material->m_color = g_gizmoColor[i];

				if (m_inAccessable == (AxisLabel)i)
				{
					continue;
				}

				if (!firstFilled)
				{
					m_mesh = m_lines[i];
					m_mesh->m_subMeshes.push_back(m_solids[i]);

					firstFilled = true;
				}
				else
				{
					m_mesh->m_subMeshes.push_back(m_lines[i]);
					m_mesh->m_subMeshes.push_back(m_solids[i]);
				}

				if (hitRes == (AxisLabel)i)
				{
					m_lines[i]->m_material->m_color = g_selectHighLightPrimaryColor;
					m_solids[i]->m_material->m_color = g_selectHighLightPrimaryColor;
				}
			}
		}

		// Static cone heads.
		std::shared_ptr<Mesh> g_ArrowHeads[3] = { nullptr, nullptr, nullptr };

		void MoveGizmo::Generate()
		{
			// Axis dimensions.
			const float tip = 0.8f, toe = 0.05f, rad = 0.1f;

			// Hit box line.
			m_hitBox[0].min = Vec3(0.05f, -0.05f, -0.05f);
			m_hitBox[0].max = Vec3(1.0f, 0.05f, 0.05f);
			
			m_hitBox[1].min = m_hitBox[0].min.yxz;
			m_hitBox[1].max = m_hitBox[0].max.yxz;

			m_hitBox[2].min = m_hitBox[0].min.zyx;
			m_hitBox[2].max = m_hitBox[0].max.zyx;

			// Hit box cone.
			m_hitBox[3].min = Vec3(tip, -rad, -rad);
			m_hitBox[3].max = Vec3(1.0f, rad, rad);

			m_hitBox[4].min = m_hitBox[3].min.yxz;
			m_hitBox[4].max = m_hitBox[3].max.yxz;

			m_hitBox[5].min = m_hitBox[3].min.zyx;
			m_hitBox[5].max = m_hitBox[3].max.zyx;

			// Lines.
			for (int i = 0; i < 3; i++)
			{
				std::vector<Vec3> points
				{
					AXIS[i] * tip,
					AXIS[i] * toe
				};
				LineBatch l(points, g_gizmoColor[i], DrawType::Line);
				// l.m_mesh->m_material->GetRenderState()->depthTestEnabled = false;
				l.m_mesh->Init();
				m_lines[i] = l.m_mesh;
				l.m_mesh = nullptr;
			}

			m_mesh = m_lines[0];
			m_mesh->m_subMeshes.push_back(m_lines[1]);
			m_mesh->m_subMeshes.push_back(m_lines[2]);

			// Solids.
			for (int i = 0; i < 3; i++)
			{
				// Cones 1 time init.
				if (g_ArrowHeads[i] == nullptr)
				{
					Cone head = Cone(1.0f - tip, rad, 10, 10);
					head.m_mesh->UnInit();

					Vec3 t(0.0f, tip, 0.0f);
					Quaternion q;

					if (i == 0)
					{
						q = glm::angleAxis(-glm::half_pi<float>(), Z_AXIS);
					}

					if (i == 2)
					{
						q = glm::angleAxis(glm::half_pi<float>(), X_AXIS);
					}

					Mat4 transform = glm::toMat4(q);
					transform = glm::translate(transform, t);
					Mat4 invTrans = glm::transpose(glm::inverse(transform));

					for (Vertex& v : head.m_mesh->m_clientSideVertices)
					{
						v.pos = transform * Vec4(v.pos, 1.0f);
						v.norm = glm::inverseTranspose(transform) * Vec4(v.norm, 1.0f);
					}

					head.m_mesh->m_material = GetMaterialManager()->GetCopyOfSolidMaterial();
					head.m_mesh->m_material->m_color = g_gizmoColor[i];
					// head.m_mesh->m_material->GetRenderState()->depthTestEnabled = false;
					head.m_mesh->Init(true);

					g_ArrowHeads[i] = head.m_mesh;
					head.m_mesh = nullptr;
				}

				m_solids[i] = g_ArrowHeads[i];
			}
		}

	}
}
