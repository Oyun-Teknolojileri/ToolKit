#pragma once

#include "Entity.h"
#include "Events.h"
#include "MathUtil.h"
#include "Resource.h"
#include "SpriteSheet.h"
#include "Types.h"

#include <functional>
#include <vector>

namespace ToolKit
{

  static VariantCategory SurfaceCategory{"Surface", 90};

  class TK_API Surface : public Entity
  {
   public:
    Surface();
    Surface(TexturePtr texture, const Vec2& pivotOffset);
    Surface(TexturePtr texture, const SpriteEntry& entry);
    Surface(const String& textureFile, const Vec2& pivotOffset);
    Surface(const Vec2& size, const Vec2& offset = {0.5f, 0.5f});
    virtual ~Surface();

    EntityType GetType() const override;
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

    void CalculateAnchorOffsets(Vec3 canvas[4], Vec3 surface[4]);

    virtual void ResetCallbacks();
    //  To reflect the size & pivot changes,
    //  this function regenerates the geometry.
    virtual void UpdateGeometry(bool byTexture);

   protected:
    void ComponentConstructor();
    void ParameterConstructor();
    void ParameterEventConstructor();
    Entity* CopyTo(Entity* other) const override;
    Entity* InstantiateTo(Entity* other) const override;

   private:
    void CreateQuat();
    void CreateQuat(const SpriteEntry& val);
    void SetSizeFromTexture();

   public:
    TKDeclareParam(Vec2, Size);
    TKDeclareParam(Vec2, PivotOffset);
    TKDeclareParam(MaterialPtr, Material);

    // UI states.
    bool m_mouseOver    = false;
    bool m_mouseClicked = false;

    struct ANCHOR_PARAMS
    {
      float m_anchorRatios[4] = {0.f, 1.f, 0.f, 1.f};
      float m_offsets[4]      = {0.f};
    };

    ANCHOR_PARAMS m_anchorParams;

    // Event Callbacks.
    SurfaceEventCallback m_onMouseEnter = nullptr;
    SurfaceEventCallback m_onMouseExit  = nullptr;
    SurfaceEventCallback m_onMouseOver  = nullptr;
    SurfaceEventCallback m_onMouseClick = nullptr;
  };

  static VariantCategory ButtonCategory{"Button", 90};

  class TK_API Button : public Surface
  {
   public:
    Button();
    explicit Button(const Vec2& size);
    Button(const TexturePtr& buttonImage, const TexturePtr& hoverImage);
    virtual ~Button();
    EntityType GetType() const override;
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;
    void ResetCallbacks() override;

   protected:
    void ParameterConstructor();
    void ParameterEventConstructor();

   public:
    TKDeclareParam(MaterialPtr, ButtonMaterial);
    TKDeclareParam(MaterialPtr, HoverMaterial);

    // Local events.
    SurfaceEventCallback m_onMouseEnterLocal;
    SurfaceEventCallback m_onMouseExitLocal;
  };

} //  namespace ToolKit
