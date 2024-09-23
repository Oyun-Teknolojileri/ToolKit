/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

namespace ToolKit
{

  enum class PLATFORM
  {
    TKWindows,
    TKWeb,
    TKAndroid
  };

#ifdef _WIN32
  #define TK_PLATFORM PLATFORM::TKWindows
  #define TK_WIN
#elif __ANDROID__
  #define TK_PLATFORM PLATFORM::TKAndroid
  #define TK_ANDROID
#elif __EMSCRIPTEN__
  #define TK_PLATFORM PLATFORM::TKWeb
  #define TK_WEB
#endif

#ifdef TK_WIN // Windows.
  #define TK_STDCAL __stdcall
  #ifdef TK_DLL_EXPORT // Dynamic binding.
    #define TK_API __declspec(dllexport)
  #elif defined(TK_DLL_IMPORT)
    #define TK_API __declspec(dllimport)
  #else // Static binding.
    #define TK_API
  #endif
#elif defined(TK_ANDROID)
  #define TK_API __attribute__((visibility("default")))
  #define TK_STDCAL
#endif

#ifdef TK_WIN // Windows.
  #define TK_PLUGIN_API __declspec(dllexport)
#else // Other OS.
  #define TK_PLUGIN_API
#endif

  inline void HyperThreadPause()
  {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86) // x86/x64 architecture

  #if defined(_MSC_VER)                         // MSVC compiler
    _mm_pause();                                // MSVC intrinsic for x86/x64
  #elif defined(__clang__) || defined(__GNUC__) // Clang or GCC compiler
    __builtin_ia32_pause(); // GCC/Clang intrinsic for x86/x64
  #else
    #error Unsupported compiler for x86/x64 architecture
  #endif

#elif defined(__aarch64__) || defined(__arm__) // ARM architecture

  #if defined(_MSC_VER)                         // MSVC compiler for ARM (rare, but possible)
    __yield(); // MSVC intrinsic for ARM
  #elif defined(__clang__) || defined(__GNUC__) // Clang or GCC compiler
    asm volatile("yield"); // Inline assembly for ARM (Clang/GCC)
  #else
    #error Unsupported compiler for ARM architecture
  #endif

#else
  #error Unsupported architecture
#endif
  }

} // namespace ToolKit
