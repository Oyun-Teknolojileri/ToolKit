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

  TK_API ubyte* ImageLoad(StringView filename, int* x, int* y, int* comp, int reqComp);
  TK_API float* ImageLoadF(StringView filename, int* x, int* y, int* comp, int reqComp);
  TK_API ubyte* ImageLoadFromMemory(const ubyte* buffer, int len, int* x, int* y, int* comp, int reqComp);
  TK_API float* ImageLoadFromMemoryF(const ubyte* buffer, int len, int* x, int* y, int* comp, int reqComp);

  TK_API int ImageResize(const ubyte* input_pixels,
                         int input_w,
                         int input_h,
                         int input_stride_in_bytes,
                         ubyte* output_pixels,
                         int output_w,
                         int output_h,
                         int output_stride_in_bytes,
                         int num_channels);

  TK_API int WritePNG(StringView filename, int x, int y, int comp, const void* data, int stride_bytes);
  TK_API int WriteHdr(StringView filename, int x, int y, int comp, const float* data);
  TK_API void ImageSetVerticalOnLoad(bool val);
  TK_API void ImageFree(void* img);

} // namespace ToolKit