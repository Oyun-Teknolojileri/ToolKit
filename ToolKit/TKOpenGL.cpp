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

  TKGL_FramebufferTexture2DMultisample tk_glFramebufferTexture2DMultisampleEXT = nullptr;

  TKGL_RenderbufferStorageMultisample tk_glRenderbufferStorageMultisampleEXT   = nullptr;

  void LoadGlFunctions(void* glGetProcAddres)
  {
#ifdef TK_WIN
    gladLoadGLES2((GLADloadfunc) glGetProcAddres);

  #ifdef GL_EXT_multisampled_render_to_texture
    if (GLAD_GL_EXT_multisampled_render_to_texture == 1)
    {
      tk_glFramebufferTexture2DMultisampleEXT = glad_glFramebufferTexture2DMultisampleEXT;
      tk_glRenderbufferStorageMultisampleEXT  = glad_glRenderbufferStorageMultisampleEXT;
    }
  #endif
#endif
  }

} // namespace ToolKit
