#pragma once

/**
* @file ParameterBlock.h Header for ParameterVariant, ParameterBlock
* and related structures.
*/

#include <variant>
#include <unordered_map>

#include "Types.h"
#include "Serialize.h"

/**
* @def TKDeclareParam(Class, Name) Auto generated code for declaring accessing
* local data for container Class.
* Any class which needs managed ParameterBlocks must declare
* ParameterBlock m_localData object. For each ParameterVariant, this macro
* can be utilized to generate access methods for the corresponding
* ParameterVariant.
* @param Class One of the supported types by ParameterVariant.
* @param Name Name of the ParameterVariant.
*/
#define TKDeclareParam(Class, Name) \
  private: size_t Name##_Index; \
  private: inline void Name##_Define(Class val, \
    const String& category, \
    int priority, \
    bool exposed, \
    bool editable) { \
    ParameterVariant var(val); \
    var.m_name = #Name; \
    var.m_category = { category, priority }; \
    var.m_exposed = exposed; \
    var.m_editable = editable; \
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

  /**
  * The category to group / access / sort and display the ParameterVariant.
  */
  struct VariantCategory
  {
    String Name;  //!< Name of the category.
    /**
    * Priority of the category. Sorted and processed by this number within
    * every aspect of the framework. Such as Editor's Property Inspector.
    */
    int Priority;
  };

  /**
  * Predefined category for user defined data.
  */
  static VariantCategory CustomDataCategory = { "Custom Data", 0 };

  /**
  * Base class for ParameterVariant. This class is responsible for providing
  * initial data to every ParameterVariant such as providing a unique id.
  */
  class TK_API ParameterVariantBase : public Serializable
  {
   public:
    /**
    * Base Constructor, creates a unique id for the ParameterVariant.
    */
    ParameterVariantBase();

    /**
    * Empty destructor.
    */
    ~ParameterVariantBase();

   public:
    ULongID m_id;  //!< Unique id for the current runtime.

   private:
    static ULongID m_handle;
  };

  /**
  * A multi type object that encapsulates std::variant. The purpose of this
  * class is to provide automated functionality such as serialization, auto
  * cloning etc... This type of parameters can be recognized and properly
  * handled trough out the ToolKit framework. Such as, Editor's
  * Property Inspector. Any exposed parameter variant will be displayed under
  * the right category automatically.
  */
  class TK_API ParameterVariant : public ParameterVariantBase
  {
   public:
    /**
    * Enums for supported ParameterVariant types. These types are used for
    * type checking and serialization.
    */
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

    /**
    * Default constructor that creates an int ParameterVariant with value of 0.
    */
    ParameterVariant();

    /**
    * Empty destructor.
    */
    ~ParameterVariant();

    /**
    * Constructs bool type variant.
    */
    explicit ParameterVariant(bool var);

    /**
    * Constructs byte type variant.
    */
    explicit ParameterVariant(byte var);

    /**
    * Constructs ubyte type variant.
    */
    explicit ParameterVariant(ubyte var);

    /**
    * Constructs float type variant.
    */
    explicit ParameterVariant(float var);

    /**
    * Constructs Vec3 type variant.
    */
    explicit ParameterVariant(int var);

    /**
    * Constructs uint type variant.
    */
    explicit ParameterVariant(uint var);

    /**
    * Constructs Vec3 type variant.
    */
    explicit ParameterVariant(const Vec3& var);

    /**
    * Constructs Vec4 type variant.
    */
    explicit ParameterVariant(const Vec4& var);

    /**
    * Constructs Mat3 type variant.
    */
    explicit ParameterVariant(const Mat3& var);

    /**
    * Constructs Mat4 type variant.
    */
    explicit ParameterVariant(const Mat4& var);

    /**
    * Constructs String type variant.
    */
    explicit ParameterVariant(const String& var);

    /**
    * Constructs const char* type variant.
    */
    explicit ParameterVariant(const char* var);

    /**
    * Constructs ULongID type variant.
    */
    explicit ParameterVariant(ULongID& var);

    /**
    * Used to retrieve VariantType of the variant.
    * @return VariantType That corresponds to current type of the variant.
    */
    VariantType GetType() const;

    /**
    * Used to access the underlying value of the variant.
    * @return A reference to the value set during the initialization of the
    * variant.
    */
    template<typename T>
    T& GetVar()
    {
      return std::get<T>(m_var);
    }

    /**
    * Returns the const reference to the value of the variant.
    * @see T& GetVar()
    * @return const reference to the value of variant.
    */
    template<typename T>
    const T& GetCVar() const
    {
      return std::get<T>(m_var);
    }

    /**
    * Returns the pointer to the value of the variant.
    * @return A pointer to the value of the variant.
    */
    template<typename T>
    T* GetVarPtr()
    {
      return &std::get<T>(m_var);
    }

    /**
    * Assign a bool to the value of the variant.
    */
    ParameterVariant& operator= (bool var);

    /**
    * Assign a byte to the value of the variant.
    */
    ParameterVariant& operator= (byte var);

    /**
    * Assign a ubyte to the value of the variant.
    */
    ParameterVariant& operator= (ubyte var);

    /**
    * Assign a float to the value of the variant.
    */
    ParameterVariant& operator= (float var);

    /**
    * Assign a int to the value of the variant.
    */
    ParameterVariant& operator= (int var);

    /**
    * Assign a uint to the value of the variant.
    */
    ParameterVariant& operator= (uint var);

    /**
    * Assign a Vec3 to the value of the variant.
    */
    ParameterVariant& operator= (const Vec3& var);

    /**
    * Assign a Vec4 to the value of the variant.
    */
    ParameterVariant& operator= (const Vec4& var);

    /**
    * Assign a Mat3 to the value of the variant.
    */
    ParameterVariant& operator= (const Mat3& var);

    /**
    * Assign a Mat4 to the value of the variant.
    */
    ParameterVariant& operator= (const Mat4& var);

    /**
    * Assign a String to the value of the variant.
    */
    ParameterVariant& operator= (const String& var);

    /**
    * Assign a const char* to the value of the variant.
    */
    ParameterVariant& operator= (const char* var);

    /**
    * Assign a ULongID to the value of the variant.
    */
    ParameterVariant& operator= (ULongID var);

    /**
    * Serializes the variant to the xml document.
    * @param doc The xml document object to serialize to.
    * @param parent The parent xml node to serialize to.
    */
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;

    /**
    * De serializes the variant from the xml document.
    * @param doc The xml document object to read from.
    * @param parent The parent xml node to read from.
    */
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

   public:
    /**
    * States if this variant exposed to framework / editor.
    */
    bool m_exposed;

    /**
    * States if this variable can be edited from framework / editor.
    * Does not provide explicit protection. The system that uses the variant
    * may chose to obey.
    */
    bool m_editable;

    /**
    * Framework accumulates and treats similarly to every variant that shares
    * the same category. Such as editor, it displays every exposed variant that
    * shares the same category under the same drop-down area.
    */
    VariantCategory m_category;
    String m_name;  //<! Name of the variant.

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
      > m_var;  //!< The variant that hold the actual data.

    VariantType m_type = VariantType::Int;  //!< Type of the variant.
  };

  /**
  * A class that can be used to group ParameterVariant objects.
  * Act like a manager class for a group of ParameterVariant objects.
  */
  class TK_API ParameterBlock : public Serializable
  {
   public:
    /**
    * Serializes the ParameterBlock to the xml document.
    * @param doc The xml document object to serialize to.
    * @param parent The parent xml node to serialize to.
    */
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;

    /**
    * De serializes the ParameterBlcok from the xml document.
    * @param doc The xml document object to read from.
    * @param parent The parent xml node to read from.
    */
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

    /**
    * Used to access ParameterVariant's by index.
    * @return Reference to indexed ParameterVariant.
    */
    ParameterVariant& operator [](size_t index);

    /**
    * Used to access ParameterVariant's by index.
    * @return const Reference to indexed ParameterVariant.
    */
    const ParameterVariant& operator [](size_t index) const;

    /**
    * Adds a variant to the ParameterBlock. No uniqueness guaranteed.
    * @param var The ParameterVariant to insert.
    */
    void Add(const ParameterVariant& var);

    /**
    * Remove's the first variant with the id.
    * @param id The id of the variant to remove.
    */
    void Remove(ULongID id);

    /**
    * Collects all unique categories and sorts the categories in
    * descending order by request.
    * @param categories An array to return unique categories.
    * @param sortDesc Sorts the categories by priority in descending order.
    */
    void GetCategories(VariantCategoryArray& categories, bool sortDesc);

    /**
    * Collects every variant by the given category.
    * @param category The category to search the variants in.
    * @param variants The resulting variant array which holds references to the
    * variants that falls under the requested category.
    */
    void GetByCategory
    (
      const String& category,
      ParameterVariantRawPtrArray& variants
    );

   public:
    /**
    * Container vector for ParameterVariants.
    */
    ParameterVariantArray m_variants;
  };

}  // namespace ToolKit
