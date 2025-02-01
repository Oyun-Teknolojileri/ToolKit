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
#else
  #define TK_API __attribute__((visibility("default")))
  #define TK_STDCAL
#endif

#ifdef TK_WIN // Windows.
  #define TK_PLUGIN_API __declspec(dllexport)
#else // Other OS.
  #define TK_PLUGIN_API
#endif

#ifdef _MSC_VER
  #if defined(__aarch64__) || defined(__arm__)
    #define HyperThreadPause() __yield()
  #else
    #define HyperThreadPause() _mm_pause()
  #endif
#else
  #if defined(__aarch64__) || defined(__arm__)
    #define HyperThreadPause() asm volatile("yield")
  #else
    #define HyperThreadPause() std::this_thread::yield()
  #endif
#endif

} // namespace ToolKit
