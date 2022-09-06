
#include "Anchor.h"

#include "App.h"
#include "Camera.h"
#include "ConsoleWindow.h"
#include "EditorViewport.h"
#include "EditorViewport2d.h"
#include "GL/glew.h"
#include "GlobalDef.h"
#include "Material.h"
#include "Mesh.h"
#include "Node.h"
#include "Primative.h"
#include "RenderState.h"
#include "ResourceComponent.h"
#include "Surface.h"
#include "Texture.h"
#include "ToolKit.h"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <vector>

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    Anchor::Anchor(const Billboard::Settings& set) : EditorBillboardBase(set)
    {
      for (int i = 0; i < 9; i++)
      {
        m_handles.push_back(std::make_shared<AnchorHandle>());
        constexpr int val                = (int) DirectionLabel::N;
        m_handles[i]->m_params.direction = static_cast<DirectionLabel>(val + i);
      }

      Update(0.0f);
    }

    EditorBillboardBase::BillboardType Anchor::GetBillboardType() const
    {
      return BillboardType::Anchor;
    }

    DirectionLabel Anchor::HitTest(const Ray& ray) const
    {
      float t, tMin = TK_FLT_MAX;
      DirectionLabel hit = DirectionLabel::None;

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
            hit = static_cast<DirectionLabel>(m_handles[i]->m_params.direction);
          }
        }
      }

      return hit;
    }

    bool Anchor::IsGrabbed(DirectionLabel direction) const
    {
      return m_grabbedDirection == direction;
    }

    void Anchor::Grab(DirectionLabel direction)
    {
      m_grabbedDirection = direction;
    }

    DirectionLabel Anchor::GetGrabbedDirection() const
    {
      return m_grabbedDirection;
    }

    AnchorHandle::Params Anchor::GetParam() const
    {
      AnchorHandle::Params p;
      p.worldLoc = m_worldLocation;
      p.color    = g_anchorColor;
      Mat4 ts    = m_node->GetTransform(TransformationSpace::TS_WORLD);
      DecomposeMatrix(ts, &p.translate, nullptr, &p.scale);

      return p;
    }

    void Anchor::Update(float deltaTime)
    {
      if (m_entity == nullptr || !m_entity->IsSurfaceInstance() ||
          m_entity->m_node->m_parent == nullptr ||
          m_entity->m_node->m_parent->m_entity == nullptr ||
          m_entity->m_node->m_parent->m_entity->GetType() !=
              EntityType::Entity_CanvasPanel)
        return;

      CanvasPanel* canvasPanel =
          static_cast<CanvasPanel*>(m_entity->m_node->m_parent->m_entity);

      Vec3 pos;
      float w = 0, h = 0;
      {
        const BoundingBox bb = canvasPanel->GetAABB(true);
        w                    = bb.GetWidth();
        h                    = bb.GetHeight();
        pos                  = Vec3(bb.min.x, bb.max.y, pos.z);
      }

      Surface* surface    = static_cast<Surface*>(m_entity);
      float* anchorRatios = surface->m_anchorParams.m_anchorRatios;

      const Vec3 axis[3] = {
          {1.f, 0.f, 0.f},
          {0.f, 1.f, 0.f},
          {0.f, 0.f, 1.f}
      };

      Vec3 guideLines[8];
      {
        guideLines[0] = pos + axis[0] * ((1.f - anchorRatios[1]) * w);
        guideLines[1] = guideLines[0] - axis[1] * h;

        guideLines[2] = pos - axis[1] * (anchorRatios[2] * h);
        guideLines[3] = guideLines[2] + axis[0] * w;

        guideLines[4] = pos + axis[0] * ((anchorRatios[0]) * w);
        guideLines[5] = guideLines[4] - axis[1] * h;

        guideLines[6] = pos - axis[1] * ((1.f - anchorRatios[3]) * h);
        guideLines[7] = guideLines[6] + axis[0] * w;
      }

      float handleTranslate = 30.f;

      if ((anchorRatios[0] + anchorRatios[1] < 0.99f) ||
          (anchorRatios[2] + anchorRatios[3] < 0.99f))
      {
        handleTranslate = 20.f;
      }

      for (AnchorHandlePtr handle : m_handles)
      {
        AnchorHandle::Params p = GetParam();
        p.type                 = AnchorHandle::SolidType::Quad;

        p.worldLoc = pos;

        const DirectionLabel direction = handle->m_params.direction;

        if (m_grabbedDirection == direction)
        {
          p.color = g_selectHighLightPrimaryColor;
        }
        else
        {
          p.color = g_anchorColor;
        }

        if (m_lastHovered == direction)
        {
          p.color = g_selectHighLightSecondaryColor;
        }

        p.direction = direction;
        if (IsGrabbed(p.direction))
        {
          p.grabPnt = m_grabPoint;
        }
        else
        {
          p.grabPnt = ZERO;
        }

        if (direction == DirectionLabel::CENTER)
        {
          p.worldLoc -= axis[1] * ((anchorRatios[2]) * h);
          p.worldLoc += axis[0] * ((anchorRatios[0]) * w);

          if ((anchorRatios[0] + anchorRatios[1] < 0.99f) ||
              (anchorRatios[2] + anchorRatios[3] < 0.99f))
          {
            handle->m_mesh.reset();
            continue;
          }
          p.type = AnchorHandle::SolidType::Circle;
        }

        if (direction == DirectionLabel::NE)
        {
          p.worldLoc -= axis[1] * ((anchorRatios[2]) * h);
          p.worldLoc += axis[0] * ((1.f - anchorRatios[1]) * w);

          p.translate = Vec3(0.f, handleTranslate, 0.f);
          p.scale     = Vec3(0.5f, 1.1f, 1.f);
          p.angle     = glm::radians(-45.f);
        }
        if (direction == DirectionLabel::SE)
        {
          p.worldLoc -= axis[1] * ((1.f - anchorRatios[3]) * h);
          p.worldLoc += axis[0] * ((1.f - anchorRatios[1]) * w);

          p.translate = Vec3(0.f, -handleTranslate, 0.f);
          p.scale     = Vec3(0.5f, 1.1f, 1.f);
          p.angle     = glm::radians(45.f);
        }
        if (direction == DirectionLabel::NW)
        {
          p.worldLoc -= axis[1] * ((anchorRatios[2]) * h);
          p.worldLoc += axis[0] * ((anchorRatios[0]) * w);

          p.translate = Vec3(0.f, handleTranslate, 0.f);
          p.scale     = Vec3(0.5f, 1.1f, 1.f);
          p.angle     = glm::radians(45.f);
        }
        if (direction == DirectionLabel::SW)
        {
          p.worldLoc -= axis[1] * ((1.f - anchorRatios[3]) * h);
          p.worldLoc += axis[0] * ((anchorRatios[0]) * w);

          p.translate = Vec3(0.f, handleTranslate, 0.f);
          p.scale     = Vec3(0.5f, 1.1f, 1.f);
          p.angle     = glm::radians(135.f);
        }
        if (direction == DirectionLabel::E)
        {
          p.worldLoc -=
              axis[1] * ((anchorRatios[2] +
                          (1.f - anchorRatios[2] - anchorRatios[3]) / 2.f) *
                         h);
          p.worldLoc += axis[0] * ((1.f - anchorRatios[1]) * w);

          if (anchorRatios[2] + anchorRatios[3] < 0.99f)
          {
            handle->m_mesh.reset();
            continue;
          }
          p.translate = Vec3(30.f, 0.f, 0.f);
          p.scale     = Vec3(1.1f, 0.5f, 1.f);
        }
        if (direction == DirectionLabel::W)
        {
          p.worldLoc -=
              axis[1] * ((anchorRatios[2] +
                          (1.f - anchorRatios[2] - anchorRatios[3]) / 2.f) *
                         h);
          p.worldLoc += axis[0] * (anchorRatios[0] * w);

          if (anchorRatios[2] + anchorRatios[3] < 0.99f)
          {
            handle->m_mesh.reset();
            continue;
          }
          p.translate = Vec3(-30.f, 0.f, 0.f);
          p.scale     = Vec3(1.1f, 0.5f, 1.f);
        }
        if (direction == DirectionLabel::N)
        {
          p.worldLoc -= axis[1] * ((anchorRatios[2]) * h);
          p.worldLoc +=
              axis[0] * ((anchorRatios[0] +
                          (1.f - anchorRatios[0] - anchorRatios[1]) / 2.f) *
                         w);

          if (anchorRatios[0] + anchorRatios[1] < 1.f)
          {
            handle->m_mesh.reset();
            continue;
          }
          p.translate = Vec3(0.f, 30.f, 0.f);
          p.scale     = Vec3(0.5f, 1.1f, 1.f);
        }
        if (direction == DirectionLabel::S)
        {
          p.worldLoc +=
              axis[0] * ((anchorRatios[0] +
                          (1.f - anchorRatios[0] - anchorRatios[1]) / 2.f) *
                         w);
          p.worldLoc -= axis[1] * ((1.f - anchorRatios[3]) * h);

          if (anchorRatios[0] + anchorRatios[1] < 1.f)
          {
            handle->m_mesh.reset();
            continue;
          }
          p.translate = Vec3(0.f, -30.f, 0.f);
          p.scale     = Vec3(0.5f, 1.1f, 1.f);
        }

        if (EditorViewport* vp = g_app->GetActiveViewport())
        {
          constexpr float s = 30.f;
          p.translate *= vp->m_zoom;
          p.scale *= Vec3(s * vp->m_zoom, s * vp->m_zoom, 1.f);
          handle->Generate(p);
        }
      }

      MeshPtr mesh = std::make_shared<Mesh>();
      for (int i = 0; i < m_handles.size(); i++)
      {
        if (m_handles[i]->m_mesh)
          mesh->m_subMeshes.push_back(m_handles[i]->m_mesh);
      }

      {
        Vec3 canvasPoints[4], surfacePoints[4];
        surface->CalculateAnchorOffsets(canvasPoints, surfacePoints);

        //{
        //  Vec3Array pnts = { surfacePoints[0], surfacePoints[3],
        //    surfacePoints[1], surfacePoints[2] };

        //  LineBatch guide(
        //    pnts, Vec4(0.81f, 0.24f, 0.44f, 0.7f), DrawType::Line, 2.5f);
        //  MeshPtr guideMesh =
        //  guide.GetComponent<MeshComponent>()->GetMeshVal();
        //  mesh->m_subMeshes.push_back(guideMesh);
        //}

        // Vec3Array pnts = { canvasPoints[0], canvasPoints[3], canvasPoints[1],
        //   canvasPoints[2] };

        // LineBatch guide(
        //   pnts, Vec4(0.11f, 0.84f, 0.34f, 0.7f), DrawType::Line, 2.5f);
        // MeshPtr guideMesh =
        // guide.GetComponent<MeshComponent>()->GetMeshVal();
        // mesh->m_subMeshes.push_back(guideMesh);
      }

      if (m_lastHovered != DirectionLabel::None ||
          GetGrabbedDirection() != DirectionLabel::None)
      {
        Vec3Array pnts = {
            guideLines[0],
            guideLines[1],
            guideLines[2],
            guideLines[3],
            guideLines[4],
            guideLines[5],
            guideLines[6],
            guideLines[7],
        };

        LineBatch guide(pnts, g_anchorGuideLineColor, DrawType::Line, 2.5f);
        MeshPtr guideMesh = guide.GetComponent<MeshComponent>()->GetMeshVal();
        mesh->m_subMeshes.push_back(guideMesh);
      }

      m_lastHovered = DirectionLabel::None;

      mesh->Init(false);
      GetComponent<MeshComponent>()->SetMeshVal(mesh);
    }

    // AnchorHandle
    //////////////////////////////////////////////////////////////////////////

    AnchorHandle::AnchorHandle()
    {
      m_params.color = g_anchorColor;
    }

    void AnchorHandle::Generate(const Params& params)
    {
      m_params = params;

      MeshPtr meshPtr;
      if (params.type == AnchorHandle::SolidType::Quad)
      {
        Quad quad;
        meshPtr = quad.GetMeshComponent()->GetMeshVal();
      }
      else if (params.type == AnchorHandle::SolidType::Circle)
      {
        Sphere sphere(.35f);
        meshPtr = sphere.GetMeshComponent()->GetMeshVal();
      }
      else
      {
        assert(false); // A new primitive type?
      }

      Mat4 mat(1.0f);

      mat = glm::translate(mat, params.worldLoc);
      mat = glm::rotate(mat, params.angle, Vec3(0.f, 0.f, 1.f));
      mat = glm::translate(mat, params.translate);
      mat = glm::scale(mat, params.scale);
      meshPtr->ApplyTransform(mat);

      m_mesh = std::make_shared<Mesh>();
      m_mesh->m_subMeshes.push_back(meshPtr);
      m_mesh->Init(false);

      meshPtr->m_material = GetMaterialManager()->GetCopyOfUnlitColorMaterial();
      meshPtr->m_material->UnInit();
      meshPtr->m_material->m_color = params.color;
      meshPtr->m_material->Init();

      meshPtr->m_material->GetRenderState()->depthTestEnabled = false;
    }

    bool AnchorHandle::HitTest(const Ray& ray, float& t) const
    {
      // Hit test done in object space bounding boxes.
      Mat4 transform = GetTransform();
      Mat4 its       = glm::inverse(transform);

      Ray rayInObj;
      rayInObj.position  = Vec4(ray.position, 1.0f);
      rayInObj.direction = Vec4(ray.direction, 0.0f);

      m_mesh->CalculateAABB();
      return RayBoxIntersection(rayInObj, m_mesh->m_aabb, t);
    }

    Mat4 AnchorHandle::GetTransform() const
    {
      Mat4 sc        = glm::scale(Mat4(), m_params.scale);
      Mat4 rt        = Mat4(1.f);
      Mat4 ts        = glm::translate(Mat4(), m_params.translate);
      Mat4 transform = ts * rt * sc;

      return transform;
    }

  } // namespace Editor
} // namespace ToolKit
