#pragma once

#include <vector>
#include <functional>

#include "Entity.h"
#include "Events.h"
#include "MathUtil.h"
#include "Resource.h"
#include "SpriteSheet.h"
#include "Surface.h"
#include "Types.h"

namespace ToolKit
{
  static VariantCategory CanvasPanelCategory{"CanvasPanel", 90};

  class TK_API CanvasPanel : public Surface
  {
   public:
    CanvasPanel();
    explicit CanvasPanel(const Vec2& size);
    EntityType GetType() const override;
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;
    void ResetCallbacks() override;

    void UpdateGeometry(bool byTexture) override;
    void ApplyRecursivResizePolicy(float width, float height);

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
