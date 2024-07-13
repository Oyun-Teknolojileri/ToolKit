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

  int TK_GL_EXT_texture_filter_anisotropic                                     = 0;

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

  #ifdef GL_EXT_texture_filter_anisotropic
    TK_GL_EXT_texture_filter_anisotropic = GLAD_GL_EXT_texture_filter_anisotropic;
  #endif

#endif

#ifdef TK_ANDROID

      typedef void* (*GL_PROC_ADDR)(const char*);
      GL_PROC_ADDR glLoader = (GL_PROC_ADDR)glGetProcAddres;

      tk_glRenderbufferStorageMultisampleEXT = (TKGL_RenderbufferStorageMultisample) glLoader("glRenderbufferStorageMultisampleEXT");
      tk_glFramebufferTexture2DMultisampleEXT = (TKGL_FramebufferTexture2DMultisample) glLoader("glFramebufferTexture2DMultisampleEXT");

#endif
  }

} // namespace ToolKit
