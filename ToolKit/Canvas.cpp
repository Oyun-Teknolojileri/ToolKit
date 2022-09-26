#include "Canvas.h"

#include "Material.h"
#include "Mesh.h"
#include "Node.h"
#include "Texture.h"
#include "ToolKit.h"
#include "Viewport.h"

#include <memory>

#include "DebugNew.h"

namespace ToolKit
{
  // Canvas
  //////////////////////////////////////////

  Canvas::Canvas() : Surface()
  {
    ParameterConstructor();
    ParameterEventConstructor();
    CreateQuadLines();
  }

  Canvas::Canvas(const Vec2& size) : Surface(size)
  {
    ParameterConstructor();
    ParameterEventConstructor();
    CreateQuadLines();
  }

  EntityType Canvas::GetType() const
  {
    return EntityType::Entity_Canvas;
  }

  void Canvas::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    Surface::Serialize(doc, parent);
  }

  void Canvas::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Surface::DeSerialize(doc, parent);
    ParameterEventConstructor();
    CreateQuadLines();
  }

  void Canvas::ParameterConstructor()
  {
    // Update surface params.
    ParamMaterial().m_exposed     = false;
    ParamSize().m_category        = CanvasCategory;
    ParamPivotOffset().m_category = CanvasCategory;
  }

  void Canvas::ParameterEventConstructor()
  {
    Surface::ParameterEventConstructor();
    ParamMaterial().m_onValueChangedFn.clear();
  }

  void Canvas::UpdateGeometry(bool byTexture)
  {
    CreateQuadLines();
  }

  void Canvas::ApplyRecursivResizePolicy(float width, float height)
  {
    const Vec2 size = GetSizeVal();
    Vec3 scale1     = m_node->GetScale();
    scale1.x        = width / size.x;
    scale1.y        = height / size.y;
    m_node->SetScale(scale1);

    for (Node* childNode : m_node->m_children)
    {
      if (Entity* ntt = childNode->m_entity)
      {
        if (ntt->IsSurfaceInstance())
        {
          Surface* surface     = static_cast<Surface*>(ntt);
          const float* offsets = surface->m_anchorParams.m_offsets;

          Vec3 canvasPoints[4], surfacePoints[4];
          surface->CalculateAnchorOffsets(canvasPoints, surfacePoints);

          const Quaternion r =
              childNode->GetOrientation(TransformationSpace::TS_WORLD);
          const Vec3 eularXYZ = glm::eulerAngles(r);
          const float angle   = (eularXYZ.z);
          Mat4 orientation(1.f);
          orientation = glm::rotate(orientation, angle, Vec3(0.f, 0.f, 1.f));

          {
            Vec4 size(surface->GetSizeVal(), 0.f, 1.f);
            size                 = orientation * size;
            const float ww       = fabs(size.x);
            const float hh       = fabs(size.y);
            const BoundingBox bb = surface->GetAABB(true);

            const float innerWidth  = canvasPoints[1].x - canvasPoints[0].x;
            const float innerHeight = canvasPoints[0].y - canvasPoints[2].y;

            const float offsetWidthRatio =
                ((innerWidth - (offsets[2] + offsets[3])) / ww);
            const float offsetHeightRatio =
                (innerHeight - ((offsets[0]) + (offsets[1]))) / hh;

            Vec3 scale(1.f, 1.f, 1.f);

            if (offsetWidthRatio > 0.f)
              scale.x = offsetWidthRatio;
            else if (offsetWidthRatio > -0.00000001f)
              scale.x = childNode->GetScale().x;
            if (offsetHeightRatio > 0.f)
              scale.y = offsetHeightRatio;
            else if (offsetHeightRatio > -0.00000001f)
              scale.y = childNode->GetScale().y;

            Vec4 scale_ = Vec4(scale, 1.f);
            scale_      = orientation * scale_;
            scale.x     = fabs(scale_.x);
            scale.y     = fabs(scale_.y);

            childNode->SetScale(scale);

            if (offsetWidthRatio > 0.f)
            {
              surface->m_anchorParams.lockWidth = false;
            }
            else
            {
              surface->m_anchorParams.lockWidth = true;
            }

            if (offsetHeightRatio > 0.f)
            {
              surface->m_anchorParams.lockHeight = false;
            }
            else
            {
              surface->m_anchorParams.lockHeight = true;
            }
          }

          {

            const BoundingBox bb = surface->GetAABB(true);
            surface->CalculateAnchorOffsets(canvasPoints, surfacePoints);
            Vec4 ax1_(1.f, 0.f, 0.f, 1.f);
            Vec4 ax2_(0.f, 1.f, 0.f, 1.f);

            ax1_ = orientation * ax1_;
            ax2_ = orientation * ax2_;

            Vec3 ax1 = Vec3(ax1_);
            Vec3 ax2 = Vec3(ax2_);
            Vec3 translate =
                (canvasPoints[0] + ax1 * offsets[2] - ax2 * offsets[0]);
            translate -= surfacePoints[0];
            /*              canvasPoints[0] +
                          ax1 *
                              (offsets[2] +
                               (surface->GetPivotOffsetVal().x) * bb.GetWidth())
               - ax2 * (offsets[0] + (1.f - surface->GetPivotOffsetVal().y) *
               bb.GetHeight());*/

            const Vec3 surfacePos =
                surface->m_node->GetTranslation(TransformationSpace::TS_WORLD);
            translate += surfacePos;
            translate.z = surfacePos.z;
            childNode->SetTranslation(translate, TransformationSpace::TS_WORLD);
          }

          if (surface->GetType() == EntityType::Entity_Canvas)
          {
            Canvas* canvasPanel  = static_cast<Canvas*>(surface);
            const BoundingBox bb = canvasPanel->GetAABB(true);
            canvasPanel->ApplyRecursivResizePolicy(bb.GetWidth(),
                                                   bb.GetHeight());
          }
        }
      }
    }
  }

  void Canvas::CreateQuadLines()
  {
    float width  = GetSizeVal().x;
    float height = GetSizeVal().y;
    float depth  = 0;
    Vec2 absOffset =
        Vec2(GetPivotOffsetVal().x * width, GetPivotOffsetVal().y * height);

    VertexArray vertices;
    vertices.resize(8);
    vertices[0].pos = Vec3(-absOffset.x, -absOffset.y, depth);
    vertices[1].pos = Vec3(width - absOffset.x, -absOffset.y, depth);
    vertices[2].pos = Vec3(width - absOffset.x, -absOffset.y, depth);
    vertices[3].pos = Vec3(width - absOffset.x, height - absOffset.y, depth);
    vertices[4].pos = Vec3(width - absOffset.x, height - absOffset.y, depth);
    vertices[5].pos = Vec3(-absOffset.x, height - absOffset.y, depth);
    vertices[6].pos = Vec3(-absOffset.x, height - absOffset.y, depth);
    vertices[7].pos = Vec3(-absOffset.x, -absOffset.y, depth);

    MeshPtr mesh               = std::make_shared<Mesh>();
    mesh->m_clientSideVertices = vertices;
    mesh->CalculateAABB();
    mesh->Init();
    GetMeshComponent()->SetMeshVal(mesh);

    MaterialPtr material = GetMaterialComponent()->GetMaterialVal();
    material->UnInit();
    material->GetRenderState()->drawType  = DrawType::Line;
    material->GetRenderState()->lineWidth = 3.f;
    material->Init();
  }

} // namespace ToolKit
