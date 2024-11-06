/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Surface.h"

namespace ToolKit
{
  static VariantCategory DpadCategory {"Dpad", 90};

  class TK_API Dpad : public Surface
  {
   public:
    TKDeclareClass(Dpad, Surface);

    Dpad();
    virtual ~Dpad();

    void ParameterConstructor() override;

    /** @param mouseXY: mouse XY position in UI coordinates. */
    void UpdateDpad(const Vec2& mouseXY);
    void Start();
    void Stop();

    float GetDeltaX();
    float GetDeltaY();
    float GetRadius();

   protected:
    void ComponentConstructor() override;
    void DeserializeComponents(const SerializationFileInfo& info, XmlNode* entityNode) override;

   public:
    TKDeclareParam(float, DpadRadius);

   private:
    bool m_active = false;
    Vec2 m_deltaXY;

    float m_activeDpadRadius = 100.0f;
    Vec3 m_lastScale         = {-1.0f, -1.0f, -1.0f};
  };

} // namespace ToolKit
