/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#ifdef IM_EXPORT
	#define IMGUI_API __declspec( dllexport )
#else
	#define IMGUI_API __declspec( dllimport )
#endif

#define TK_NO_POLYGON_MODE

#include "../glm/glm/glm.hpp"

#define IM_VEC2_CLASS_EXTRA                                                                                            \
  ImVec2(const glm::vec2& f)                                                                                           \
  {                                                                                                                    \
    x = f.x;                                                                                                           \
    y = f.y;                                                                                                           \
  }                                                                                                                    \
  operator glm::vec2() const { return glm::vec2(x, y); }

#define IM_VEC4_CLASS_EXTRA                                                                                            \
  ImVec4(const glm::vec4& f)                                                                                           \
  {                                                                                                                    \
    x = f.x;                                                                                                           \
    y = f.y;                                                                                                           \
    z = f.z;                                                                                                           \
    w = f.w;                                                                                                           \
  }                                                                                                                    \
  operator glm::vec4() const { return glm::vec4(x, y, z, w); }
