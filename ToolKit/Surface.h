/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Entity.h"
#include "SpriteSheet.h"
#include "Types.h"

namespace ToolKit
{

  static VariantCategory SurfaceCategory {"Surface", 90};

  class TK_API Surface : public Entity
  {
   public:
    TKDeclareClass(Surface, Entity);

    Surface();
    virtual ~Surface();

    void NativeConstruct() override;

    void Update(TexturePtr texture, const Vec2& pivotOffset);
    void Update(TexturePtr texture, const SpriteEntry& entry);
    void Update(const String& textureFile, const Vec2& pivotOffset);
    void Update(const Vec2& size, const Vec2& offset = Vec2(0.5f));

    void CalculateAnchorOffsets(Vec3 canvas[4], Vec3 surface[4]);
    virtual void ResetCallbacks();

    /**
     * To reflect the size & pivot changes,this function regenerates the geometry.
     * @param byTexture - if true send, the geometry is updated based on material's diffuse texture.
     */
    virtual void UpdateGeometry(bool byTexture);

   protected:
    virtual void ComponentConstructor();
    void ParameterConstructor() override;
    void ParameterEventConstructor() override;
    Entity* CopyTo(Entity* other) const override;

    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;
    XmlNode* DeSerializeImpV045(const SerializationFileInfo& info, XmlNode* parent);

    virtual void SetDefaultMaterialIfMaterialIsNotOverriden();

   private:
    void CreateQuat();
    void CreateQuat(const SpriteEntry& val);
    void SetSizeFromTexture();

   public:
    TKDeclareParam(Vec2, Size);
    TKDeclareParam(Vec2, PivotOffset);
    TKDeclareParam(MaterialPtr, Material);
    TKDeclareParam(VariantCallback, UpdateSizeFromTexture);

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

  static VariantCategory ButtonCategory {"Button", 90};

  class TK_API Button : public Surface
  {
   public:
    TKDeclareClass(Button, Surface);

    Button();
    virtual ~Button();
    void NativeConstruct() override;
    void SetBtnImage(const TexturePtr buttonImage, const TexturePtr hoverImage);
    void ResetCallbacks() override;

   protected:
    void ParameterConstructor() override;
    void ParameterEventConstructor() override;
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;
    XmlNode* DeSerializeImpV045(const SerializationFileInfo& info, XmlNode* parent);

   public:
    TKDeclareParam(MaterialPtr, ButtonMaterial);
    TKDeclareParam(MaterialPtr, HoverMaterial);

    // Local events.
    SurfaceEventCallback m_onMouseEnterLocal;
    SurfaceEventCallback m_onMouseExitLocal;
  };

  typedef std::shared_ptr<Button> ButtonPtr;

} //  namespace ToolKit
