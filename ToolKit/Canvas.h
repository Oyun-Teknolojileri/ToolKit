#pragma once

#include "Entity.h"
#include "Events.h"
#include "MathUtil.h"
#include "Resource.h"
#include "SpriteSheet.h"
#include "Surface.h"
#include "Types.h"

#include <functional>
#include <vector>

namespace ToolKit
{
  static VariantCategory CanvasCategory{"Canvas", 90};

  class TK_API Canvas : public Surface
  {
   public:
    Canvas();
    explicit Canvas(const Vec2& size);
    EntityType GetType() const override;
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

    void UpdateGeometry(bool byTexture) override;
    void ApplyRecursiveResizePolicy(float width, float height);

   protected:
    void ParameterConstructor();
    void ParameterEventConstructor();

   private:
    void CreateQuadLines();

   public:
    TKDeclareParam(MaterialPtr, CanvasPanelMaterial);

    // Local events.
    SurfaceEventCallback m_onMouseEnterLocal;
    SurfaceEventCallback m_onMouseExitLocal;
  };
} //  namespace ToolKit
