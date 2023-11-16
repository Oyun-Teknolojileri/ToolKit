/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Types.h"

namespace ToolKit
{

  struct TK_API ClassMeta
  {
    ClassMeta* Super = nullptr;   //!< Compile time assigned base class for this class.
    String Name;                  //!< Compile time assigned unique class name.
    ULongID HashId = NULL_HANDLE; //!< Compile time assigned hash code.

    /**
     * Holds meta data, information such as if the class will be visible to editor, where it will store takes place
     * here.
     */
    std::unordered_map<StringView, StringView> MetaKeys;

    bool operator==(const ClassMeta& other) const
    {
      assert(HashId != NULL_HANDLE && "Class is not registered.");
      return (HashId == other.HashId);
    }

    bool operator!=(const ClassMeta& other) const
    {
      assert(HashId != NULL_HANDLE && "Class is not registered.");
      return HashId != other.HashId;
    }

    /**
     * Checks if the class is of the same type of base ( equal or derived from base ).
     * @param base - The target class to check equality for.
     * @return true in case of this class being equal base or derived from base.
     */
    bool IsSublcassOf(ClassMeta* base)
    {
      if (*base == *Super)
      {
        return true;
      }

      if (*this == *base)
      {
        return true;
      }

      // This specific condition is only valid for Object, marking this point as the end.
      if (*this == *Super)
      {
        return false; // No match found.
      }

      return Super->IsSublcassOf(base);
    }
  };

} // namespace ToolKit
