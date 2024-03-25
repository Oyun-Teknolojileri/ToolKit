/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#ifdef __ANDROID__
  #include <GLES3/gl32.h>
#elif defined(__EMSCRIPTEN__)
  #include <GL/glew.h>
#else
  #include <glad/gles2.h>
#endif

namespace ToolKit
{
  void LoadGlFunctions(void* glGetProcAddres);
}