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

#pragma once

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
  
  TK_API void ImageSetVerticalOnLoad(bool val);

  TK_API void ImageFree(void* img);
}