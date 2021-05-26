#pragma once

#ifndef SKIP_TK_DEBUG_NEW
  #ifdef _DEBUG
    #define _CRTDBG_MAP_ALLOC
    #include <cstdlib>
    #include <crtdbg.h>
    #define TK_DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
  #endif

  #if defined TK_DEBUG_NEW
    #define new TK_DEBUG_NEW
  #endif
#endif