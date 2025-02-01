/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#ifdef __ARM_FP
  // enables simd on android phones, if supported
  #define STBI_NEON
#endif
#include "stb/stb_image.h"

namespace ToolKit
{

  ubyte* ImageLoad(StringView filename, int* x, int* y, int* comp, int req_comp)
  {
    return stbi_load(filename.data(), x, y, comp, req_comp);
  }

  float* ImageLoadF(StringView filename, int* x, int* y, int* comp, int req_comp)
  {
    return stbi_loadf(filename.data(), x, y, comp, req_comp);
  }

  ubyte* ImageLoadFromMemory(const ubyte* buffer, int len, int* x, int* y, int* comp, int req_comp)
  {
    return stbi_load_from_memory(buffer, len, x, y, comp, req_comp);
  }

  float* ImageLoadFromMemoryF(const ubyte* buffer, int len, int* x, int* y, int* comp, int req_comp)
  {
    return stbi_loadf_from_memory(buffer, len, x, y, comp, req_comp);
  }

  int ImageResize(const ubyte* input_pixels,
                  int input_w,
                  int input_h,
                  int input_stride_in_bytes,
                  ubyte* output_pixels,
                  int output_w,
                  int output_h,
                  int output_stride_in_bytes,
                  int num_channels)
  {
    return stbir_resize_uint8(input_pixels,
                              input_w,
                              input_h,
                              input_stride_in_bytes,
                              output_pixels,
                              output_w,
                              output_h,
                              output_stride_in_bytes,
                              num_channels);
  }

  int WritePNG(StringView filename, int x, int y, int comp, const void* data, int stride_bytes)
  {
    return stbi_write_png(filename.data(), x, y, comp, data, stride_bytes);
  }

  int WriteHdr(StringView filename, int x, int y, int comp, const float* data)
  {
    return stbi_write_hdr(filename.data(), x, y, comp, data);
  }

  void ImageSetVerticalOnLoad(bool val) { stbi_set_flip_vertically_on_load(val); }

  void ImageFree(void* img) { stbi_image_free(img); }

} // namespace ToolKit
