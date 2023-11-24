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

    /*
     * @param mouseXY: mouse XY position in UI coordinates
     */
    void UpdateDpad(const Vec2& mouseXY);

    inline float GetDeltaX() { return m_deltaXY.x; }
    inline float GetDeltaY() { return m_deltaXY.y; }
    inline float GetRadius() { return m_activeDpadRadius; }

   public:
    TKDeclareParam(float, DpadRadius);

   private:
    Vec2 m_deltaXY;

    float m_activeDpadRadius = 100.0f;
    Vec3 m_lastScale         = {-1.0f, -1.0f, -1.0f};
  };
} // namespace ToolKit
