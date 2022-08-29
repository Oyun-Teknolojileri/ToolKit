#pragma once

#include <memory>
#include <vector>

#include "Gizmo.h"
#include "Light.h"
#include "MathUtil.h"
#include "Primative.h"
#include "Renderer.h"
#include "Viewport.h"

namespace ToolKit
{
  namespace Editor
  {
      using AnchorPtr = std::shared_ptr<class Anchor>;
      using AnchorHandlePtr = std::shared_ptr<class AnchorHandle>;

      class AnchorHandle
      {
       public:
          enum class SolidType
          {
              Quad,
              Circle
          };

          struct Params
          {
              // Worldspace data.
              Vec3 worldLoc;
              Vec3 grabPnt;
              // Billboard values.
              Vec3 scale;
              Vec3 translate;
              float angle = 0.f;
              // Geometry.
              DirectionLabel direction;
              Vec3 color;
              SolidType type;
          };

       public:
          AnchorHandle();

          virtual void Generate(const Params& params);
          virtual bool HitTest(const Ray& ray, float& t) const;
          Mat4 GetTransform() const;

       public:
          Params m_params;
          MeshPtr m_mesh;
      };

      class Anchor : public EditorBillboardBase
      {
       public:
          explicit Anchor(const Billboard::Settings& set);
          BillboardType GetBillboardType() const override;

          virtual DirectionLabel HitTest(const Ray& ray) const;
          virtual void Update(float deltaTime);
          bool IsGrabbed(DirectionLabel direction) const;
          void Grab(DirectionLabel direction);
          DirectionLabel GetGrabbedDirection() const;

       protected:
          virtual AnchorHandle::Params GetParam() const;

       public:
          Vec3 m_grabPoint;
          DirectionLabel m_lastHovered;
          std::vector<AnchorHandlePtr> m_handles;
          float m_anchorRatios[4];

       protected:
          DirectionLabel m_grabbedDirection;
      };
  }  // namespace Editor
}  // namespace ToolKit
