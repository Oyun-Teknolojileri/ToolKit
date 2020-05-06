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

		// GizmoHandle
		//////////////////////////////////////////////////////////////////////////

		GizmoHandle::GizmoHandle()
		{
		}

		GizmoHandle::~GizmoHandle()
		{
		}

		void GizmoHandle::Generate(const Params& params)
		{
			m_params = params;

			// Object is oriented in world space, translated to world space while rendering.
			std::vector<Vec3> pnts =
			{
				params.dir.direction * params.toeTip.x,
				params.dir.direction * params.toeTip.y
			};

			LineBatch line(pnts, params.color, DrawType::Line, 2.0f);
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
				
				switch (params.axis)
				{
				case AxisLabel::X:
					v.pos = v.pos.yxz;
					break;
				case AxisLabel::Z:
					v.pos = v.pos.zxy;
					break;
				case AxisLabel::Y:
				default:
					break;
				}

				v.pos = params.normalVectors * v.pos;
			}
		}

		bool GizmoHandle::HitTest(const Ray& ray, float& t) const
		{
			// Hit test done in object space bounding boxes. Ray is transformed to object space.
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

		void PolarHandle::Generate(const Params& params)
		{
			m_params = params;
			
			// Object is oriented in world space, translated to world space while rendering.
			int cornerCount = 60;
			std::vector<Vec3> corners;

			float deltaAngle = glm::two_pi<float>() / cornerCount;
			for (int i = 0; i < cornerCount; i++)
			{
				float angle = deltaAngle * i;
				corners.push_back(Vec3(glm::cos(angle), glm::sin(angle), 0.0f));

				switch (params.axis)
				{
				case AxisLabel::X:
					corners[i] = corners[i].yzx;
					break;
				case AxisLabel::Y:
					corners[i] = corners[i].xzy;
					break;
				case AxisLabel::Z:
					corners[i] = corners[i].xzy;
					break;
				default:
					assert(false);
					break;
				}
			}
			corners.push_back(corners.front());

			LineBatch circle(corners, params.color, DrawType::LineStrip, 4.0f);
			m_mesh = circle.m_mesh;
			circle.m_mesh = nullptr;

			for (Vertex& v : m_mesh->m_clientSideVertices)
			{
				switch (params.axis)
				{
				case AxisLabel::X:
					v.pos = v.pos.yxz;
					break;
				case AxisLabel::Z:
					v.pos = v.pos.zxy;
					break;
				case AxisLabel::Y:
				default:
					break;
				}

				v.pos = params.normalVectors * v.pos;
			}
		}

		LineBatch* GenerateBoundingVolumeGeometry(const BoundingBox& box, Mat4* transform = nullptr)
		{			
			Vec3 scale = box.max - box.min;
			Cube cube(scale);

			std::vector<Vec3> vertices =
			{
				Vec3(-0.5f, 0.5f, 0.5f) * scale, // FTL.
				Vec3(-0.5f, -0.5f, 0.5f) * scale, // FBL.
				Vec3(0.5f, -0.5f, 0.5f) * scale, // FBR.
				Vec3(0.5f, 0.5f, 0.5f) * scale, // FTR.
				Vec3(-0.5f, 0.5f, 0.5f) * scale, // FTL.
				Vec3(-0.5f, 0.5f, -0.5f) * scale, // BTL.
				Vec3(-0.5f, -0.5f, -0.5f) * scale, // BBL.
				Vec3(0.5f, -0.5f, -0.5f) * scale, // BBR.
				Vec3(0.5f, 0.5f, -0.5f) * scale, // BTR.
				Vec3(-0.5f, 0.5f, -0.5f) * scale, // BTL.
				Vec3(0.5f, 0.5f, -0.5f) * scale, // BTR.
				Vec3(0.5f, 0.5f, 0.5f) * scale, // FTR.
				Vec3(0.5f, -0.5f, 0.5f) * scale, // FBR.
				Vec3(0.5f, -0.5f, -0.5f) * scale, // BBR.
				Vec3(-0.5f, -0.5f, -0.5f) * scale, // BBL.
				Vec3(-0.5f, -0.5f, 0.5f) * scale // FBL.
			};


			Vec3 mid = (box.min + box.max) * 0.5f;
			for (Vec3& v : vertices)
			{
				v += mid;
				if (transform != nullptr)
				{
					v = *transform * Vec4(v, 1.0f);
				}
			}

			LineBatch* lineForm = new LineBatch(vertices, X_AXIS, DrawType::LineStrip, 2.0f);
			return lineForm;
		}

		bool PolarHandle::HitTest(const Ray& ray, float& t) const
		{
			t = TK_FLT_MAX;

			Mat4 ts = m_params.parentTransform;
			if (Viewport* vp = g_app->GetActiveViewport())
			{
				for (size_t i = 1; i < m_mesh->m_clientSideVertices.size(); i++)
				{
					Vec3& v1 = m_mesh->m_clientSideVertices[i - 1].pos;
					Vec3& v2 = m_mesh->m_clientSideVertices[i].pos;
					Vec3 mid = (v1 + v2) * 0.5f;
					BoundingBox bb =
					{
						mid - Vec3(0.05f),
						mid + Vec3(0.05f)
					};

					bb.min = m_params.parentTransform * Vec4(bb.min, 1.0f);
					bb.max = m_params.parentTransform * Vec4(bb.max, 1.0f);

					float tInt;
					if (RayBoxIntersection(ray, bb, tInt))
					{
						if (tInt < t)
						{
							t = tInt;
						}
					}

					LineBatch* vol = GenerateBoundingVolumeGeometry(bb, nullptr);
					g_app->m_perFrameDebugObjects.push_back(vol);
				}
			}

			// Prevent back face selection by masking.
			float maskDist;
			BoundingSphere maskSphere = { m_params.dir.position, 0.95f };
			
			// Calculate scaled rad due to window aspect. (billboard prop.)
			Vec3 rad(0.95f);
			rad = m_params.parentTransform * Vec4(rad, 0.0f);
			assert(glm::all(glm::equal(rad, rad.xxx())) && "Uniform scale expected.");
			
			maskSphere.radius = rad.x;
			Sphere* s = new Sphere(rad.x);
			g_app->m_perFrameDebugObjects.push_back(s);

			if (RaySphereIntersection(ray, maskSphere, maskDist))
			{
				if (maskDist < t)
				{
					return false;
				}
			}

			return t != TK_FLT_MAX;

			PlaneEquation plane = PlaneFrom(m_params.dir.position, m_params.dir.direction);
			if (LinePlaneIntersection(ray, plane, t))
			{
				
				float maskDist;
				BoundingSphere maskSphere = { m_params.dir.position, 0.95f * 0.5f };
				if (RaySphereIntersection(ray, maskSphere, maskDist))
				{
					if (maskDist < t)
					{
						return false;
					}
				}

				Vec3 intersection = PointOnRay(ray, t);
				float dist2center = glm::distance(intersection, m_params.dir.position);
				return dist2center > 0.47f && dist2center < 0.51f;
			}

			return false;
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
			for (size_t i = 0; i < m_handles.size(); i++)
			{
				SafeDel(m_handles[i]);
			}
		}

		AxisLabel Gizmo::HitTest(const Ray& ray) const
		{
			float t, tMin = std::numeric_limits<float>::max();
			AxisLabel hit = AxisLabel::None;
			for (size_t i = 0; i < m_handles.size(); i++)
			{
				if (m_handles[i]->HitTest(ray, t))
				{
					if (t < tMin)
					{
						tMin = t;
						hit = (AxisLabel)i;
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

		AxisLabel Gizmo::GetGrabbedAxis() const
		{
			return m_grabbedAxis;
		}

		// LinearGizmo
		//////////////////////////////////////////////////////////////////////////

		LinearGizmo::LinearGizmo()
			: Gizmo({ false, 6.0f, 400.0f })
		{
			m_handles =
			{
				new GizmoHandle(),
				new GizmoHandle(),
				new GizmoHandle()
			};

			Update(0.0f);
		}

		LinearGizmo::~LinearGizmo()
		{
		}

		void LinearGizmo::Update(float deltaTime)
		{
			GizmoHandle::Params p = GetParam();

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

				if (IsLocked((AxisLabel)i))
				{
					p.color = g_gizmoLocked;
				}
				else if (m_lastHovered == (AxisLabel)i)
				{
					p.color = g_selectHighLightSecondaryColor;
					m_lastHovered = AxisLabel::None;
				}

				p.normalVectors = m_normalVectors;
				p.axis = (AxisLabel)i;
				p.dir.direction = m_normalVectors[i];
				m_handles[i]->Generate(p);
			}

			m_mesh = m_handles[0]->m_mesh;
			m_mesh->m_subMeshes.push_back(m_handles[1]->m_mesh);
			m_mesh->m_subMeshes.push_back(m_handles[2]->m_mesh);
		}

		GizmoHandle::Params LinearGizmo::GetParam() const
		{
			const float tip = 0.8f, toe = 0.05f, rad = 0.1f;

			GizmoHandle::Params p;
			p.normalVectors = m_normalVectors;
			p.dir.position = m_node->GetTranslation(TransformationSpace::TS_WORLD);
			p.solidDim.xyz = Vec3(rad, 1.0f - tip, rad);
			p.toeTip = Vec3(toe, tip, 0.0f);
			p.type = GizmoHandle::SolidType::Cone;

			return p;
		}

		MoveGizmo::MoveGizmo()
		{
		}

		MoveGizmo::~MoveGizmo()
		{
		}

		ScaleGizmo::ScaleGizmo()
		{
		}

		ScaleGizmo::~ScaleGizmo()
		{
		}

		GizmoHandle::Params ScaleGizmo::GetParam() const
		{
			GizmoHandle::Params p = LinearGizmo::GetParam();
			p.solidDim = Vec3(0.15f);
			p.type = GizmoHandle::SolidType::Cube;

			return p;
		}

		PolarGizmo::PolarGizmo()
			: Gizmo({ false, 0.0f, 400.0f })
		{
			m_handles =
			{
				new PolarHandle(),
				new PolarHandle(),
				new PolarHandle()
			};

			Update(0.0f);
		}

		PolarGizmo::~PolarGizmo()
		{
		}

		void PolarGizmo::LookAt(Camera* cam)
		{
			Billboard::LookAt(cam);
			m_camDir = cam->GetDir();
			Update(0.0f); // Plane cuts.
		}

		void PolarGizmo::Update(float deltaTime)
		{
			GizmoHandle::Params p;
			p.parentTransform = m_node->GetTransform(TransformationSpace::TS_WORLD);
			p.dir.position = m_node->GetTranslation(TransformationSpace::TS_WORLD);

			for (int i = 0; i < 3; i++)
			{
				p.dir.direction = m_normalVectors[i];

				if (m_grabbedAxis == (AxisLabel)i)
				{
					p.color = g_selectHighLightPrimaryColor;
				}
				else
				{
					p.color = g_gizmoColor[i];
				}

				if (IsLocked((AxisLabel)i))
				{
					p.color = g_gizmoLocked;
				}
				else if (m_lastHovered == (AxisLabel)i)
				{
					p.color = Vec3(1.0f);
					m_lastHovered = AxisLabel::None;
				}

				p.normalVectors = m_normalVectors;
				p.axis = (AxisLabel)i;
				m_handles[i]->Generate(p);
			}

			m_mesh = m_handles[0]->m_mesh;
			m_mesh->m_subMeshes.push_back(m_handles[1]->m_mesh);
			m_mesh->m_subMeshes.push_back(m_handles[2]->m_mesh);
		}

		void PolarGizmo::Render(Renderer* renderer, Camera* cam)
		{
			// Draw an inverted sphere to mask back side.
			static std::shared_ptr<Sphere> sphere = nullptr;
			if (sphere == nullptr)
			{
				sphere = std::make_shared<Sphere>();
				sphere->m_mesh->m_material->GetRenderState()->cullMode = CullingType::Front;
			}

			*sphere->m_node = *m_node;
			sphere->m_node->Scale(Vec3(0.95f));

			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			renderer->Render(sphere.get(), cam);
			
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			renderer->Render(this, cam);
		}

	}

}
