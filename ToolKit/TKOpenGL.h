#ifdef __ANDROID__
  #include <GLES3/gl32.h>
#elif defined(__EMSCRIPTEN__)
  #include <GL/glew.h>
#else
  #include <gles2.h>
#endif