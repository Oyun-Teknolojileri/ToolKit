/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Canvas.h"

#include "Material.h"
#include "Mesh.h"
#include "Node.h"
#include "Texture.h"
#include "ToolKit.h"
#include "Viewport.h"

namespace ToolKit
{
  // Canvas
  //////////////////////////////////////////

  TKDefineClass(Canvas, Surface);

  Canvas::Canvas() : Surface() {}

  void Canvas::NativeConstruct()
  {
    Super::NativeConstruct();

    // TODO: Quad lines are not needed for the game engine. This can be moved to EditorCanvas.
    m_canvasMaterial                              = GetMaterialManager()->GetCopyOfUnlitMaterial();
    m_canvasMaterial->m_name                      = "CavasBorder";
    m_canvasMaterial->GetRenderState()->drawType  = DrawType::Line;
    m_canvasMaterial->GetRenderState()->lineWidth = 3.0f;
    GetMaterialComponent()->SetFirstMaterial(m_canvasMaterial);

    CreateQuadLines();
  }

  void Canvas::ParameterConstructor()
  {
    Super::ParameterConstructor();

    // Update surface params.
    ParamMaterial().m_exposed     = false;
    ParamSize().m_category        = CanvasCategory;
    ParamPivotOffset().m_category = CanvasCategory;
  }

  void Canvas::ParameterEventConstructor()
  {
    Super::ParameterEventConstructor();
    ParamMaterial().m_onValueChangedFn.clear();
  }

  XmlNode* Canvas::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* surfaceNode = Super::SerializeImp(doc, parent);
    XmlNode* canvasNode  = CreateXmlNode(doc, StaticClass()->Name, surfaceNode);

    return canvasNode;
  }

  void Canvas::DeserializeComponents(const SerializationFileInfo& info, XmlNode* entityNode)
  {
    // Just keep using defaults.
  }

  XmlNode* Canvas::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    if (m_version >= TKV045)
    {
      return DeSerializeImpV045(info, parent);
    }

    // Old file, keep parsing.
    XmlNode* surfaceNode = Surface::DeSerializeImp(info, parent);
    CreateQuadLines();

    return surfaceNode;
  }

  XmlNode* Canvas::DeSerializeImpV045(const SerializationFileInfo& info, XmlNode* parent)
  {
    XmlNode* surfaceNode = Surface::DeSerializeImp(info, parent);
    CreateQuadLines();

    return surfaceNode->first_node(StaticClass()->Name.c_str());
  }

  void Canvas::UpdateGeometry(bool byTexture)
  {
    InvalidateSpatialCaches();
    CreateQuadLines();
  }

  void Canvas::ApplyRecursiveResizePolicy(float width, float height)
  {
    const Vec2 canvasCurrentSize = GetSizeVal();
    Vec3 canvasScale             = m_node->GetScale();
    canvasScale.x                = width / canvasCurrentSize.x;
    canvasScale.y                = height / canvasCurrentSize.y;
    m_node->SetScale(canvasScale);

    for (Node* childNode : m_node->m_children)
    {
      if (EntityPtr ntt = childNode->OwnerEntity())
      {
        if (Surface* surface = ntt->As<Surface>())
        {
          float* const offsets = surface->m_anchorParams.m_offsets;

          Vec3 currentScale(1.f, 1.f, 1.f);
          childNode->SetScale(currentScale);

          Vec3 canvasPoints[4], surfacePoints[4];
          surface->CalculateAnchorOffsets(canvasPoints, surfacePoints);

          // Scale operation
          {
            Vec4 surfaceCurrentSize(surface->GetSizeVal(), 0.f, 1.f);
            const BoundingBox& surfaceBB = surface->GetBoundingBox(true);

            float surfaceAbsoluteWidth   = surfaceBB.GetWidth();
            float surfaceAbsoluteHeight  = surfaceBB.GetHeight();
            float innerWidth             = canvasPoints[1].x - canvasPoints[0].x;
            float innerHeight            = canvasPoints[0].y - canvasPoints[2].y;
            float offsetWidthRatio       = ((innerWidth - (offsets[2] + offsets[3])) / surfaceAbsoluteWidth);
            float offsetHeightRatio      = ((innerHeight - (offsets[0] + offsets[1])) / surfaceAbsoluteHeight);

            Vec3 surfaceScale(1.f);
            surfaceScale.x = std::max(offsetWidthRatio, 0.0001f);
            surfaceScale.y = std::max(offsetHeightRatio, 0.0001f);

            Mat4 scaleMat;
            scaleMat = glm::scale(scaleMat, surfaceScale);
            childNode->Transform(scaleMat, TransformationSpace::TS_WORLD);
          }

          // Translate operation
          {
            surface->CalculateAnchorOffsets(canvasPoints, surfacePoints);

            float innerWidth         = canvasPoints[1].x - canvasPoints[0].x;
            float innerHeight        = canvasPoints[0].y - canvasPoints[2].y;

            Vec3 surfaceCurrentScale = childNode->GetScale();
            if (glm::epsilonNotEqual<float>(innerWidth, 0.0f, 0.00000001f) && surfaceCurrentScale.x < 0.0f)
            {
              std::swap(surfacePoints[0], surfacePoints[1]);
              std::swap(surfacePoints[2], surfacePoints[3]);
            }

            if (glm::epsilonNotEqual<float>(innerHeight, 0.0f, 0.00000001f) && surfaceCurrentScale.y < 0.0f)
            {
              std::swap(surfacePoints[0], surfacePoints[2]);
              std::swap(surfacePoints[1], surfacePoints[3]);
            }

            Vec3 widthVector(1.f, 0.f, 0.f);
            Vec3 heightVector(0.f, 1.f, 0.f);

            Vec3 translate          = (canvasPoints[0] + widthVector * offsets[2] - heightVector * offsets[0]);

            translate              -= surfacePoints[0];

            Vec3 surfaceCurrentPos  = surface->m_node->GetTranslation(TransformationSpace::TS_WORLD);

            translate              += surfaceCurrentPos;
            translate.z             = surfaceCurrentPos.z;

            childNode->SetTranslation(translate, TransformationSpace::TS_WORLD);
          }

          if (surface->IsA<Canvas>())
          {
            Canvas* canvasPanel   = static_cast<Canvas*>(surface);
            const BoundingBox& bb = canvasPanel->GetBoundingBox(true);
            canvasPanel->ApplyRecursiveResizePolicy(bb.GetWidth(), bb.GetHeight());
          }
        }
      }
    }
  }

  void Canvas::CreateQuadLines()
  {
    BoundingBox box = GetBoundingBox();
    Vec3 min        = box.min;
    Vec3 max        = box.max;
    float depth     = min.z;

    // Lines of the boundary.
    VertexArray vertices;
    vertices.resize(8);

    vertices[0].pos            = min;
    vertices[1].pos            = Vec3(max.x, min.y, depth);
    vertices[2].pos            = Vec3(max.x, min.y, depth);
    vertices[3].pos            = Vec3(max.x, max.y, depth);
    vertices[4].pos            = Vec3(max.x, max.y, depth);
    vertices[5].pos            = Vec3(min.x, max.y, depth);
    vertices[6].pos            = Vec3(min.x, max.y, depth);
    vertices[7].pos            = min;

    MeshPtr mesh               = MakeNewPtr<Mesh>();
    mesh->m_clientSideVertices = vertices;
    mesh->CalculateAABB();
    mesh->Init();
    GetMeshComponent()->SetMeshVal(mesh);
  }

} // namespace ToolKit
