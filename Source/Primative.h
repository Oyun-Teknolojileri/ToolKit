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
    class Params : public ParameterBlock
    {
    public:
      Params()
      {
        m_variants.push_back(Vec3(1.0f));
      }
      Params(const Vec3& scale)
      {
        m_variants.push_back(scale);
      }
    } m_params;

  public:
    Cube(bool genDef = true);
    Cube(const Params& params);

    virtual EntityType GetType() const override;
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

  protected:
    virtual Entity* CopyTo(Entity* copyTo) const override;
    virtual Entity* InstantiateTo(Entity* copyTo) const override;

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
    class Params : public ParameterBlock
    {
    public:
      Params()
      {
        m_variants.push_back(1.0f);
      }
      Params(float rad)
      {
        m_variants.push_back(rad);
      }
    } m_params;

  public:
    Sphere(bool genDef = true);
    Sphere(const Params& params);

    virtual EntityType GetType() const override;
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

  protected:
    virtual Entity* CopyTo(Entity* copyTo) const override;
    virtual Entity* InstantiateTo(Entity* copyTo) const override;

  private:
    void Generate();
  };

  class TK_API Cone final : public Drawable
  {
  public:
    class Params : public ParameterBlock
    {
    public:
      Params()
      {
        m_variants.push_back(1.0f);
        m_variants.push_back(1.0f);
        m_variants.push_back(30);
        m_variants.push_back(20);
      }
      Params(float height, float rad, int nSegBase, int nSegHeight)
      {
        m_variants.push_back(height);
        m_variants.push_back(rad);
        m_variants.push_back(nSegBase);
        m_variants.push_back(nSegHeight);
      }
    } m_params;

  public:
    Cone(bool genDef = true);
    Cone(const Params& params);

    virtual EntityType GetType() const override;
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

  protected:
    virtual Entity* CopyTo(Entity* copyTo) const override;
    virtual Entity* InstantiateTo(Entity* copyTo) const override;

  private:
    void Generate();
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
