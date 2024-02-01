/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Gizmo.h"

#include "App.h"
#include "EditorViewport2d.h"

#include <Material.h>
#include <MathUtil.h>
#include <Mesh.h>

#include <DebugNew.h>

namespace ToolKit
{
  namespace Editor
  {
    TKDefineClass(EditorBillboardBase, Billboard);

    EditorBillboardBase::EditorBillboardBase() {}

    EditorBillboardBase::EditorBillboardBase(const Settings& settings) : Billboard(settings) {}

    void EditorBillboardBase::NativeConstruct()
    {
      Super::NativeConstruct();
      Generate();
    }

    void EditorBillboardBase::Generate()
    {
      MeshComponentPtr mCom = GetComponent<MeshComponent>();

      // Billboard
      QuadPtr quad          = MakeNewPtr<Quad>();
      MeshPtr meshPtr       = quad->GetMeshComponent()->GetMeshVal();
      MaterialPtr matPtr    = GetMaterialManager()->GetCopyOfUnlitMaterial();
      matPtr->UnInit();
      matPtr->m_diffuseTexture                    = m_iconImage;
      matPtr->GetRenderState()->blendFunction     = BlendFunction::ALPHA_MASK;
      matPtr->GetRenderState()->alphaMaskTreshold = 0.1f;
      meshPtr->m_material                         = matPtr;
      mCom->SetMeshVal(meshPtr);
    }

    TKDefineClass(Cursor, EditorBillboardBase);

    Cursor::Cursor() : EditorBillboardBase({true, 10.0f, 60.0f, true}) {}

    Cursor::~Cursor() {}

    EditorBillboardBase::BillboardType Cursor::GetBillboardType() const { return BillboardType::Cursor; }

    void Cursor::Generate()
    {
      MeshComponentPtr parentMeshComp = GetComponent<MeshComponent>();
      MeshPtr parentMesh              = parentMeshComp->GetMeshVal();
      parentMesh->UnInit();

      // Billboard
      QuadPtr quad       = MakeNewPtr<Quad>();
      MeshPtr meshPtr    = quad->GetMeshComponent()->GetMeshVal();
      MaterialPtr matPtr = GetMaterialManager()->GetCopyOfUnlitMaterial();
      matPtr->UnInit();
      matPtr->m_diffuseTexture =
          GetTextureManager()->Create<Texture>(TexturePath(ConcatPaths({"Icons", "cursor4k.png"}), true));
      matPtr->GetRenderState()->blendFunction     = BlendFunction::ALPHA_MASK;
      matPtr->GetRenderState()->alphaMaskTreshold = 0.1f;
      matPtr->Init();
      meshPtr->m_material = matPtr;
      meshPtr->Init();
      parentMesh->m_subMeshes.push_back(meshPtr);

      // Lines
      VertexArray vertices;
      vertices.resize(8);

      vertices[0].pos.x                       = 0.2f;
      vertices[1].pos.x                       = 0.5f;

      vertices[2].pos.x                       = -0.2f;
      vertices[3].pos.x                       = -0.5f;

      vertices[4].pos.y                       = 0.2f;
      vertices[5].pos.y                       = 0.5f;

      vertices[6].pos.y                       = -0.2f;
      vertices[7].pos.y                       = -0.5f;

      MaterialPtr newMaterial                 = GetMaterialManager()->GetCopyOfUnlitColorMaterial();
      newMaterial->m_color                    = Vec3(0.1f, 0.1f, 0.1f);
      newMaterial->GetRenderState()->drawType = DrawType::Line;

      parentMesh->m_clientSideVertices        = vertices;
      parentMesh->m_material                  = newMaterial;

      parentMesh->Init();

      parentMesh->CalculateAABB();
    }

    TKDefineClass(Axis3d, EditorBillboardBase);

    Axis3d::Axis3d() : EditorBillboardBase({false, 10.0f, 60.0f, true}) {}

    Axis3d::~Axis3d() {}

    EditorBillboardBase::BillboardType Axis3d::GetBillboardType() const { return BillboardType::Axis3d; }

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

        Arrow2dPtr arrow = MakeNewPtr<Arrow2d>();
        arrow->Generate(t);

        MeshComponentPtr arrowMeshComp = arrow->GetComponent<MeshComponent>();
        MeshPtr arrowMesh              = arrowMeshComp->GetMeshVal();
        if (i == 0)
        {
          GetMeshComponent()->SetMeshVal(arrowMesh);
        }
        else
        {
          GetMeshComponent()->GetMeshVal()->m_subMeshes.push_back(arrowMesh);
        }
      }
    }

    // GizmoHandle
    //////////////////////////////////////////////////////////////////////////

    GizmoHandle::GizmoHandle() {}

    GizmoHandle::~GizmoHandle() {}

    void GizmoHandle::Generate(const Params& params)
    {
      m_params          = params;

      Vec3 dir          = AXIS[(int) params.axis % 3];
      Vec3Array pnts    = {dir * params.toeTip.x, dir * params.toeTip.y};

      m_mesh            = nullptr;

      LineBatchPtr line = MakeNewPtr<LineBatch>();
      line->Generate(pnts, params.color, DrawType::Line, 2.0f);
      MeshPtr lnMesh = line->GetComponent<MeshComponent>()->GetMeshVal();
      m_mesh = lnMesh;

      MaterialPtr material = GetMaterialManager()->GetCopyOfUnlitColorMaterial(false);
      material->m_color    = params.color;

      if (params.type == SolidType::Cube)
      {
        CubePtr solid = MakeNewPtr<Cube>();
        solid->SetCubeScaleVal(params.solidDim);

        MeshPtr mesh     = solid->GetComponent<MeshComponent>()->GetMeshVal();
        mesh->m_material = material;
        m_mesh->m_subMeshes.push_back(mesh);
      }
      else if (params.type == SolidType::Cone)
      {
        ConePtr solid = MakeNewPtr<Cone>();
        solid->Generate(params.solidDim.y, params.solidDim.x, 10, 10);

        MeshPtr mesh     = solid->GetComponent<MeshComponent>()->GetMeshVal();
        mesh->m_material = material;
        m_mesh->m_subMeshes.push_back(mesh);
      }
      else
      {
        assert(false);
        return;
      }

      MeshPtr mesh = m_mesh->m_subMeshes.back();
      mesh->UnInit();
      for (Vertex& v : mesh->m_clientSideVertices)
      {
        v.pos.y += params.toeTip.y;
        switch (params.axis)
        {
        case AxisLabel::X:
          v.pos = Vec3(v.pos.y, v.pos.x, v.pos.z);
          break;
        case AxisLabel::Z:
          v.pos = Vec3(v.pos.z, v.pos.x, v.pos.y);
          break;
        case AxisLabel::Y:
        default:
          break;
        }
      }
      mesh->Init();

      // Guide line.
      if (!glm::isNull(params.grabPnt, glm::epsilon<float>()))
      {
        int axisInd        = (int) m_params.axis;
        Vec3 axis          = AXIS[axisInd];
        Vec3Array pnts     = {axis * 999.0f, axis * -999.0f};

        LineBatchPtr guide = MakeNewPtr<LineBatch>();
        guide->Generate(pnts, g_gizmoColor[axisInd % 3], DrawType::Line, 1.0f);
        MeshPtr guideMesh = guide->GetComponent<MeshComponent>()->GetMeshVal();
        m_mesh->m_subMeshes.push_back(guideMesh);
      }
    }

    bool GizmoHandle::HitTest(const Ray& ray, float& t) const
    {
      // Hit test done in object space bounding boxes.
      Mat4 transform = GetTransform();
      Mat4 its       = glm::inverse(transform);

      Ray rayInObj;
      rayInObj.position  = its * Vec4(ray.position, 1.0f);
      rayInObj.direction = its * Vec4(ray.direction, 0.0f);

      m_mesh->CalculateAABB();
      return RayBoxIntersection(rayInObj, m_mesh->m_aabb, t);
    }

    Mat4 GizmoHandle::GetTransform() const
    {
      Mat4 sc        = glm::scale(Mat4(), m_params.scale);
      Mat4 rt        = Mat4(m_params.normals);
      Mat4 ts        = glm::translate(Mat4(), m_params.translate);
      Mat4 transform = ts * rt * sc;

      return transform;
    }

    void PolarHandle::Generate(const Params& params)
    {
      m_params        = params;

      int cornerCount = 60;
      Vec3Array corners;
      corners.reserve(cornerCount + 1);

      float deltaAngle = glm::two_pi<float>() / cornerCount;
      for (int i = 0; i < cornerCount; i++)
      {
        float angle = deltaAngle * i;
        corners.push_back(Vec3(glm::cos(angle), glm::sin(angle), 0.0f));

        switch (params.axis)
        {
        case AxisLabel::X:
          corners[i] = Vec3(corners[i].z, corners[i].y, corners[i].x);
          break;
        case AxisLabel::Y:
          corners[i] = Vec3(corners[i].x, corners[i].z, corners[i].y);
          break;
        case AxisLabel::Z:
          break;
        default:
          assert(false);
          break;
        }
      }
      corners.push_back(corners.front());

      LineBatchPtr circle = MakeNewPtr<LineBatch>();
      circle->Generate(corners, params.color, DrawType::LineStrip, 4.0f);
      MeshPtr circleMesh = circle->GetComponent<MeshComponent>()->GetMeshVal();
      m_mesh             = circleMesh;

      // Guide line.
      if (!glm::isNull(params.grabPnt, glm::epsilon<float>()))
      {
        // Bring the grab point to object space.
        Vec3 glcl    = params.grabPnt - params.worldLoc;
        glcl         = glm::normalize(glm::inverse(params.normals) * glcl);

        int axisIndx = static_cast<int>(params.axis);
        Vec3 axis    = AXIS[axisIndx];

        // Neighbor points for parallel line.
        Vec3 p1      = glm::normalize(glm::angleAxis(0.0001f, axis) * glcl);
        Vec3 p2      = glm::normalize(glm::angleAxis(-0.0001f, axis) * glcl);
        Vec3 dir     = glm::normalize(p1 - p2);
        m_tangentDir = glm::normalize(params.normals * dir);

        Vec3Array pnts;
        pnts.push_back(glcl + dir * 999.0f);
        pnts.push_back(glcl - dir * 999.0f);

        LineBatchPtr guide = MakeNewPtr<LineBatch>();
        guide->Generate(pnts, g_gizmoColor[axisIndx], DrawType::Line, 1.0f);

        MeshPtr guideMesh = guide->GetComponent<MeshComponent>()->GetMeshVal();
        m_mesh->m_subMeshes.push_back(guideMesh);
      }
    }

    bool PolarHandle::HitTest(const Ray& ray, float& t) const
    {
      t              = TK_FLT_MAX;

      Mat4 sc        = glm::scale(Mat4(), m_params.scale);
      Mat4 rt        = Mat4(m_params.normals);
      Mat4 ts        = glm::translate(Mat4(), m_params.translate);
      Mat4 transform = ts * rt * sc;
      Mat4 its       = glm::inverse(transform);

      Ray rayInObj;
      rayInObj.position  = its * Vec4(ray.position, 1.0f);
      rayInObj.direction = its * Vec4(ray.direction, 0.0f);

      for (size_t i = 1; i < m_mesh->m_clientSideVertices.size(); i++)
      {
        Vec3& v1       = m_mesh->m_clientSideVertices[i - 1].pos;
        Vec3& v2       = m_mesh->m_clientSideVertices[i].pos;
        Vec3 mid       = (v1 + v2) * 0.5f;
        BoundingBox bb = {mid - Vec3(0.05f), mid + Vec3(0.05f)};

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
      m_params                             = params;

      QuadPtr solid                        = MakeNewPtr<Quad>();
      MaterialPtr material                 = GetMaterialManager()->GetCopyOfUnlitColorMaterial(false);
      material->m_color                    = params.color;
      material->GetRenderState()->cullMode = CullingType::TwoSided;

      MeshPtr mesh                         = solid->GetMeshComponent()->GetMeshVal();
      mesh->m_material                     = material;
      m_mesh                               = mesh;

      float scale                          = 0.15f;
      float offset                         = 2.0f;

      m_mesh->UnInit();
      for (Vertex& v : m_mesh->m_clientSideVertices)
      {
        v.pos.y += params.toeTip.y;
        switch (params.axis)
        {
        case AxisLabel::XY:
          v.pos   *= scale;
          v.pos.x += 0.75f * scale;
          v.pos   += Vec3(Vec2(offset * scale), 0.0f);
          break;
        case AxisLabel::YZ:
          v.pos    = Vec3(v.pos.z, v.pos.y, v.pos.x) * scale;
          v.pos.z += 0.75f * scale;
          v.pos   += Vec3(0.0f, Vec2(offset * scale));
          break;
        case AxisLabel::ZX:
          v.pos    = Vec3(v.pos.x, v.pos.z, v.pos.y) * scale;
          v.pos.x += 0.75f * scale;
          v.pos   += Vec3(offset * scale) * Vec3(1.0f, 0.0f, 1.0f);
          break;
        default:
          break;
        }
      }
      m_mesh->Init();

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
          guides[i] = new LineBatch // Use MakeNewPtr if this comment is opened
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
      Mat4 sc        = glm::scale(Mat4(), m_params.scale);
      Mat4 rt        = Mat4(m_params.normals);
      Mat4 ts        = glm::translate(Mat4(), m_params.translate);
      Mat4 transform = ts * rt * sc;
      Mat4 its       = glm::inverse(transform);

      Ray rayInObj;
      rayInObj.position  = its * Vec4(ray.position, 1.0f);
      rayInObj.direction = its * Vec4(ray.direction, 0.0f);

      m_mesh->CalculateAABB();
      return RayBoxIntersection(rayInObj, m_mesh->m_aabb, t);
    }

    // Gizmo
    //////////////////////////////////////////////////////////////////////////

    TKDefineClass(Gizmo, EditorBillboardBase);

    Gizmo::Gizmo() {}

    Gizmo::Gizmo(const Billboard::Settings& set) : EditorBillboardBase(set) {}

    Gizmo::~Gizmo()
    {
      for (size_t i = 0; i < m_handles.size(); i++)
      {
        SafeDel(m_handles[i]);
      }
    }

    void Gizmo::NativeConstruct()
    {
      Super::NativeConstruct();
      Update(0.0f);
    }

    EditorBillboardBase::BillboardType Gizmo::GetBillboardType() const { return BillboardType::Gizmo; }

    AxisLabel Gizmo::HitTest(const Ray& ray) const
    {
      float t, tMin = TK_FLT_MAX;
      AxisLabel hit = AxisLabel::None;
      for (size_t i = 0; i < m_handles.size(); i++)
      {
        if (!m_handles[i]->m_mesh)
        {
          continue;
        }
        if (m_handles[i]->HitTest(ray, t))
        {
          if (t < tMin)
          {
            tMin = t;
            hit  = static_cast<AxisLabel>(m_handles[i]->m_params.axis);
          }
        }
      }

      return hit;
    }

    bool Gizmo::IsLocked(AxisLabel axis) const { return contains(m_lockedAxis, axis); }

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
      m_lockedAxis.erase(std::remove(m_lockedAxis.begin(), m_lockedAxis.end(), axis), m_lockedAxis.end());
    }

    bool Gizmo::IsGrabbed(AxisLabel axis) const { return m_grabbedAxis == axis; }

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

    AxisLabel Gizmo::GetGrabbedAxis() const { return m_grabbedAxis; }

    void Gizmo::LookAt(CameraPtr cam, float windowHeight)
    {
      Billboard::LookAt(cam, windowHeight);
      m_node->SetOrientation(glm::toQuat(m_normalVectors));
    }

    GizmoHandle::Params Gizmo::GetParam() const
    {
      GizmoHandle::Params p;
      p.normals    = m_normalVectors;
      p.worldLoc   = m_worldLocation;
      p.initialPnt = m_initialPoint;
      Mat4 ts      = m_node->GetTransform(TransformationSpace::TS_WORLD);
      DecomposeMatrix(ts, &p.translate, nullptr, &p.scale);

      return p;
    }

    // LinearGizmo
    //////////////////////////////////////////////////////////////////////////

    TKDefineClass(LinearGizmo, Gizmo);

    LinearGizmo::LinearGizmo() : Gizmo({false, 6.0f, 60.0f})
    {
      m_handles.resize(3);
      for (uint i = 0; i < 3; i++)
      {
        GizmoHandle* gizmo   = new GizmoHandle();
        gizmo->m_params.axis = (AxisLabel) i;
        gizmo->m_params.type = GizmoHandle::SolidType::Cone;
        m_handles[i]         = gizmo;
      }
    }

    LinearGizmo::~LinearGizmo() {}

    void LinearGizmo::Update(float deltaTime)
    {
      if (m_handles.empty())
      {
        return;
      }

      GizmoHandle::Params p = GetParam();

      for (size_t i = 0; i < m_handles.size(); i++)
      {
        GizmoHandle* handle = m_handles[i];
        AxisLabel axis      = handle->m_params.axis;
        p.type              = handle->m_params.type;

        if (m_grabbedAxis == axis)
        {
          p.color = g_selectHighLightPrimaryColor;
        }
        else if (axis != AxisLabel::XYZ)
        {
          p.color = g_gizmoColor[(int) axis % 3];
        }
        else
        {
          p.color  = Vec3(1.0f);
          p.toeTip = Vec3(0.0f);
          p.scale  = Vec3(0.8f);
        }

        if (IsLocked(axis))
        {
          p.color = g_gizmoLocked;
        }
        else if (m_lastHovered == axis)
        {
          p.color       = g_selectHighLightSecondaryColor;
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

      MeshPtr mesh = m_handles[0]->m_mesh;
      for (int i = 1; i < m_handles.size(); i++)
      {
        mesh->m_subMeshes.push_back(m_handles[i]->m_mesh);
      }
      mesh->Init(false);
      mesh->CalculateAABB();
      GetComponent<MeshComponent>()->SetMeshVal(mesh);
    }

    GizmoHandle::Params LinearGizmo::GetParam() const
    {
      const float tip = 0.8f, toe = 0.05f, rad = 0.1f;

      GizmoHandle::Params p;
      p.normals    = m_normalVectors;
      p.worldLoc   = m_worldLocation;
      p.initialPnt = m_initialPoint;
      Mat4 ts      = m_node->GetTransform(TransformationSpace::TS_WORLD);
      DecomposeMatrix(ts, &p.translate, nullptr, &p.scale);

      p.solidDim = Vec3(rad, 1.0f - tip, rad);
      p.toeTip   = Vec3(toe, tip, 0.0f);
      p.type     = GizmoHandle::SolidType::Cone;

      return p;
    }

    TKDefineClass(MoveGizmo, Gizmo);

    MoveGizmo::MoveGizmo()
    {
      for (int i = 3; i < 6; i++)
      {
        m_handles.push_back(new QuadHandle());
        m_handles[i]->m_params.axis = (AxisLabel) i;
      }
    }

    MoveGizmo::~MoveGizmo() {}

    EditorBillboardBase::BillboardType MoveGizmo::GetBillboardType() const { return BillboardType::Move; }

    TKDefineClass(ScaleGizmo, Gizmo);

    ScaleGizmo::ScaleGizmo()
    {
      for (uint i = 0; i < 3; i++)
      {
        m_handles[i]->m_params.type = GizmoHandle::SolidType::Cube;
      }

      for (int i = 3; i < 6; i++)
      {
        m_handles.push_back(new QuadHandle());
        m_handles[i]->m_params.axis = static_cast<AxisLabel>(i);
      }

      // Central uniform scale cube gizmo
      {
        m_handles.push_back(new GizmoHandle());
        GizmoHandle* cube    = m_handles[6];
        cube->m_params.axis  = AxisLabel::XYZ;
        cube->m_params.type  = GizmoHandle::SolidType::Cube;
        cube->m_params.color = Vec3(1.0);
        cube->m_params.scale = Vec3(5);
      }
    }

    ScaleGizmo::~ScaleGizmo() {}

    EditorBillboardBase::BillboardType ScaleGizmo::GetBillboardType() const { return BillboardType::Scale; }

    GizmoHandle::Params ScaleGizmo::GetParam() const
    {
      GizmoHandle::Params p = LinearGizmo::GetParam();
      p.solidDim            = Vec3(0.15f);
      p.type                = GizmoHandle::SolidType::Cube;

      return p;
    }

    TKDefineClass(PolarGizmo, Gizmo);

    PolarGizmo::PolarGizmo() : Gizmo({false, 6.0f, 60.0f})
    {
      m_handles = {new PolarHandle(), new PolarHandle(), new PolarHandle()};
    }

    PolarGizmo::~PolarGizmo() {}

    EditorBillboardBase::BillboardType PolarGizmo::GetBillboardType() const { return BillboardType::Rotate; }

    void PolarGizmo::Update(float deltaTime)
    {
      if (m_handles.empty())
      {
        return;
      }

      GizmoHandle::Params p = GetParam();

      // Clear all meshes
      for (int i = 0; i < 3; i++)
      {
        m_handles[i]->m_mesh = nullptr;
      }

      EditorViewport2d* viewport2D = dynamic_cast<EditorViewport2d*>(g_app->GetActiveViewport());
      for (int i = 0; i < 3; i++)
      {
        // If gizmo is in 2D view, just generate Z axis
        if (viewport2D && i != static_cast<int>(AxisLabel::Z))
        {
          continue;
        }
        if (m_grabbedAxis == static_cast<AxisLabel>(i))
        {
          p.color = g_selectHighLightPrimaryColor;
        }
        else
        {
          p.color = g_gizmoColor[i];
        }

        if (IsLocked(static_cast<AxisLabel>(i)))
        {
          p.color = g_gizmoLocked;
        }
        else if (m_lastHovered == static_cast<AxisLabel>(i))
        {
          p.color       = g_selectHighLightSecondaryColor;
          m_lastHovered = AxisLabel::None;
        }

        p.axis = static_cast<AxisLabel>(i);
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

      MeshPtr mesh = nullptr;
      for (int i = 0; i < m_handles.size(); i++)
      {
        if (m_handles[i]->m_mesh)
        {
          if (mesh == nullptr)
          {
            mesh = m_handles[i]->m_mesh;
          }
          else
          {
            mesh->m_subMeshes.push_back(m_handles[i]->m_mesh);
          }
        }
      }

      GetComponent<MeshComponent>()->SetMeshVal(mesh);
    }

    TKDefineClass(SkyBillboard, EditorBillboardBase);

    SkyBillboard::SkyBillboard() : EditorBillboardBase({true, 3.5f, 10.0f}) {}

    SkyBillboard::~SkyBillboard() {}

    EditorBillboardBase::BillboardType SkyBillboard::GetBillboardType() const { return BillboardType::Sky; }

    void SkyBillboard::Generate()
    {
      m_iconImage = UI::m_skyIcon;
      EditorBillboardBase::Generate();
    }

    TKDefineClass(LightBillboard, EditorBillboardBase);

    LightBillboard::LightBillboard() : EditorBillboardBase({true, 3.5f, 10.0f}) {}

    LightBillboard::~LightBillboard() {}

    EditorBillboardBase::BillboardType LightBillboard::GetBillboardType() const { return BillboardType::Light; }

    void LightBillboard::Generate()
    {
      m_iconImage = UI::m_lightIcon;
      EditorBillboardBase::Generate();
    }

  } // namespace Editor
} // namespace ToolKit
