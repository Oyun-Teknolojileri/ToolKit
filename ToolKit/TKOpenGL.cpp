/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#define GLAD_GLES2_IMPLEMENTATION
#include "TKOpenGL.h"

#include "Types.h"

namespace ToolKit
{

  TKGL_FramebufferTexture2DMultisample tk_glFramebufferTexture2DMultisampleEXT = nullptr;

  TKGL_RenderbufferStorageMultisample tk_glRenderbufferStorageMultisampleEXT   = nullptr;

  TKGL_InsertEventMarker tk_glInsertEventMarkerEXT                             = nullptr;

  TKGL_PopGroupMarker tk_glPopGroupMarkerEXT                                   = nullptr;

  TKGL_PushGroupMarker tk_glPushGroupMarkerEXT                                 = nullptr;

  TKGL_GetObjectLabel tk_glGetObjectLabelEXT                                   = nullptr;

  TKGL_LabelObject tk_glLabelObjectEXT                                         = nullptr;

  int TK_GL_EXT_texture_filter_anisotropic                                     = 0;

  int TK_GL_OES_texture_float_linear                                           = 0;

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

  #ifdef GL_OES_texture_float_linear
    TK_GL_OES_texture_float_linear = GLAD_GL_OES_texture_float_linear;
  #endif

  #ifdef GL_EXT_debug_marker

    tk_glInsertEventMarkerEXT = glad_glInsertEventMarkerEXT;
    tk_glPopGroupMarkerEXT    = glad_glPopGroupMarkerEXT;
    tk_glPushGroupMarkerEXT   = glad_glPushGroupMarkerEXT;

  #endif

  #ifdef GL_EXT_debug_label

    tk_glLabelObjectEXT    = glad_glLabelObjectEXT;
    tk_glGetObjectLabelEXT = glad_glGetObjectLabelEXT;

  #endif

#endif

#ifdef TK_ANDROID

    typedef void* (*GL_PROC_ADDR)(const char*);
    GL_PROC_ADDR glLoader = (GL_PROC_ADDR) glGetProcAddres;

    tk_glRenderbufferStorageMultisampleEXT =
        (TKGL_RenderbufferStorageMultisample) glLoader("glRenderbufferStorageMultisampleEXT");
    tk_glFramebufferTexture2DMultisampleEXT =
        (TKGL_FramebufferTexture2DMultisample) glLoader("glFramebufferTexture2DMultisampleEXT");

    tk_glInsertEventMarkerEXT         = (TKGL_InsertEventMarker) glLoader("glInsertEventMarkerEXT");
    tk_glPopGroupMarkerEXT            = (TKGL_PopGroupMarker) glLoader("glPopGroupMarkerEXT");
    tk_glPushGroupMarkerEXT           = (TKGL_PushGroupMarker) glLoader("glPushGroupMarkerEXT");
    tk_glLabelObjectEXT               = (TKGL_LabelObject) glLoader("glLabelObjectEXT");
    tk_glGetObjectLabelEXT            = (TKGL_GetObjectLabel) glLoader("glGetObjectLabelEXT");

    // String Checks for Extensions.
    PFNGLGETSTRINGPROC tk_glGetString = nullptr;
    tk_glGetString                    = (PFNGLGETSTRINGPROC) glLoader("glGetString");

    if (tk_glGetString)
    {
      const GLubyte* extensions = tk_glGetString(GL_EXTENSIONS);
      if (extensions != nullptr)
      {
        String extensionsStr((const char*) extensions);
        TK_GL_OES_texture_float_linear       = extensionsStr.find("GL_OES_texture_float_linear") != String::npos;
        TK_GL_EXT_texture_filter_anisotropic = extensionsStr.find("GL_EXT_texture_filter_anisotropic") != String::npos;
      }
    }

#endif
  }

} // namespace ToolKit
