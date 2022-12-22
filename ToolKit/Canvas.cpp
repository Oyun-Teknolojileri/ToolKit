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

  EntityType Canvas::GetType() const { return EntityType::Entity_Canvas; }

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

  void Canvas::UpdateGeometry(bool byTexture) { CreateQuadLines(); }

  void Canvas::ApplyRecursiveResizePolicy(float width, float height)
  {
    const Vec2 canvasCurrentSize = GetSizeVal();
    Vec3 canvasScale             = m_node->GetScale();
    canvasScale.x                = width / canvasCurrentSize.x;
    canvasScale.y                = height / canvasCurrentSize.y;
    m_node->SetScale(canvasScale);

    for (Node* childNode : m_node->m_children)
    {
      if (Entity* ntt = childNode->m_entity)
      {
        if (ntt->IsSurfaceInstance())
        {
          Surface* surface     = static_cast<Surface*>(ntt);
          const float* offsets = surface->m_anchorParams.m_offsets;

          Vec3 currentScale(1.f, 1.f, 1.f);
          childNode->SetScale(currentScale);

          Vec3 canvasPoints[4], surfacePoints[4];
          surface->CalculateAnchorOffsets(canvasPoints, surfacePoints);

          // Scale operation
          {
            const Vec4 surfaceCurrentSize(surface->GetSizeVal(), 0.f, 1.f);
            const BoundingBox surfaceBB       = surface->GetAABB(true);

            const float surfaceAbsoluteWidth  = surfaceBB.GetWidth();
            const float surfaceAbsoluteHeight = surfaceBB.GetHeight();

            const float innerWidth  = canvasPoints[1].x - canvasPoints[0].x;
            const float innerHeight = canvasPoints[0].y - canvasPoints[2].y;

            const float offsetWidthRatio =
                ((innerWidth - (offsets[2] + offsets[3])) /
                 surfaceAbsoluteWidth);
            const float offsetHeightRatio =
                ((innerHeight - (offsets[0] + offsets[1])) /
                 surfaceAbsoluteHeight);

            Vec3 surfaceScale(1.f);
            surfaceScale.x = std::max(offsetWidthRatio, 0.0001f);
            surfaceScale.y = std::max(offsetHeightRatio, 0.0001f);

            Mat4 scaleMat;
            scaleMat = glm::scale(scaleMat, surfaceScale);
            childNode->Transform(scaleMat,
                                 TransformationSpace::TS_WORLD,
                                 false);
          }

          // Translate operation
          {
            surface->CalculateAnchorOffsets(canvasPoints, surfacePoints);

            const float innerWidth  = canvasPoints[1].x - canvasPoints[0].x;
            const float innerHeight = canvasPoints[0].y - canvasPoints[2].y;

            const Vec3 surfaceCurrentScale = childNode->GetScale();
            if (glm::epsilonNotEqual<float>(innerWidth, 0.0f, 0.00000001f) &&
                surfaceCurrentScale.x < 0.0f)
            {
              std::swap(surfacePoints[0], surfacePoints[1]);
              std::swap(surfacePoints[2], surfacePoints[3]);
            }

            if (glm::epsilonNotEqual<float>(innerHeight, 0.0f, 0.00000001f) &&
                surfaceCurrentScale.y < 0.0f)
            {
              std::swap(surfacePoints[0], surfacePoints[2]);
              std::swap(surfacePoints[1], surfacePoints[3]);
            }

            const Vec3 widthVector(1.f, 0.f, 0.f);
            const Vec3 heightVector(0.f, 1.f, 0.f);

            Vec3 translate = (canvasPoints[0] + widthVector * offsets[2] -
                              heightVector * offsets[0]);

            translate      -= surfacePoints[0];

            const Vec3 surfaceCurrentPos =
                surface->m_node->GetTranslation(TransformationSpace::TS_WORLD);

            translate   += surfaceCurrentPos;
            translate.z = surfaceCurrentPos.z;

            childNode->SetTranslation(translate, TransformationSpace::TS_WORLD);
          }

          if (surface->GetType() == EntityType::Entity_Canvas)
          {
            Canvas* canvasPanel  = static_cast<Canvas*>(surface);
            const BoundingBox bb = canvasPanel->GetAABB(true);
            canvasPanel->ApplyRecursiveResizePolicy(bb.GetWidth(),
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

    MeshPtr mesh    = std::make_shared<Mesh>();
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
