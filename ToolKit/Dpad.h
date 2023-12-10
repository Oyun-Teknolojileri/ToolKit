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

    inline void Start() { m_active = true; }

    inline void Stop()
    {
      m_deltaXY = Vec2(0.0f);
      m_active  = false;
    }

    inline float GetDeltaX() { return m_deltaXY.x; }

    inline float GetDeltaY() { return m_deltaXY.y; }

    inline float GetRadius() { return m_activeDpadRadius; }

   protected:
    void ComponentConstructor() override;
    void SetDefaultMaterialIfMaterialIsNotOverriden() override;

   public:
    TKDeclareParam(float, DpadRadius);

   private:
    bool m_active = false;
    Vec2 m_deltaXY;

    float m_activeDpadRadius = 100.0f;
    Vec3 m_lastScale         = {-1.0f, -1.0f, -1.0f};
  };
} // namespace ToolKit
