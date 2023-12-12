#pragma once

#ifdef _WIN32
  #define TK_PLATFORM PLATFORM::TKWindows
#elif __ANDROID__
  #define TK_PLATFORM PLATFORM::TKAndroid
#elif __EMSCRIPTEN__
  #define TK_PLATFORM PLATFORM::TKWeb
#endif
