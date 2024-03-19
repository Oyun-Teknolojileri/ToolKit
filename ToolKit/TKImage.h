/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Types.h"

namespace ToolKit
{
  TK_API unsigned char* ImageLoad(const char* filename, int* x, int* y, int* comp, int req_comp);
  TK_API float* ImageLoadF(const char* filename, int* x, int* y, int* comp, int req_comp);

  TK_API unsigned char* ImageLoadFromMemory(const unsigned char* buffer,
                                            int len,
                                            int* x,
                                            int* y,
                                            int* comp,
                                            int req_comp);

  TK_API float* ImageLoadFromMemoryF(const unsigned char* buffer, int len, int* x, int* y, int* comp, int req_comp);

  TK_API int ImageResize(const unsigned char* input_pixels,
                         int input_w,
                         int input_h,
                         int input_stride_in_bytes,
                         unsigned char* output_pixels,
                         int output_w,
                         int output_h,
                         int output_stride_in_bytes,
                         int num_channels);

  TK_API int WritePNG(const char* filename, int x, int y, int comp, const void* data, int stride_bytes);

  TK_API int WriteHDR(const char* filename, int x, int y, int comp, const float* data);

  TK_API void ImageSetVerticalOnLoad(bool val);

  TK_API void ImageFree(void* img);
} // namespace ToolKit