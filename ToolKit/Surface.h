/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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

  static VariantCategory SurfaceCategory {"Surface", 90};

  class TK_API Surface : public Entity
  {
   public:
    TKDeclareClass(Surface, Entity);

    Surface();
    Surface(TexturePtr texture, const Vec2& pivotOffset);
    Surface(TexturePtr texture, const SpriteEntry& entry);
    Surface(const String& textureFile, const Vec2& pivotOffset);
    Surface(const Vec2& size, const Vec2& offset = {0.5f, 0.5f});
    virtual ~Surface();

    EntityType GetType() const override;
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

    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerializeImp(XmlDocument* doc, XmlNode* parent) override;

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
    explicit Button(const Vec2& size);
    Button(const TexturePtr& buttonImage, const TexturePtr& hoverImage);
    virtual ~Button();
    EntityType GetType() const override;
    void DeSerializeImp(XmlDocument* doc, XmlNode* parent) override;
    void ResetCallbacks() override;

   protected:
    void ParameterConstructor();
    void ParameterEventConstructor();
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;

   public:
    TKDeclareParam(MaterialPtr, ButtonMaterial);
    TKDeclareParam(MaterialPtr, HoverMaterial);

    // Local events.
    SurfaceEventCallback m_onMouseEnterLocal;
    SurfaceEventCallback m_onMouseExitLocal;
  };

} //  namespace ToolKit
