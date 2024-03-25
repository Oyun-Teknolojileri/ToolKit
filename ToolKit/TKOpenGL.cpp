/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#define GLAD_GLES2_IMPLEMENTATION
#include "TKOpenGL.h"

namespace ToolKit
{
  void LoadGlFunctions(void* glGetProcAddres)
  {
#ifdef _WIN32
    gladLoadGLES2((GLADloadfunc) glGetProcAddres);
#endif
  }

} // namespace ToolKit
