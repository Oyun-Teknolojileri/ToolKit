
#include "Gizmo.h"

#include <vector>
#include <memory>
#include <glm/gtc/matrix_transform.hpp>

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
#include "ConsoleWindow.h"
#include "EditorViewport.h"
#include "GL/glew.h"
#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    Cursor::Cursor()
      : Billboard({ true, 10.0f, 60.0f })
    {
      Generate();
    }

    Cursor::~Cursor()
    {
    }

    void Cursor::Generate()
    {
      MeshPtr& parentMesh = GetMesh();
      parentMesh->UnInit();

      // Billboard
      Quad quad;
      MeshPtr& meshPtr = quad.GetMesh();

      meshPtr->m_material = GetMaterialManager()->GetCopyOfUnlitMaterial();
      meshPtr->m_material->UnInit();
      meshPtr->m_material->m_diffuseTexture =
      GetTextureManager()->Create<Texture>
      (
        TexturePath(ConcatPaths({ "Icons", "cursor4k.png" }), true)
      );
      meshPtr->m_material->GetRenderState()->blendFunction =
      BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA;
      meshPtr->m_material->Init();

      meshPtr->m_material->GetRenderState()->depthTestEnabled = false;
      parentMesh->m_subMeshes.push_back(meshPtr);

      // Lines
      VertexArray vertices;
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

      MaterialPtr newMaterial =
      GetMaterialManager()->GetCopyOfUnlitColorMaterial();
      newMaterial->m_color = Vec3(0.1f, 0.1f, 0.1f);
      newMaterial->GetRenderState()->drawType = DrawType::Line;
      newMaterial->GetRenderState()->depthTestEnabled = false;

      parentMesh->m_clientSideVertices = vertices;
      parentMesh->m_material = newMaterial;

      parentMesh->CalculateAABB();
    }

    Axis3d::Axis3d()
      : Billboard({ false, 10.0f, 60.0f })
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
        MeshPtr& mesh = arrow.GetMesh();
        mesh->m_material->GetRenderState()->depthTestEnabled = false;
        if (i == 0)
        {
          SetMesh(mesh);
        }
        else
        {
          GetMesh()->m_subMeshes.push_back(mesh);
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

      Vec3 dir = AXIS[static_cast<int>(params.axis) % 3];
      std::vector<Vec3> pnts =
      {
        dir * params.toeTip.x,
        dir * params.toeTip.y
      };

      m_mesh = std::make_shared<Mesh>();

      LineBatch line(pnts, params.color, DrawType::Line, 2.0f);
      m_mesh->m_subMeshes.push_back(line.GetMesh());

      MaterialPtr material =
      GetMaterialManager()->GetCopyOfUnlitColorMaterial();
      material->m_color = params.color;

      if (params.type == SolidType::Cube)
      {
        Cube solid(params.solidDim);
        MeshPtr& mesh = solid.GetMesh();
        mesh->m_material = material;
        m_mesh->m_subMeshes.push_back(mesh);
      }
      else if (params.type == SolidType::Cone)
      {
        Cone solid({ params.solidDim.y, params.solidDim.x, 10, 10 });
        MeshPtr& mesh = solid.GetMesh();
        mesh->m_material = material;
        m_mesh->m_subMeshes.push_back(mesh);
      }
      else
      {
        assert(false);
        return;
      }

      MeshPtr mesh = m_mesh->m_subMeshes.back();
      for (Vertex& v : mesh->m_clientSideVertices)
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
      }

      // Guide line.
      if (!glm::isNull(params.grabPnt, glm::epsilon<float>()))
      {
        int axisInd = static_cast<int>(m_params.axis);
        Vec3 axis = AXIS[axisInd];
        Vec3Array pnts =
        {
          axis * 999.0f,
          axis * -999.0f
        };

        LineBatch guide(pnts, g_gizmoColor[axisInd % 3], DrawType::Line, 1.0f);
        m_mesh->m_subMeshes.push_back(guide.GetMesh());
      }
    }

    bool GizmoHandle::HitTest(const Ray& ray, float& t) const
    {
      // Hit test done in object space bounding boxes.
      Mat4 transform = GetTransform();
      Mat4 its = glm::inverse(transform);

      Ray rayInObj;
      rayInObj.position = its * Vec4(ray.position, 1.0f);
      rayInObj.direction = its * Vec4(ray.direction, 0.0f);

      m_mesh->CalculateAABB();
      return RayBoxIntersection(rayInObj, m_mesh->m_aabb, t);
    }

    Mat4 GizmoHandle::GetTransform() const
    {
      Mat4 sc = glm::scale(Mat4(), m_params.scale);
      Mat4 rt = Mat4(m_params.normals);
      Mat4 ts = glm::translate(Mat4(), m_params.translate);
      Mat4 transform = ts * rt * sc;

      return transform;
    }

    void PolarHandle::Generate(const Params& params)
    {
      m_params = params;

      int cornerCount = 60;
      std::vector<Vec3> corners;
      corners.reserve(cornerCount + 1);

      float deltaAngle = glm::two_pi<float>() / cornerCount;
      for (int i = 0; i < cornerCount; i++)
      {
        float angle = deltaAngle * i;
        corners.push_back(Vec3(glm::cos(angle), glm::sin(angle), 0.0f));

        switch (params.axis)
        {
        case AxisLabel::X:
          corners[i] = corners[i].zyx;
          break;
        case AxisLabel::Y:
          corners[i] = corners[i].xzy;
          break;
        case AxisLabel::Z:
          break;
        default:
          assert(false);
          break;
        }
      }
      corners.push_back(corners.front());

      LineBatch circle(corners, params.color, DrawType::LineStrip, 4.0f);
      m_mesh = circle.GetMesh();

      // Guide line.
      if (!glm::isNull(params.grabPnt, glm::epsilon<float>()))
      {
        // Bring the grab point to object space.
        Vec3 glcl = params.grabPnt - params.worldLoc;
        glcl = glm::normalize(glm::inverse(params.normals) * glcl);

        int axisIndx = static_cast<int>(params.axis);
        Vec3 axis = AXIS[axisIndx];

        // Neighbor points for parallel line.
        Vec3 p1 = glm::normalize(glm::angleAxis(0.0001f, axis) * glcl);
        Vec3 p2 = glm::normalize(glm::angleAxis(-0.0001f, axis) * glcl);
        Vec3 dir = glm::normalize(p1 - p2);
        m_tangentDir = glm::normalize(params.normals * dir);

        Vec3Array pnts;
        pnts.push_back(glcl + dir * 999.0f);
        pnts.push_back(glcl - dir * 999.0f);

        LineBatch guide(pnts, g_gizmoColor[axisIndx], DrawType::Line, 1.0f);
        m_mesh->m_subMeshes.push_back(guide.GetMesh());
      }
    }

    bool PolarHandle::HitTest(const Ray& ray, float& t) const
    {
      t = TK_FLT_MAX;

      Mat4 sc = glm::scale(Mat4(), m_params.scale);
      Mat4 rt = Mat4(m_params.normals);
      Mat4 ts = glm::translate(Mat4(), m_params.translate);
      Mat4 transform = ts * rt * sc;
      Mat4 its = glm::inverse(transform);

      Ray rayInObj;
      rayInObj.position = its * Vec4(ray.position, 1.0f);
      rayInObj.direction = its * Vec4(ray.direction, 0.0f);

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

        float tInt;
        if (RayBoxIntersection(rayInObj, bb, tInt))
        {
          if (tInt < t)
          {
            t = tInt;
          }
        }
      }

      // No box hit.
      if (t == TK_FLT_MAX)
      {
        return false;
      }

      // Prevent back face selection by masking.
      float maskDist;
      BoundingSphere maskSphere;
      maskSphere.radius = 0.95f;

      if (RaySphereIntersection(rayInObj, maskSphere, maskDist))
      {
        if (maskDist < t)
        {
          return false;
        }
      }

      return true;
    }

    // QuadHandle
    //////////////////////////////////////////////////////////////////////////

    void QuadHandle::Generate(const Params& params)
    {
      m_params = params;

      Quad solid;
      MaterialPtr material =
      GetMaterialManager()->GetCopyOfUnlitColorMaterial();
      material->m_color = params.color;
      material->GetRenderState()->cullMode = CullingType::TwoSided;

      MeshPtr& mesh = solid.GetMesh();
      mesh->m_material = material;
      m_mesh = mesh;

      float scale = 0.15f;
      float offset = 2.0f;

      for (Vertex& v : m_mesh->m_clientSideVertices)
      {
        v.pos.y += params.toeTip.y;
        switch (params.axis)
        {
        case AxisLabel::XY:
          v.pos *= scale;
          v.pos.x += 0.75f * scale;
          v.pos.xy += Vec2(offset * scale);
          break;
        case AxisLabel::YZ:
          v.pos = v.pos.zyx * scale;
          v.pos.z += 0.75f * scale;
          v.pos.yz += Vec2(offset * scale);
          break;
        case AxisLabel::ZX:
          v.pos = v.pos.xzy * scale;
          v.pos.x += 0.75f * scale;
          v.pos.zx += Vec2(offset * scale);
          break;
        default:
          break;
        }
      }

      // Guide line.
      // Gizmo updates later, transform modes uses previous frame's locatin.
      // Fix Node Set - Get - Apply transfroms before.
      /*if (!glm::isNull(params.grabPnt, glm::epsilon<float>()))
      {
        Mat4 its = glm::inverse(GetTransform());
        Vec3 glcl = its * Vec4(params.initialPnt, 1.0f);

        LineBatch* guides[3];
        for (int i = 0; i < 3; i++)
        {
          guides[i] = new LineBatch
          (
            {
              glcl + AXIS[i] * 999.0f,
              glcl + AXIS[i] * -999.0f
            },
            g_gizmoColor[i],
            DrawType::Line, 
            1.0f
          );
        }

        int next = (((int)(params.axis) % 3) + 1) % 3;
        m_mesh->m_subMeshes.push_back(guides[next]->m_mesh);
        guides[next]->m_mesh = nullptr;

        next = (next + 1) % 3;
        m_mesh->m_subMeshes.push_back(guides[next]->m_mesh);
        guides[next]->m_mesh = nullptr;

        for (int i = 0; i < 3; i++)
        {
          SafeDel(guides[i]);
        }
      }*/
    }

    bool QuadHandle::HitTest(const Ray& ray, float& t) const
    {
      Mat4 sc = glm::scale(Mat4(), m_params.scale);
      Mat4 rt = Mat4(m_params.normals);
      Mat4 ts = glm::translate(Mat4(), m_params.translate);
      Mat4 transform = ts * rt * sc;
      Mat4 its = glm::inverse(transform);

      Ray rayInObj;
      rayInObj.position = its * Vec4(ray.position, 1.0f);
      rayInObj.direction = its * Vec4(ray.direction, 0.0f);

      m_mesh->CalculateAABB();
      return RayBoxIntersection(rayInObj, m_mesh->m_aabb, t);
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
      float t, tMin = TK_FLT_MAX;
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
      return std::find
      (
        m_lockedAxis.begin(),
        m_lockedAxis.end(),
        axis
      ) != m_lockedAxis.end();
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
        std::remove
        (
          m_lockedAxis.begin(),
          m_lockedAxis.end(),
          axis
        ),
        m_lockedAxis.end()
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

    void Gizmo::LookAt(Camera* cam, float windowHeight)
    {
      Billboard::LookAt(cam, windowHeight);
      m_node->SetOrientation(glm::toQuat(m_normalVectors));
    }

    GizmoHandle::Params Gizmo::GetParam() const
    {
      GizmoHandle::Params p;
      p.normals = m_normalVectors;
      p.worldLoc = m_worldLocation;
      p.initialPnt = m_initialPoint;
      Mat4 ts = m_node->GetTransform(TransformationSpace::TS_WORLD);
      DecomposeMatrix(ts, &p.translate, nullptr, &p.scale);

      return p;
    }

    // LinearGizmo
    //////////////////////////////////////////////////////////////////////////

    LinearGizmo::LinearGizmo()
      : Gizmo({ false, 6.0f, 60.0f })
    {
      m_handles =
      {
        new GizmoHandle(),
        new GizmoHandle(),
        new GizmoHandle()
      };
      m_handles[0]->m_params.axis = AxisLabel::X;
      m_handles[1]->m_params.axis = AxisLabel::Y;
      m_handles[2]->m_params.axis = AxisLabel::Z;

      Update(0.0f);
    }

    LinearGizmo::~LinearGizmo()
    {
    }

    void LinearGizmo::Update(float deltaTime)
    {
      GizmoHandle::Params p = GetParam();

      for (int i = 0; i < static_cast<int>(m_handles.size()); i++)
      {
        GizmoHandle* handle = m_handles[i];
        AxisLabel axis = handle->m_params.axis;
        if (m_grabbedAxis == axis)
        {
          p.color = g_selectHighLightPrimaryColor;
        }
        else
        {
          p.color = g_gizmoColor[static_cast<int>(axis) % 3];
        }

        if (IsLocked(axis))
        {
          p.color = g_gizmoLocked;
        }
        else if (m_lastHovered == axis)
        {
          p.color = g_selectHighLightSecondaryColor;
          m_lastHovered = AxisLabel::None;
        }

        p.axis = axis;
        if (IsGrabbed(p.axis))
        {
          p.grabPnt = m_grabPoint;
        }
        else
        {
          p.grabPnt = ZERO;
        }

        handle->Generate(p);
      }

      MeshPtr mesh = std::make_shared<Mesh>();
      for (int i = 0; i < m_handles.size(); i++)
      {
        mesh->m_subMeshes.push_back(m_handles[i]->m_mesh);
      }
      mesh->Init(false);
      SetMesh(mesh);
    }

    GizmoHandle::Params LinearGizmo::GetParam() const
    {
      const float tip = 0.8f, toe = 0.05f, rad = 0.1f;

      GizmoHandle::Params p;
      p.normals = m_normalVectors;
      p.worldLoc = m_worldLocation;
      p.initialPnt = m_initialPoint;
      Mat4 ts = m_node->GetTransform(TransformationSpace::TS_WORLD);
      DecomposeMatrix(ts, &p.translate, nullptr, &p.scale);

      p.solidDim.xyz = Vec3(rad, 1.0f - tip, rad);
      p.toeTip = Vec3(toe, tip, 0.0f);
      p.type = GizmoHandle::SolidType::Cone;

      return p;
    }

    MoveGizmo::MoveGizmo()
    {
      for (int i = 3; i < 6; i++)
      {
        m_handles.push_back(new QuadHandle());
        m_handles[i]->m_params.axis = (AxisLabel)i;
      }

      Update(0.0);
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
      : Gizmo({ false, 6.0f, 60.0f })
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

    void PolarGizmo::Update(float deltaTime)
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

        p.axis = (AxisLabel)i;
        if (IsGrabbed(p.axis))
        {
          p.grabPnt = m_grabPoint;
        }
        else
        {
          p.grabPnt = ZERO;
        }

        m_handles[i]->Generate(p);
      }

      MeshPtr mesh = std::make_shared<Mesh>();
      for (int i = 0; i < m_handles.size(); i++)
      {
        mesh->m_subMeshes.push_back(m_handles[i]->m_mesh);
      }

      SetMesh(mesh);
    }

    void PolarGizmo::Render(Renderer* renderer, Camera* cam)
    {
      // Draw an inverted sphere to mask back side.
      static std::shared_ptr<Sphere> sphere = nullptr;
      if (sphere == nullptr)
      {
        sphere = std::make_shared<Sphere>(1.0f);
        sphere->GetMesh()->m_material->GetRenderState()->cullMode =
        CullingType::Front;
      }

      *sphere->m_node = *m_node;
      sphere->m_node->Scale(Vec3(0.95f));

      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
      renderer->Render(sphere.get(), cam);

      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      renderer->Render(this, cam);
    }

    LightBillboard::LightBillboard(LightType lightType)
      : Billboard({ true, 10.0f, 60.0f })
    {
      String path = "";
      if (lightType == LightType::LightDirectional)
      {
        path = ConcatPaths({ "Icons", "light_sun.png" });
      }
      else
      {
        path = ConcatPaths({ "Icons", "light_point.png" });
      }

      MeshPtr& parentMesh = GetMesh();
      parentMesh->UnInit();

      // Billboard
      Quad quad;
      MeshPtr& meshPtr = quad.GetMesh();

      meshPtr->m_material = GetMaterialManager()->GetCopyOfUnlitMaterial();
      meshPtr->m_material->UnInit();
      meshPtr->m_material->m_diffuseTexture =
      GetTextureManager()->Create<Texture>(TexturePath(path, true));
      meshPtr->m_material->GetRenderState()->blendFunction =
      BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA;
      meshPtr->m_material->Init();
      parentMesh->m_subMeshes.push_back(meshPtr);
    }

    LightBillboard::~LightBillboard()
    {
    }

    void LightBillboard::RenderBillboard
    (
      Renderer* renderer,
      Viewport* viewport,
      Light* light
    )
    {
      // Translate billboard
      m_worldLocation =
      light->m_node->GetTranslation(TransformationSpace::TS_WORLD);

      // Rotate billboard
      LookAt(viewport->GetCamera(), viewport->m_zoom);

      // Render billboard
      renderer->Render(this, viewport->GetCamera());
    }

    LightBillboard::LightBillboard()
      : Billboard({ true, 10.0f, 60.0f })
    {
    }

    SpotLightGizmo::SpotLightGizmo()
      : Billboard({ false, 10.0f, 60.0f })
    {
      m_circleVertexCount = 36;
      m_identityMatrix = Mat4(1.0f);
      m_rot = Mat4(1.0f);

      m_pnts.resize(2);
      m_line = new LineBatch();

      GetComponent<MeshComponent>()->m_mesh = m_line->GetMesh();

      MeshPtr mesh = GetComponent<MeshComponent>()->m_mesh;

      mesh->m_material->Init();

      m_innerCirclePnts.resize(m_circleVertexCount + 1);
      m_outerCirclePnts.resize(m_circleVertexCount + 1);
      m_innerCircle = new LineBatch();
      m_outerCircle = new LineBatch();

      mesh->m_subMeshes.push_back(m_innerCircle->GetMesh());
      mesh->m_subMeshes[0]->m_material->Init();
      mesh->m_subMeshes.push_back(m_outerCircle->GetMesh());
      mesh->m_subMeshes[1]->m_material->Init();

      m_conePnts.resize(2 * (m_circleVertexCount / 4));
      m_coneLines = new LineBatch();

      mesh->m_subMeshes.push_back(m_coneLines->GetMesh());
      mesh->m_subMeshes[2]->m_material->Init();
    }

    SpotLightGizmo::~SpotLightGizmo()
    {
      SafeDel(m_innerCircle);
      SafeDel(m_outerCircle);
      SafeDel(m_coneLines);

      GetMesh()->UnInit();
      SafeDel(m_line);
    }

    void SpotLightGizmo::RenderGizmo
    (
      Renderer* renderer,
      Viewport* viewport,
      DirectionalLight* light
    )
    {
      // Middle line
      Vec3 d = light->GetDirection();
      float r = light->m_lightData.radius;
      m_pnts[0] = Vec3
      (
        light->m_node->GetTranslation(TransformationSpace::TS_WORLD)
      );
      m_pnts[1] = Vec3
      (
        light->m_node->GetTranslation(TransformationSpace::TS_WORLD)
        + d * r * 2.25f
      );
      m_line->Generate(m_pnts, Vec3(0.0f), DrawType::Line, 1.0f);

      // Calculating circles
      int zeroCount = 0;
      if (d.x == 0.0f)
      {
        zeroCount++;
      }
      if (d.y == 0.0f)
      {
        zeroCount++;
      }
      if (d.z == 0.0f)
      {
        zeroCount++;
      }

      Vec3 per;
      if (zeroCount == 0)
      {
        per.z = 0.0f;
        per.x = -d.y;
        per.y = d.x;
      }
      else if (zeroCount == 1)
      {
        if (d.x == 0.0f)
        {
          per.x = 0.0f;
          per.y = -d.z;
          per.z = d.y;
        }
        else if (d.y == 0.0f)
        {
          per.y = 0.0f;
          per.x = -d.z;
          per.z = d.x;
        }
        else
        {
          per.z = 0.0f;
          per.y = -d.x;
          per.x = d.y;
        }
      }
      else if (zeroCount == 2)
      {
        if (d.x == 0.0f && d.y == 0.0f)
        {
          per.x = 1.0f;
          per.y = 0.0f;
          per.z = 0.0f;
        }
        else if (d.x == 0.0f && d.z == 0.0f)
        {
          per.x = 1.0f;
          per.y = 0.0f;
          per.z = 0.0f;
        }
        else
        {
          per.x = 0.0f;
          per.y = 1.0f;
          per.z = 0.0f;
        }
      }
      per = glm::normalize(per);

      Vec3 normD = glm::normalize(d);
      d = normD * r;

      float innerCircleRadius = r
      * glm::tan(light->m_lightData.innerAngle / 2);
      float outerCircleRadius = r
      * glm::tan(light->m_lightData.outerAngle / 2);

      Vec3 lp = light->m_node->GetTranslation(TransformationSpace::TS_WORLD);
      Vec3 inStartPoint = lp + d + per * innerCircleRadius;
      Vec3 outStartPoint = lp + d + per * outerCircleRadius;
      m_innerCirclePnts[0] = inStartPoint;
      m_outerCirclePnts[0] = outStartPoint;

      float deltaAngle = glm::two_pi<float>() / m_circleVertexCount;
      m_rot = Mat4(1.0f);
      for (int i = 1; i < m_circleVertexCount + 1; i++)
      {
        // Inner circle vertices
        inStartPoint -= lp;
        m_rot = glm::rotate(m_identityMatrix, deltaAngle, normD);
        inStartPoint = Vec3(m_rot * Vec4(inStartPoint, 1.0f));
        inStartPoint += lp;
        m_innerCirclePnts[i] = inStartPoint;

        // Outer circle outStartPoint
        outStartPoint -= lp;
        m_rot = glm::rotate(m_identityMatrix, deltaAngle, normD);
        outStartPoint = Vec3(m_rot * Vec4(outStartPoint, 1.0f));
        outStartPoint += lp;
        m_outerCirclePnts[i] = outStartPoint;
      }

      m_innerCircle->Generate
      (
        m_innerCirclePnts,
        Vec3(0.15f),
        DrawType::LineStrip, 1.0f
      );
      m_outerCircle->Generate
      (
        m_outerCirclePnts,
        Vec3(0.15f),
        DrawType::LineStrip, 1.0f
      );

      MeshPtr mp = GetComponent<MeshComponent>()->m_mesh;
      mp->m_subMeshes.push_back(m_innerCircle->GetMesh());
      mp->m_subMeshes.push_back(m_outerCircle->GetMesh());
      mp->m_subMeshes[0]->m_material->Init();
      mp->m_subMeshes[1]->m_material->Init();

      // Cone
      int coneIndex = 0;
      for (int i = 0; i < m_circleVertexCount; i += 4)
      {
        m_conePnts[coneIndex] = lp;
        m_conePnts[coneIndex + 1] = m_outerCirclePnts[i];
        coneIndex += 2;
      }

      m_coneLines->Generate(m_conePnts, Vec3(0.15f), DrawType::Line, 1.0f);

      mp->m_subMeshes.push_back(m_coneLines->GetMesh());
      mp->m_subMeshes[2]->m_material->Init();

      // Render gizmo
      renderer->Render(this, viewport->GetCamera());
    }
  }  // namespace Editor
}  // namespace ToolKit
