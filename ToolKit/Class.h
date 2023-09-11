/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "Types.h"

namespace ToolKit
{

  struct TK_API TKClass
  {
    TKClass* Super = nullptr;     //!< Compile time assigned base class for this class.
    String Name;                  //!< Compile time assigned unique class name.
    ULongID HashId = NULL_HANDLE; //!< Unique has id assigned to class when registered to TKObjectFactory.

    bool operator==(const TKClass& other) const
    {
      assert(HashId != NULL_HANDLE && "Class is not registered.");
      return (HashId == other.HashId);
    }

    bool operator==(const TKClass* other) const
    {
      assert(HashId != NULL_HANDLE && "Class is not registered.");
      return (HashId == other->HashId);
    }

    bool operator!=(const TKClass& other) const
    {
      assert(HashId != NULL_HANDLE && "Class is not registered.");
      return HashId != other.HashId;
    }

    bool operator!=(const TKClass* other) const
    {
      assert(HashId != NULL_HANDLE && "Class is not registered.");
      return HashId != other->HashId;
    }

    /**
     * Checks if the class is of the same type of base ( equal or derived from base ).
     * @param base - The target class to check equality for.
     * @return true in case of this class being equal base or derived from base.
     */
    bool IsSublcassOf(TKClass* base)
    {
      if (base == Super)
      {
        return true;
      }

      if (this == base)
      {
        return true;
      }

      // This specific condition is only valid for Object, marking this point as the end.
      if (this == Super)
      {
        return false; // No match found.
      }

      return Super->IsSublcassOf(base);
    }
  };

} // namespace ToolKit