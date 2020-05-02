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

		Cursor::~Cursor()
		{
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

		Axis3d::~Axis3d()
		{
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

		// Gizmo
		//////////////////////////////////////////////////////////////////////////

		Gizmo::Gizmo(const Billboard::Settings& set)
			: Billboard(set)
		{
			m_grabbedAxis = AxisLabel::None;
		}

		Gizmo::~Gizmo()
		{
		}
		
		ToolKit::AxisLabel Gizmo::HitTest(const Ray& ray) const
		{
			Mat4 invMat = m_node->GetTransform(TransformationSpace::TS_WORLD);
			Mat4 tpsMat = glm::transpose(invMat);
			invMat = glm::inverse(invMat);

			Ray rayInObjectSpace = ray;
			rayInObjectSpace.position = invMat * Vec4(rayInObjectSpace.position, 1.0f);
			rayInObjectSpace.direction = tpsMat * Vec4(rayInObjectSpace.direction, 1.0f);

			AxisLabel hit = AxisLabel::None;
			float d, t = std::numeric_limits<float>::infinity();
			for (const LabelBoxPair& pair : m_hitBoxes)
			{
				for (const BoundingBox& hitBox : pair.second)
				{
					if (RayBoxIntersection(rayInObjectSpace, hitBox, d))
					{
						if (d < t)
						{
							t = d;
							hit = pair.first;
						}
					}
				}
			}

			return hit;
		}

		bool Gizmo::IsLocked(AxisLabel axis) const
		{
			return std::find(m_lockedAxis.begin(), m_lockedAxis.end(), axis) != m_lockedAxis.end();
		}

		void Gizmo::Lock(AxisLabel axis)
		{
			assert(axis != AxisLabel::None);
			if (axis != AxisLabel::None && !IsLocked(axis))
			{
				m_lockedAxis.push_back(axis);
			}
		}

		void Gizmo::UnLock(AxisLabel axis)
		{
			m_lockedAxis.erase
			(
				std::remove(m_lockedAxis.begin(), m_lockedAxis.end(), axis), m_lockedAxis.end()
			);
		}

		bool Gizmo::IsGrabbed(AxisLabel axis) const
		{
			return m_grabbedAxis == axis;
		}

		void Gizmo::Grab(AxisLabel axis)
		{
			if (axis != AxisLabel::None)
			{
				bool locked = IsLocked(axis);
				assert(!locked && "A locked axis cant be grabbed.");
				if (!locked)
				{
					m_grabbedAxis = axis;
				}
			}
			else
			{
				m_grabbedAxis = axis;
			}
		}

		ToolKit::AxisLabel Gizmo::GetGrabbedAxis() const
		{
			return m_grabbedAxis;
		}

		// MoveGizmo
		//////////////////////////////////////////////////////////////////////////

		MoveGizmo::MoveGizmo()
			: Gizmo({ false, 6.0f, 400.0f })
		{
			// Mesh.
			Generate();
		}

		MoveGizmo::~MoveGizmo()
		{
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
			assert(allMeshes.size() <= 9 && "Max expected size is 9");

			for (Mesh* mesh : allMeshes)
			{
				mesh->m_subMeshes.clear();
			}

			AxisLabel hitRes = HitTest(vp->RayFromMousePosition());

			// Arrows.
			bool firstFilled = false;
			for (int i = 0; i < 3; i++)
			{
				m_lines[i]->m_material->m_color = g_gizmoColor[i];
				m_lines[i + 3]->m_material->m_color = g_gizmoColor[i];
				m_solids[i]->m_material->m_color = g_gizmoColor[i];

				if (IsLocked((AxisLabel)i))
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

				if (hitRes == (AxisLabel)i || m_grabbedAxis == (AxisLabel)i)
				{
					m_lines[i]->m_material->m_color = g_selectHighLightPrimaryColor;
					m_solids[i]->m_material->m_color = g_selectHighLightPrimaryColor;
				}
			}

			// Planes.
			for (int i = 0; i < 3; i++)
			{
				m_lines[i + 3]->m_material->m_color = g_gizmoColor[(i + 2) % 3];
				
				if (hitRes == (AxisLabel)(i + 3) || m_grabbedAxis == (AxisLabel)(i + 3))
				{
					m_lines[i + 3]->m_material->m_color = g_selectHighLightPrimaryColor;
				}

				m_mesh->m_subMeshes.push_back(m_lines[i + 3]);
			}
		}

		// Static cone heads.
		std::shared_ptr<Mesh> g_ArrowHeads[3] = { nullptr, nullptr, nullptr };

		void MoveGizmo::Generate()
		{
			// Axis dimensions.
			const float tip = 0.8f, toe = 0.05f, rad = 0.1f;

			LabelBoxPair hb;
			hb.first = AxisLabel::X;

			// Line.
			hb.second.push_back
			(
				{
				Vec3(0.05f, -0.05f, -0.05f),
				Vec3(1.0f, 0.05f, 0.05f)
				}
			);

			// Cone.
			hb.second.push_back
			(
				{
					Vec3(tip, -rad, -rad),
					Vec3(1.0f, rad, rad)
				}
			);
			m_hitBoxes.push_back(hb);

			hb.first = AxisLabel::Y;
			hb.second[0].min = hb.second[0].min.yxz;
			hb.second[0].max = hb.second[0].max.yxz;
			hb.second[1].min = hb.second[1].min.yxz;
			hb.second[1].max = hb.second[1].max.yxz;
			m_hitBoxes.push_back(hb);

			hb.first = AxisLabel::Z;
			hb.second[0].min = hb.second[0].min.zxy;
			hb.second[0].max = hb.second[0].max.zxy;
			hb.second[1].min = hb.second[1].min.zxy;
			hb.second[1].max = hb.second[1].max.zxy;
			m_hitBoxes.push_back(hb);

			// Lines.
			for (int i = 0; i < 3; i++)
			{
				std::vector<Vec3> points
				{
					AXIS[i] * tip,
					AXIS[i] * toe,
				};

				LineBatch l(points, g_gizmoColor[i], DrawType::Line);
				l.m_mesh->Init();
				m_lines[i] = l.m_mesh;
				l.m_mesh = nullptr;
			}

			// Planes
			const float o = 0.35f, s = 0.15f;
			for (int i = 0; i < 3; i++)
			{
				Vec3 axis1 = AXIS[i] * o;
				Vec3 off1 = AXIS[i] * s;
				Vec3 axis2 = AXIS[(i + 1) % 3] * o;
				Vec3 off2 = AXIS[(i + 1) % 3] * s;

				std::vector<Vec3> points
				{
					axis1 + axis2,
					axis1 + off1 + axis2,
					axis1 + off1 + axis2 + off2,
					axis1 + axis2 + off2,
					axis1 + axis2,
					axis1 + off1 + axis2 + off2,
					axis1 + axis2 + off2
				};

				LineBatch plane(points, g_gizmoColor[(i + 2) % 3], DrawType::Triangle);
				plane.m_mesh->m_material->GetRenderState()->cullMode = CullingType::TwoSided;

				plane.m_mesh->Init();
				m_lines[i + 3] = plane.m_mesh;

				LabelBoxPair hbPlane;
				hbPlane.first = (AxisLabel)(i + 3);
				hbPlane.second.push_back(plane.m_mesh->m_aabb);
				m_hitBoxes.push_back(hbPlane);
				plane.m_mesh = nullptr;
			}

			m_mesh = m_lines[0];
			for (int i = 1; i < 6; i++)
			{
				m_mesh->m_subMeshes.push_back(m_lines[i]);
			}

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
					head.m_mesh->Init(true);

					g_ArrowHeads[i] = head.m_mesh;
					head.m_mesh = nullptr;
				}

				m_solids[i] = g_ArrowHeads[i];
			}
		}

		// ScaleGizmo
		//////////////////////////////////////////////////////////////////////////

		ScaleGizmo::ScaleGizmo()
			: Gizmo({ false, 6.0f, 400.0f })
		{
			// Mesh.
			Generate();
		}

		ScaleGizmo::~ScaleGizmo()
		{
		}

		void ScaleGizmo::Update(float deltaTime)
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

			// Axes.
			bool firstFilled = false;
			for (int i = 0; i < 3; i++)
			{
				m_lines[i]->m_material->m_color = g_gizmoColor[i];
				m_lines[i + 3]->m_material->m_color = g_gizmoColor[i];
				m_solids[i]->m_material->m_color = g_gizmoColor[i];

				if (IsLocked((AxisLabel)i))
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

				if (hitRes == (AxisLabel)i || m_grabbedAxis == (AxisLabel)i)
				{
					m_solids[i]->m_material->m_color = g_selectHighLightPrimaryColor;
				}
			}
		}

		// Static box heads.
		std::shared_ptr<Mesh> g_boxHeads[3] = { nullptr, nullptr, nullptr };

		void ScaleGizmo::Generate()
		{
			// Axis dimensions.
			const float tip = 0.8f, toe = 0.05f, rad = 0.05f;

			LabelBoxPair hb;
			hb.first = AxisLabel::X;

			// Line.
			hb.second.push_back
			(
				{
				Vec3(0.05f, -0.05f, -0.05f),
				Vec3(1.0f, 0.05f, 0.05f)
				}
			);

			// Box.
			hb.second.push_back
			(
				{
					Vec3(tip, -rad, -rad),
					Vec3(1.0f, rad, rad)
				}
			);
			m_hitBoxes.push_back(hb);

			hb.first = AxisLabel::Y;
			hb.second[0].min = hb.second[0].min.yxz;
			hb.second[0].max = hb.second[0].max.yxz;
			hb.second[1].min = hb.second[1].min.yxz;
			hb.second[1].max = hb.second[1].max.yxz;
			m_hitBoxes.push_back(hb);

			hb.first = AxisLabel::Z;
			hb.second[0].min = hb.second[0].min.zxy;
			hb.second[0].max = hb.second[0].max.zxy;
			hb.second[1].min = hb.second[1].min.zxy;
			hb.second[1].max = hb.second[1].max.zxy;
			m_hitBoxes.push_back(hb);

			// Lines.
			for (int i = 0; i < 3; i++)
			{
				std::vector<Vec3> points
				{
					AXIS[i] * tip,
					AXIS[i] * toe,
				};

				LineBatch l(points, g_gizmoColor[i], DrawType::Line);
				l.m_mesh->Init();
				m_lines[i] = l.m_mesh;
				l.m_mesh = nullptr;
			}

			m_mesh = m_lines[0];
			for (int i = 1; i < 3; i++)
			{
				m_mesh->m_subMeshes.push_back(m_lines[i]);
			}

			// Solids.
			for (int i = 0; i < 3; i++)
			{
				// Boxes 1 time init.
				if (g_boxHeads[i] == nullptr)
				{
					Cube head = Cube(Vec3(1.0f - tip));
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
					head.m_mesh->Init(true);

					g_boxHeads[i] = head.m_mesh;
					head.m_mesh = nullptr;
				}

				m_solids[i] = g_boxHeads[i];
			}
		}

		RotateGizmo::RotateGizmo()
			: Gizmo({ false, 6.0f, 400.0f })
		{
		}

		RotateGizmo::~RotateGizmo()
		{
		}

		ToolKit::AxisLabel RotateGizmo::HitTest(const Ray& ray) const
		{
			for (int i = 0; i < 3; i++)
			{
				if (m_handles[i].HitTest(ray))
				{
					return (AxisLabel)i;
				}
			}

			return AxisLabel::None;
		}

		void RotateGizmo::Update(float deltaTime)
		{
			Viewport* vp = g_app->GetActiveViewport();
			if (vp == nullptr)
			{
				return;
			}

			GizmoHandle::HandleParams p = GetParam();

			for (int i = 0; i < 3; i++)
			{
				if (m_grabbedAxis == (AxisLabel)i)
				{
					p.color = g_selectHighLightPrimaryColor;
				}
				else
				{
					p.color = g_gizmoColor[i];
				}
				
				p.dir.direction = m_normalVectors[i];
				m_handles[i].Generate(p);
			}

			m_mesh = m_handles[0].m_mesh;
			m_mesh->m_subMeshes.push_back(m_handles[1].m_mesh);
			m_mesh->m_subMeshes.push_back(m_handles[2].m_mesh);
		}

		GizmoHandle::HandleParams RotateGizmo::GetParam() const
		{
			const float tip = 0.8f, toe = 0.05f, rad = 0.05f;

			GizmoHandle::HandleParams p;
			p.dir.position = m_node->GetTranslation(TransformationSpace::TS_WORLD);
			p.solidDim.xyz = Vec3(rad, 1.0f - tip, rad);
			p.toeTip = Vec2(toe, tip);
			p.type = GizmoHandle::SolidType::Cone;

			return p;
		}

		GizmoHandle::GizmoHandle()
		{
		}

		GizmoHandle::~GizmoHandle()
		{
		}

		void GizmoHandle::Generate(const HandleParams& params)
		{
			m_params = params;

			std::vector<Vec3> pnts =
			{
				Vec3(0.0f, params.toeTip.x, 0.0f),
			  Vec3(0.0f, params.toeTip.y, 0.0f)
			};

			Quaternion headOrientation = RotationTo(AXIS[1], params.dir.direction);
			for (int i = 0; i < 2; i++)
			{
				pnts[i] = headOrientation * pnts[i];
				//pnts[i] += params.dir.position;
			}

			LineBatch line(pnts, params.color, DrawType::Line);
			m_mesh = line.m_mesh;
			line.m_mesh = nullptr;

			MaterialPtr material = GetMaterialManager()->GetCopyOfSolidMaterial();
			material->m_color = params.color;

			if (params.type == SolidType::Cube)
			{
				Cube solid(params.solidDim);
				solid.m_mesh->m_material = material;
				m_mesh->m_subMeshes.push_back(solid.m_mesh);
				solid.m_mesh = nullptr;
			}
			else if (params.type == SolidType::Cone)
			{
				Cone solid(params.solidDim.y, params.solidDim.x, 10, 10);
				solid.m_mesh->m_material = material;
				m_mesh->m_subMeshes.push_back(solid.m_mesh);
				solid.m_mesh = nullptr;
			}
			else
			{
				assert(false);
				return;
			}

			MeshPtr m = m_mesh->m_subMeshes.back();
			for (Vertex& v : m->m_clientSideVertices)
			{
				v.pos.y += params.toeTip.y;
				v.pos = headOrientation * v.pos;
				//v.pos += params.dir.position;
			}
		}

		bool GizmoHandle::HitTest(const Ray& ray) const
		{
			Mat4 ts = glm::toMat4(RotationTo(AXIS[1], m_params.dir.direction));
			ts[3].xyz = m_params.dir.position;
			Mat4 its = glm::inverse(ts);

			Ray rayInObj;
			rayInObj.position = its * Vec4(ray.position, 1.0f);
			rayInObj.direction = its * Vec4(ray.direction, 0.0f);

			// Check for line.
			BoundingBox hitBox;
			hitBox.min.x = -0.05f;
			hitBox.min.y = m_params.toeTip.x;
			hitBox.min.z = -0.05f;
			hitBox.max.x = 0.05f;
			hitBox.max.y = m_params.toeTip.y;
			hitBox.max.z = 0.05f;

			float t;
			if (RayBoxIntersection(rayInObj, hitBox, t))
			{
				return true;
			}

			// Check for solid.
			hitBox.min.x = -m_params.solidDim.x * 0.5f;
			hitBox.min.y = m_params.toeTip.y;
			hitBox.min.z = -m_params.solidDim.z * 0.5f;
			hitBox.max.x = m_params.solidDim.x * 0.5f;
			hitBox.max.y = m_params.toeTip.y + m_params.solidDim.y;
			hitBox.max.z = m_params.solidDim.z * 0.5f;

			if (RayBoxIntersection(rayInObj, hitBox, t))
			{
				return true;
			}

			return false;
		}

	}

}
