/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "TKPlatform.h"

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

  // GL_EXT_debug_marker
  //////////////////////////////////////////

  typedef void(TK_STDCAL* TKGL_InsertEventMarker)(GLsizei length, const GLchar* marker);

  extern TKGL_InsertEventMarker tk_glInsertEventMarkerEXT;

#undef glInsertEventMarkerEXT
#define glInsertEventMarkerEXT tk_glInsertEventMarkerEXT

  typedef void(TK_STDCAL* TKGL_PopGroupMarker)(void);

  extern TKGL_PopGroupMarker tk_glPopGroupMarkerEXT;

#undef glPopGroupMarkerEXT
#define glPopGroupMarkerEXT tk_glPopGroupMarkerEXT

  typedef void(TK_STDCAL* TKGL_PushGroupMarker)(GLsizei length, const GLchar* marker);

  extern TKGL_PushGroupMarker tk_glPushGroupMarkerEXT;

#undef glPushGroupMarkerEXT
#define glPushGroupMarkerEXT tk_glPushGroupMarkerEXT

  // GL_EXT_multisampled_render_to_texture
  //////////////////////////////////////////

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
  //////////////////////////////////////////

  extern int TK_GL_EXT_texture_filter_anisotropic;

#undef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF

#undef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE

  // GL_OES_texture_float_linear
  //////////////////////////////////////////

  extern int TK_GL_OES_texture_float_linear;

  // GL Loader function
  //////////////////////////////////////////

  extern void LoadGlFunctions(void* glGetProcAddres);

} // namespace ToolKit