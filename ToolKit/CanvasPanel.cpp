#include "CanvasPanel.h"

#include <memory>

#include "Mesh.h"
#include "Texture.h"
#include "Material.h"
#include "Node.h"
#include "Viewport.h"
#include "ToolKit.h"
#include "DebugNew.h"

namespace ToolKit
{
  // CanvasPanel
  //////////////////////////////////////////

  CanvasPanel::CanvasPanel() : Surface()
  {
    ParameterConstructor();
    ParameterEventConstructor();
    ResetCallbacks();
  }

  CanvasPanel::CanvasPanel(const Vec2& size) : Surface(size)
  {
    ParameterConstructor();
    ParameterEventConstructor();
    ResetCallbacks();
  }

  EntityType CanvasPanel::GetType() const
  {
    return EntityType::Entity_CanvasPanel;
  }

  void CanvasPanel::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    Surface::Serialize(doc, parent);
  }

  void CanvasPanel::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Surface::DeSerialize(doc, parent);
    ParameterEventConstructor();
  }

  void CanvasPanel::ResetCallbacks()
  {
    Surface::ResetCallbacks();
  }

  void CanvasPanel::ParameterConstructor()
  {
    MaterialPtr nttMat;
    if (MaterialComponentPtr matCom = GetComponent<MaterialComponent>())
    {
      if (nttMat = matCom->GetMaterialVal())
      {
        // nttMat->GetRenderState()->drawType = DrawType::LineStrip;
        nttMat->m_color = {0.01f, 0.01f, 0.9f};
      }
    }
    // Update surface params.
    ParamMaterial().m_exposed     = false;
    ParamSize().m_category        = CanvasPanelCategory;
    ParamPivotOffset().m_category = CanvasPanelCategory;

    MaterialPtr material = GetMaterialManager()->GetCopyOfUnlitColorMaterial();
    material->UnInit();
    material->GetRenderState()->drawType  = DrawType::Line;
    material->GetRenderState()->lineWidth = 3.f;
    material->Init();
    // mat->GetRenderState()->drawType = DrawType::Line;
    //  Define button params.
    CanvasPanelMaterial_Define(material,
                               CanvasPanelCategory.Name,
                               CanvasPanelCategory.Priority,
                               true,
                               true);
  }

  void CanvasPanel::ParameterEventConstructor()
  {
    // Always rewire events for correctness.
    Surface::ParameterEventConstructor();

    ParamCanvasPanelMaterial().m_onValueChangedFn = nullptr;
  }

  void CanvasPanel::UpdateGeometry(bool byTexture)
  {
    MeshPtr mesh = GetMeshComponent()->GetMeshVal();
    mesh->UnInit();
    CreateQuadLines();
    mesh->Init(false);
  }

  void CanvasPanel::ApplyRecursivResizePolicy(float width, float height)
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

          if (surface->GetType() == EntityType::Entity_CanvasPanel)
          {
            CanvasPanel* canvasPanel = static_cast<CanvasPanel*>(surface);
            const BoundingBox bb     = canvasPanel->GetAABB(true);
            canvasPanel->ApplyRecursivResizePolicy(bb.GetWidth(),
                                                   bb.GetHeight());
          }
        }
      }
    }
  }

  void CanvasPanel::CreateQuadLines()
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

    MeshPtr mesh               = GetMeshComponent()->GetMeshVal();
    mesh->m_clientSideVertices = vertices;
    mesh->CalculateAABB();

    MaterialPtr material = GetMaterialComponent()->GetMaterialVal();
    material->UnInit();
    material->GetRenderState()->drawType  = DrawType::Line;
    material->GetRenderState()->lineWidth = 3.f;
    material->Init();
  }

} // namespace ToolKit
