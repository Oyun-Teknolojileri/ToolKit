#pragma once

#include "Types.h"
#include "Serialize.h"
#include <variant>

namespace ToolKit
{

  struct VariantCategory
  {
    String Name;
    int Priority;
  };

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
      String,
      Bool,
      ULongID
    };

    ParameterVariant() { SetVar(0); }
    virtual ~ParameterVariant() { }
    ParameterVariant(bool var) { SetVar(var); }
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
    ParameterVariant(const char* var) { SetVar(var); }
    ParameterVariant(const ULongID& var) { SetVar(var); }

    VariantType GetType() const { return m_type; }
    template<typename T> const T& GetVar() const { return std::get<T>(m_var); }
    void SetVar(bool var) { m_type = VariantType::Bool; m_var = var; }
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
    void SetVar(const char* var) { m_type = VariantType::String; m_var = String(var); }
    void SetVar(const ULongID& var) { m_type = VariantType::ULongID; m_var = var; }

    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

  public:
    String m_name = "Var";
    bool m_exposed = false; // Is this variant exposed to editor.
    bool m_editable = false; // Is the variable can be edited from editor.
    // Parameters will be shown under the category in the Inspector.
    // The higher the priority the earlier it will be displayed in the Inspector.
    VariantCategory m_category = { "Default", 0 };
  private:
    std::variant<bool, byte, ubyte, float, int, uint, Vec3, Vec4, Mat3, Mat4, String, ULongID> m_var;
    VariantType m_type;
  };

  class TK_API ParameterBlock : public Serializable
  {
  public:
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;


    // Collection utilities.
    ParameterVariant& operator [](int indx)
    {
      return m_variants[indx];
    }

    void Add(const ParameterVariant& var) { m_variants.push_back(var); }
    void GetCategories(VariantCategoryArray& categories, bool sortDesc);
    void GetByCategory(const String& category, ParameterVariantArray& variants);
    bool GetFirstByName(const String& name, ParameterVariant& var);
    void GetByName(const String& name, ParameterVariantArray& variants);

  public:
    ParameterVariantArray m_variants;
  };

}