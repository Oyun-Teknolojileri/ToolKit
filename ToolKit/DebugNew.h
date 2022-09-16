#pragma once

#ifndef SKIP_TK_DEBUG_NEW
  #ifdef TK_DEBUG
    #define _CRTDBG_MAP_ALLOC
    #include <crtdbg.h>

    #include <cstdlib>
    #define TK_DEBUG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
  #endif

  #if defined TK_DEBUG_NEW
    #define new TK_DEBUG_NEW
  #endif
#endif
