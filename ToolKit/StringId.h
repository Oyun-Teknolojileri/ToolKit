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

#include <unordered_map>

namespace ToolKit
{

  /**
   * This is a class that generates unique ids for Strings during the current runtime.
   * Algorithm is not deterministic. Meaning that each run may give different ids to the same string.
   * But within the current runtime, it guarantees to return same id for the given string and vise versa.
   */
  class StringId
  {
    friend class StringIdManager;

   private:
    StringId(const String& str, ULongID id) : m_string(str), m_id(id) {}

   public:
    StringId(const StringId& other)
    {
      m_string = other.m_string;
      m_id     = other.m_id;
    }

    bool operator==(const StringId& other) const { return (m_id == other.m_id); }

    bool operator!=(const StringId& other) const { return !(*this == other); }

    StringId& operator=(const StringId& other)
    {
      if (this != &other)
      {
        m_string = other.m_string;
        m_id     = other.m_id;
      }
      return *this;
    }

    const String& Str() { return m_string; }

    ULongID Id() { return m_id; }

   private:
    String m_string;
    ULongID m_id = 0;
  };

  /**
   * Manager responsible for creating StringIds from the given string. Manager guarantees to provide collision free
   * unique ids for the current runtime.
   */
  class StringIdManager
  {
   public:
    StringId CreateStringId(const String& str)
    {
      // Retrieve existing pair.
      if (Exist(str))
      {
        return StringId(str, m_stringToId[str]);
      }

      std::hash<String> hashFunc;
      ULongID hashValue = hashFunc(str);
      ULongID uniqueId  = hashValue ^ ++m_counter;

      // Prevent collision.
      while (Exist(uniqueId))
      {
        uniqueId = hashValue ^ ++m_counter;
      }

      m_stringToId[str]      = uniqueId;
      m_idToString[uniqueId] = str;

      return StringId(str, uniqueId);
    }

    String IdToString(ULongID id)
    {
      auto it = m_idToString.find(id);
      if (it != m_idToString.end())
      {
        return it->second;
      }

      return String();
    }

    ULongID StringToId(const String& str)
    {
      auto it = m_stringToId.find(str);
      if (it != m_stringToId.end())
      {
        return it->second;
      }

      return NULL_HANDLE;
    }

    bool Exist(ULongID id)
    {
      auto it = m_idToString.find(id);
      return it != m_idToString.end();
    }

    bool Exist(const String& str)
    {
      auto it = m_stringToId.find(str);
      return it != m_stringToId.end();
    }

   private:
    std::unordered_map<String, ULongID> m_stringToId;
    std::unordered_map<ULongID, String> m_idToString;
    ULongID m_counter = 0;
  };

} // namespace ToolKit