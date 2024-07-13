/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Types.h"

#ifdef TK_ANDROID
  #include <GLES3/gl32.h>
#elif defined(TK_WEB)
  #include <GL/glew.h>
#else
  #include <glad/gles2.h>
#endif

namespace ToolKit
{

  // GL Extensions used by ToolKit.

  // glFramebufferTexture2DMultisampleEXT
  //////////////////////////////////////////////////////////////////////////

  typedef void(TK_STDCAL* TKGL_FramebufferTexture2DMultisample)(GLenum target,
                                                                GLenum attachment,
                                                                GLenum textarget,
                                                                GLuint texture,
                                                                GLint level,
                                                                GLsizei samples);

  extern TKGL_FramebufferTexture2DMultisample tk_glFramebufferTexture2DMultisampleEXT;

#undef glFramebufferTexture2DMultisampleEXT
#define glFramebufferTexture2DMultisampleEXT tk_glFramebufferTexture2DMultisampleEXT

  typedef void(TK_STDCAL* TKGL_RenderbufferStorageMultisample)(GLenum target,
                                                               GLsizei samples,
                                                               GLenum internalformat,
                                                               GLsizei width,
                                                               GLsizei height);

  extern TKGL_RenderbufferStorageMultisample tk_glRenderbufferStorageMultisampleEXT;

#undef glRenderbufferStorageMultisampleEXT
#define glRenderbufferStorageMultisampleEXT tk_glRenderbufferStorageMultisampleEXT

  // GL_EXT_texture_filter_anisotropic
  //////////////////////////////////////////////////////////////////////////

  extern int TK_GL_EXT_texture_filter_anisotropic;

#undef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF

#undef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE

  void LoadGlFunctions(void* glGetProcAddres);
} // namespace ToolKit