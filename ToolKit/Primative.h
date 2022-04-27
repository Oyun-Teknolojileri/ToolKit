#pragma once

#include "ToolKit.h"
#include "ParameterBlock.h"

namespace ToolKit
{

  class TK_API Billboard : public Drawable
  {
  public:
    struct Settings
    {
      bool lookAtCamera = true;
      // If grater then 0, place the billboard always at approximately (Difference due to RadialToPlanarDistance conversion) given distance to camera.
      float distanceToCamera = 0.0f;
      // If greater then 0, Fov changes due to window height changes doesn't shrink the object.
      float heightInScreenSpace = 0.0f;
    };

  public:
    Billboard(const Settings& settings);

    virtual void LookAt(class Camera* cam, float scale);
    virtual EntityType GetType() const override;

  protected:
    virtual Entity* CopyTo(Entity* copyTo) const override;
    virtual Entity* InstantiateTo(Entity* copyTo) const override;

  public:
    Settings m_settings;
    Vec3 m_worldLocation;
  };

  class TK_API Cube final : public Drawable
  {
  public:
    Cube(bool genDef = true);
    Cube(const Vec3& scale);

    virtual EntityType GetType() const override;
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

  public:
    TKDeclareParam(Vec3, Scale, Geometry, 90, true, true);

  protected:
    virtual Entity* CopyTo(Entity* copyTo) const override;
    virtual Entity* InstantiateTo(Entity* copyTo) const override;
    void ParameterConstructor();

  private:
    void Generate();
  };

  class TK_API Quad final : public Drawable
  {
  public:
    Quad(bool genDef = true);

    virtual EntityType GetType() const override;

  protected:
    virtual Entity* CopyTo(Entity* copyTo) const override;
    virtual Entity* InstantiateTo(Entity* copyTo) const override;
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

  private:
    void Generate();
  };

  class TK_API Sphere final : public Drawable
  {
  public:
    Sphere(bool genDef = true);
    Sphere(float radius);

    virtual EntityType GetType() const override;
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

  protected:
    virtual Entity* CopyTo(Entity* copyTo) const override;
    virtual Entity* InstantiateTo(Entity* copyTo) const override;
    void ParameterConstructor();

  private:
    void Generate();

  public:
    TKDeclareParam(float, Radius, Geometry, 90, true, true);
  };

  class TK_API Cone final : public Drawable
  {
  public:
    Cone(bool genDef = true);
    Cone(float height, float radius, int segBase, int segHeight);

    virtual EntityType GetType() const override;
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

  protected:
    virtual Entity* CopyTo(Entity* copyTo) const override;
    virtual Entity* InstantiateTo(Entity* copyTo) const override;
    void ParameterConstructor();

  private:
    void Generate();

  public:
    TKDeclareParam(float, Height, Geometry, 90, true, true);
    TKDeclareParam(float, Radius, Geometry, 90, true, true);
    TKDeclareParam(int, SegBase, Geometry, 90, true, true);
    TKDeclareParam(int, SegHeight, Geometry, 90, true, true);
  };

  class TK_API Arrow2d final : public Drawable
  {
  public:
    Arrow2d(bool genDef = true);
    Arrow2d(AxisLabel label); // X - Y - Z.
    virtual EntityType GetType() const override;

  protected:
    virtual Entity* CopyTo(Entity* copyTo) const override;
    virtual Entity* InstantiateTo(Entity* copyTo) const override;

  private:
    void Generate();

  private:
    AxisLabel m_label;
  };

  class TK_API LineBatch final : public Drawable
  {
  public:
    LineBatch();
    LineBatch(const Vec3Array& linePnts, const Vec3& color, DrawType t, float lineWidth = 1.0f);

    virtual EntityType GetType() const override;
    void Generate(const Vec3Array& linePnts, const Vec3& color, DrawType t, float lineWidth = 1.0f);

  protected:
    virtual Entity* CopyTo(Entity* copyTo) const override;
  };

}
