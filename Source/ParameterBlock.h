#pragma once

#include "Types.h"
#include "Serialize.h"
#include <variant>

namespace ToolKit
{

  class TK_API ParameterVariant : public Serializable
  {
  public:
    enum class VariantType
    {
      byte,
      ubyte,
      Float,
      Int,
      UInt,
      Vec3,
      Vec4,
      Mat3,
      Mat4,
      String
    };

    ParameterVariant() { SetVar(0); }
    virtual ~ParameterVariant() { }
    ParameterVariant(byte var) { SetVar(var); }
    ParameterVariant(ubyte var) { SetVar(var); }
    ParameterVariant(float var) { SetVar(var); }
    ParameterVariant(int var) { SetVar(var); }
    ParameterVariant(uint var) { SetVar(var); }
    ParameterVariant(const Vec3& var) { SetVar(var); }
    ParameterVariant(const Vec4& var) { SetVar(var); }
    ParameterVariant(const Mat3& var) { SetVar(var); }
    ParameterVariant(const Mat4& var) { SetVar(var); }
    ParameterVariant(const String& var) { SetVar(var); }

    VariantType GetType() const { return m_type; }
    template<typename T> const T& GetVar() const { return std::get<T>(m_var); }
    void SetVar(byte var) { m_type = VariantType::byte; m_var = var; }
    void SetVar(ubyte var) { m_type = VariantType::ubyte; m_var = var; }
    void SetVar(float var) { m_type = VariantType::Float; m_var = var; }
    void SetVar(int var) { m_type = VariantType::Int; m_var = var; }
    void SetVar(uint var) { m_type = VariantType::UInt; m_var = var; }
    void SetVar(const Vec3& var) { m_type = VariantType::Vec3; m_var = var; }
    void SetVar(const Vec4& var) { m_type = VariantType::Vec4; m_var = var; }
    void SetVar(const Mat3& var) { m_type = VariantType::Mat3; m_var = var; }
    void SetVar(const Mat4& var) { m_type = VariantType::Mat4; m_var = var; }
    void SetVar(const String& var) { m_type = VariantType::String; m_var = var; }

    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

  private:
    std::variant<byte, ubyte, float, int, uint, Vec3, Vec4, Mat3, Mat4, String> m_var;
    VariantType m_type;
  };

  class TK_API ParameterBlock : public Serializable
  {
  public:
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

    ParameterVariant& operator [](int indx)
    {
      return m_variants[indx];
    }

  public:
    std::vector<ParameterVariant> m_variants;
  };

}