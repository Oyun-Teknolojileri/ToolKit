/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
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

// STL
#include <assert.h>

#include <algorithm>
#include <random>
#include <unordered_map>
#include <vector>

// GLM
#define GLM_FORCE_XYZW_ONLY
#define GLM_FORCE_CTOR_INIT
#define GLM_ENABLE_EXPERIMENTAL
#ifndef GLM_FORCE_SWIZZLE
  #define GLM_FORCE_SWIZZLE
#endif

#include "glm/glm.hpp"
#include "glm/gtc/epsilon.hpp"
#include "glm/gtc/matrix_access.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/random.hpp"
#include "glm/gtx/closest_point.hpp"
#include "glm/gtx/component_wise.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/matrix_operation.hpp"
#include "glm/gtx/matrix_query.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/scalar_relational.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/vector_query.hpp"

// Stb
#include "stb/stb_image.h"

// RapidXml
#include "rapidxml_ext.h"
#include "rapidxml_utils.hpp"

// ToolKit
#include "Events.h"
#include "Logger.h"
#include "Serialize.h"
#include "ToolKit.h"

#ifdef TK_EDITOR

  #include "ImGui/imgui.h"
  #include "ImGui/misc/cpp/imgui_stdlib.h"
  #include "Imgui/backends/imgui_impl_opengl3.h"
  #include "Imgui/backends/imgui_impl_sdl2.h"

#endif