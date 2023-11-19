/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#ifdef _MSC_VER
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
#endif