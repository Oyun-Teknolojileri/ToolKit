/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazilim
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "TKImage.h"

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
  unsigned char* ImageLoad(const char* filename, int* x, int* y, int* comp, int req_comp)
  {
    return stbi_load(filename, x, y, comp, req_comp);
  }

  float* ImageLoadF(const char* filename, int* x, int* y, int* comp, int req_comp)
  {
    return stbi_loadf(filename, x, y, comp, req_comp);
  }

  unsigned char* ImageLoadFromMemory(const unsigned char* buffer, int len, int* x, int* y, int* comp, int req_comp)
  {
    return stbi_load_from_memory(buffer, len, x, y, comp, req_comp);
  }

  float* ImageLoadFromMemoryF(const unsigned char* buffer, int len, int* x, int* y, int* comp, int req_comp)
  {
    return stbi_loadf_from_memory(buffer, len, x, y, comp, req_comp);
  }

  int ImageResize(const unsigned char *input_pixels,
                  int input_w , 
                  int input_h , 
                  int input_stride_in_bytes,
                  unsigned char *output_pixels, 
                  int output_w, 
                  int output_h, 
                  int output_stride_in_bytes,
                  int num_channels)
  {
    return stbir_resize_uint8(input_pixels,
                              input_w ,
                              input_h ,
                              input_stride_in_bytes,
                              output_pixels,
                              output_w,
                              output_h,
                              output_stride_in_bytes,
                              num_channels);
  }

  int WritePNG(const char* filename, int x, int y, int comp, const void* data, int stride_bytes)
  {
    return stbi_write_png(filename, x, y, comp, data, stride_bytes);
  }

  void ImageSetVerticalOnLoad(bool val) { stbi_set_flip_vertically_on_load(val); }

  void ImageFree(void* img) { stbi_image_free(img); }
} // namespace ToolKit
