#pragma once

#include "Types.h"
#include "Serialize.h"
#include <variant>
#include <unordered_map>

#define TKDeclareParam(Class, Name, Category, Priority, Exposed, Editable) \
  private: size_t Name##_Index; \
  private: inline void Name##_Define(ParameterVariant var) { \
    var.m_name = #Name; \
    var.m_category = { #Category, Priority }; \
    var.m_exposed = Exposed; \
    var.m_editable = Editable; \
    Name##_Index = m_localData.m_variants.size(); \
    m_localData.Add(var); \
  } \
  public: inline Class& Name() { \
   return m_localData[Name##_Index].GetVar<Class>(); \
  } \
  public: inline const Class& Name##C() const { \
   return m_localData[Name##_Index].GetCVar<Class>(); \
  }

namespace ToolKit
{

  struct VariantCategory
  {
    String Name;
    int Priority;
  };

  static VariantCategory CustomDataCategory = { "Custom Data", 0 };

  class TK_API ParameterVariantBase : public Serializable
  {
  public:
    ParameterVariantBase();

  public:
    ULongID m_id;

  private:
    static ULongID m_handle;
  };

  class TK_API ParameterVariant : public ParameterVariantBase
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

    ParameterVariant();
    ~ParameterVariant();
    ParameterVariant(bool var);
    ParameterVariant(byte var);
    ParameterVariant(ubyte var);
    ParameterVariant(float var);
    ParameterVariant(int var);
    ParameterVariant(uint var);
    ParameterVariant(const Vec3& var);
    ParameterVariant(const Vec4& var);
    ParameterVariant(const Mat3& var);
    ParameterVariant(const Mat4& var);
    ParameterVariant(const String& var);
    ParameterVariant(const char* var);
    ParameterVariant(const ULongID& var);

    VariantType GetType() const;
    
    template<typename T> 
    T& GetVar()
    {
      return std::get<T>(m_var);
    }

    template<typename T>
    const T& GetCVar() const
    {
      return std::get<T>(m_var);
    }

    template<typename T> 
    T* GetVarPtr()
    {
      return &std::get<T>(m_var);
    }

    ParameterVariant& operator= (bool var);
    ParameterVariant& operator= (byte var);
    ParameterVariant& operator= (ubyte var);
    ParameterVariant& operator= (float var);
    ParameterVariant& operator= (int var);
    ParameterVariant& operator= (uint var);
    ParameterVariant& operator= (const Vec3& var);
    ParameterVariant& operator= (const Vec4& var);
    ParameterVariant& operator= (const Mat3& var);
    ParameterVariant& operator= (const Mat4& var);
    ParameterVariant& operator= (const String& var);
    ParameterVariant& operator= (const char* var);
    ParameterVariant& operator= (const ULongID& var);

    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

  public:
    bool m_exposed; // Is this variant exposed to editor.
    bool m_editable; // Is the variable can be edited from editor.

    // Parameters will be shown under the category in the Inspector.
    // The higher the priority the earlier it will be displayed in the Inspector.
    VariantCategory m_category;
    String m_name;

  private:
    std::variant
    <
    bool,
    byte,
    ubyte,
    float,
    int,
    uint,
    Vec3,
    Vec4,
    Mat3,
    Mat4,
    String,
    ULongID
    > m_var;

    VariantType m_type = VariantType::Int;
  };

  class TK_API ParameterBlock : public Serializable
  {
  public:
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

    ParameterVariant& operator [](size_t index);
    const ParameterVariant& operator [](size_t index) const;

    void Add(const ParameterVariant& var);
    void Remove(ULongID id);
    void GetCategories(VariantCategoryArray& categories, bool sortDesc);
    void GetByCategory(const String& category, ParameterVariantRawPtrArray& variants);

  public:
    ParameterVariantArray m_variants;
  };

  static const ParameterVariant NullParam;
}