#pragma once

#include "Types.h"
#include "Drawable.h"
#include "RenderState.h"
#include "ParameterBlock.h"

namespace ToolKit
{
  class TK_API Billboard : public Entity
  {
   public:
    struct Settings
    {
      bool lookAtCamera = true;

      /**
      * If grater then 0, place the billboard always at approximately 
      * (Difference due to RadialToPlanarDistance conversion) 
      * given distance to camera.
      */
      float distanceToCamera = 0.0f;

      /**
      * If greater then 0, 
      * Fov changes due to window height changes doesn't shrink the object.
      */
      float heightInScreenSpace = 0.0f;
    };

   public:
    explicit Billboard(const Settings& settings);

    virtual void LookAt(class Camera* cam, float scale);
    EntityType GetType() const override;

   protected:
    Entity* CopyTo(Entity* copyTo) const override;
    Entity* InstantiateTo(Entity* copyTo) const override;

   public:
    Settings m_settings;
    Vec3 m_worldLocation;
    Entity* m_entity = nullptr;
  };

  class TK_API Cube final : public Entity
  {
   public:
    Cube(bool genDef = true);
    Cube(const Vec3& scale);

    EntityType GetType() const override;
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

    void Generate();

   public:
    TKDeclareParam(Vec3, Scale);

   protected:
    Entity* CopyTo(Entity* copyTo) const override;
    Entity* InstantiateTo(Entity* copyTo) const override;
    void ParameterConstructor();

   private:
    bool m_generated = false;
  };

  class TK_API Quad final : public Entity
  {
   public:
    Quad(bool genDef = true);

    EntityType GetType() const override;

   protected:
    Entity* CopyTo(Entity* copyTo) const override;
    Entity* InstantiateTo(Entity* copyTo) const override;
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

   private:
    void Generate();
  };

  class TK_API Sphere final : public Entity
  {
   public:
    Sphere(bool genDef = true);
    Sphere(float radius);

    EntityType GetType() const override;
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

   protected:
    Entity* CopyTo(Entity* copyTo) const override;
    Entity* InstantiateTo(Entity* copyTo) const override;
    void ParameterConstructor(float radius);

   private:
    void Generate();

   public:
    TKDeclareParam(float, Radius);
  };

  class TK_API Cone final : public Entity
  {
   public:
    Cone(bool genDef = true);
    Cone(float height, float radius, int segBase, int segHeight);

    EntityType GetType() const override;
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

   protected:
    Entity* CopyTo(Entity* copyTo) const override;
    Entity* InstantiateTo(Entity* copyTo) const override;
    void ParameterConstructor();

   private:
    void Generate();

   public:
    TKDeclareParam(float, Height);
    TKDeclareParam(float, Radius);
    TKDeclareParam(int, SegBase);
    TKDeclareParam(int, SegHeight);
  };

  class TK_API Arrow2d final : public Entity
  {
   public:
    Arrow2d(bool genDef = true);
    Arrow2d(AxisLabel label);  // X - Y - Z.
    EntityType GetType() const override;

   protected:
    Entity* CopyTo(Entity* copyTo) const override;
    Entity* InstantiateTo(Entity* copyTo) const override;

   private:
    void Generate();

   private:
    AxisLabel m_label;
  };

  class TK_API LineBatch final : public Entity
  {
   public:
    LineBatch();
    LineBatch
    (
      const Vec3Array& linePnts,
      const Vec3& color,
      DrawType t,
      float lineWidth = 1.0f
    );

    EntityType GetType() const override;
    void Generate
    (
      const Vec3Array& linePnts,
      const Vec3& color,
      DrawType t,
      float lineWidth = 1.0f
    );

   protected:
    Entity* CopyTo(Entity* copyTo) const override;
  };

}  // namespace ToolKit

