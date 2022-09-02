
#include "Anchor.h"

#include <memory>
#include <vector>

#include "App.h"
#include "Camera.h"
#include "ConsoleWindow.h"
#include "DebugNew.h"
#include "EditorViewport.h"
#include "EditorViewport2d.h"
#include "Material.h"
#include "Mesh.h"
#include "Node.h"
#include "GL/glew.h"
#include <glm/gtc/matrix_transform.hpp>
#include "GlobalDef.h"
#include "Primative.h"
#include "RenderState.h"
#include "ResourceComponent.h"
#include "Surface.h"
#include "Texture.h"
#include "ToolKit.h"

namespace ToolKit
{
  namespace Editor
  {
    Anchor::Anchor(const Billboard::Settings& set)
      : EditorBillboardBase(set)
      , m_anchorRatios { 0.f }
    {
      for (int i = 0; i < 9; i++)
      {
        m_handles.push_back(std::make_shared<AnchorHandle>());
        constexpr int val = static_cast<int>(DirectionLabel::N);
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
      p.color = g_anchorColor;
      Mat4 ts = m_node->GetTransform(TransformationSpace::TS_WORLD);
      DecomposeMatrix(ts, &p.translate, nullptr, &p.scale);

      return p;
    }

    void Anchor::Update(float deltaTime)
    {
      Vec3 axes[3] = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f },
        { 0.f, 0.f, 1.f } };
      float w = 0, h = 0;
      Vec3 pos;
      if (m_entity && m_entity->IsSurfaceInstance())
      {
        Surface* surface = static_cast<Surface*>(m_entity);
        if (Entity* parent = surface->m_node->m_parent->m_entity)
        {
          if (parent->GetType() == EntityType::Entity_CanvasPanel)
          {
            CanvasPanel* canvasPanel = static_cast<CanvasPanel*>(parent);
            pos = canvasPanel->m_node->GetTranslation(
              TransformationSpace::TS_WORLD);
            w = canvasPanel->GetSizeVal().x;
            h = canvasPanel->GetSizeVal().y;
            pos -= Vec3(w / 2.f, -h / 2.f, 0.f);
          }
        }
      }

      Vec3 guideLines[8];
      {
        guideLines[0] = pos + axes[0] * ((1.f - m_anchorRatios[1]) * w);
        guideLines[1] = guideLines[0] - axes[1] * h;

        guideLines[2] = pos - axes[1] * (m_anchorRatios[2] * h);
        guideLines[3] = guideLines[2] + axes[0] * w;

        guideLines[4] = pos + axes[0] * ((m_anchorRatios[0]) * w);
        guideLines[5] = guideLines[4] - axes[1] * h;

        guideLines[6] = pos - axes[1] * ((1.f - m_anchorRatios[3]) * h);
        guideLines[7] = guideLines[6] + axes[0] * w;
      }

      float handleTranslate = 30.f;

      if ((m_anchorRatios[0] + m_anchorRatios[1] < 0.99f)
        || (m_anchorRatios[2] + m_anchorRatios[3] < 0.99f))
      {
        handleTranslate = 20.f;
      }

      for (AnchorHandlePtr handle : m_handles)
      {
        AnchorHandle::Params p = GetParam();
        p.type = AnchorHandle::SolidType::Quad;

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
          p.worldLoc -= axes[1] * ((m_anchorRatios[2]) * h);
          p.worldLoc += axes[0] * ((m_anchorRatios[0]) * w);

          if ((m_anchorRatios[0] + m_anchorRatios[1] < 0.99f)
            || (m_anchorRatios[2] + m_anchorRatios[3] < 0.99f))
          {
            handle->m_mesh.reset();
            continue;
          }
          p.type = AnchorHandle::SolidType::Circle;
        }

        if (direction == DirectionLabel::NE)
        {
          p.worldLoc -= axes[1] * ((m_anchorRatios[2]) * h);
          p.worldLoc += axes[0] * ((1.f - m_anchorRatios[1]) * w);

          p.translate = Vec3(0.f, handleTranslate, 0.f);
          p.scale = Vec3(0.5f, 1.1f, 1.f);
          p.angle = glm::radians(-45.f);
        }
        if (direction == DirectionLabel::SE)
        {
          p.worldLoc -= axes[1] * ((1.f - m_anchorRatios[3]) * h);
          p.worldLoc += axes[0] * ((1.f - m_anchorRatios[1]) * w);

          p.translate = Vec3(0.f, -handleTranslate, 0.f);
          p.scale = Vec3(0.5f, 1.1f, 1.f);
          p.angle = glm::radians(45.f);
        }
        if (direction == DirectionLabel::NW)
        {
          p.worldLoc -= axes[1] * ((m_anchorRatios[2]) * h);
          p.worldLoc += axes[0] * ((m_anchorRatios[0]) * w);

          p.translate = Vec3(0.f, handleTranslate, 0.f);
          p.scale = Vec3(0.5f, 1.1f, 1.f);
          p.angle = glm::radians(45.f);
        }
        if (direction == DirectionLabel::SW)
        {
          p.worldLoc -= axes[1] * ((1.f - m_anchorRatios[3]) * h);
          p.worldLoc += axes[0] * ((m_anchorRatios[0]) * w);

          p.translate = Vec3(0.f, handleTranslate, 0.f);
          p.scale = Vec3(0.5f, 1.1f, 1.f);
          p.angle = glm::radians(135.f);
        }
        if (direction == DirectionLabel::E)
        {
          p.worldLoc -= axes[1]
            * ((m_anchorRatios[2]
                 + (1.f - m_anchorRatios[2] - m_anchorRatios[3]) / 2.f)
              * h);
          p.worldLoc += axes[0] * ((1.f - m_anchorRatios[1]) * w);

          if (m_anchorRatios[2] + m_anchorRatios[3] < 0.99f)
          {
            handle->m_mesh.reset();
            continue;
          }
          p.translate = Vec3(30.f, 0.f, 0.f);
          p.scale = Vec3(1.1f, 0.5f, 1.f);
        }
        if (direction == DirectionLabel::W)
        {
          p.worldLoc -= axes[1]
            * ((m_anchorRatios[2]
                 + (1.f - m_anchorRatios[2] - m_anchorRatios[3]) / 2.f)
              * h);
          p.worldLoc += axes[0] * (m_anchorRatios[0] * w);

          if (m_anchorRatios[2] + m_anchorRatios[3] < 0.99f)
          {
            handle->m_mesh.reset();
            continue;
          }
          p.translate = Vec3(-30.f, 0.f, 0.f);
          p.scale = Vec3(1.1f, 0.5f, 1.f);
        }
        if (direction == DirectionLabel::N)
        {
          p.worldLoc -= axes[1] * ((m_anchorRatios[2]) * h);
          p.worldLoc += axes[0]
            * ((m_anchorRatios[0]
                 + (1.f - m_anchorRatios[0] - m_anchorRatios[1]) / 2.f)
              * w);

          if (m_anchorRatios[0] + m_anchorRatios[1] < 1.f)
          {
            handle->m_mesh.reset();
            continue;
          }
          p.translate = Vec3(0.f, 30.f, 0.f);
          p.scale = Vec3(0.5f, 1.1f, 1.f);
        }
        if (direction == DirectionLabel::S)
        {
          p.worldLoc += axes[0]
            * ((m_anchorRatios[0]
                 + (1.f - m_anchorRatios[0] - m_anchorRatios[1]) / 2.f)
              * w);
          p.worldLoc -= axes[1] * ((1.f - m_anchorRatios[3]) * h);

          if (m_anchorRatios[0] + m_anchorRatios[1] < 1.f)
          {
            handle->m_mesh.reset();
            continue;
          }
          p.translate = Vec3(0.f, -30.f, 0.f);
          p.scale = Vec3(0.5f, 1.1f, 1.f);
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

      if (m_lastHovered != DirectionLabel::None
        || GetGrabbedDirection() != DirectionLabel::None)
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

    AnchorHandle::AnchorHandle() { m_params.color = g_anchorColor; }

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
        assert(false);  // A new primitive type?
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
      Mat4 its = glm::inverse(transform);

      Ray rayInObj;
      rayInObj.position = Vec4(ray.position, 1.0f);
      rayInObj.direction = Vec4(ray.direction, 0.0f);

      m_mesh->CalculateAABB();
      return RayBoxIntersection(rayInObj, m_mesh->m_aabb, t);
    }

    Mat4 AnchorHandle::GetTransform() const
    {
      Mat4 sc = glm::scale(Mat4(), m_params.scale);
      Mat4 rt = Mat4(1.f);
      Mat4 ts = glm::translate(Mat4(), m_params.translate);
      Mat4 transform = ts * rt * sc;

      return transform;
    }
  }  // namespace Editor
}  // namespace ToolKit
