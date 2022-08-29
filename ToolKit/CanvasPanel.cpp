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

  CanvasPanel::CanvasPanel()
      : Surface()
  {
      ParameterConstructor();
      ParameterEventConstructor();
      ResetCallbacks();
  }

  CanvasPanel::CanvasPanel(const Vec2& size)
      : Surface(size)
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
      // Update surface params.
      ParamMaterial().m_exposed = false;
      ParamSize().m_category = CanvasPanelCategory;
      ParamPivotOffset().m_category = CanvasPanelCategory;

      MaterialPtr material =
          GetMaterialManager()->GetCopyOfUnlitColorMaterial();
      material->GetRenderState()->drawType = DrawType::Line;
      material->GetRenderState()->lineWidth = 3.f;

      // Define button params.
      CanvasPanelMaterial_Define
      (
          material,
          CanvasPanelCategory.Name,
          CanvasPanelCategory.Priority,
          true,
          true
      );
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
      mesh->Init();
  }

  void CanvasPanel::CreateQuadLines()
  {
      float width = GetSizeVal().x;
      float height = GetSizeVal().y;
      float depth = 0;
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

      MeshPtr mesh = GetMeshComponent()->GetMeshVal();
      mesh->m_clientSideVertices = vertices;
      mesh->CalculateAABB();

      MaterialPtr material = GetMaterialComponent()->GetMaterialVal();
      material->GetRenderState()->drawType = DrawType::Line;
      material->GetRenderState()->lineWidth = 3.f;
  }
}  // namespace ToolKit

