#pragma once

// STL
#include <assert.h>

#include <algorithm>
#include <random>
#include <unordered_map>
#include <vector>

#ifndef __EMSCRIPTEN__
  #pragma message("Compiling precompiled headers.\n")
  #define GLM_FORCE_MESSAGES
#endif

// GLM
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
#include "glm/gtx/closest_point.hpp"
#include "glm/gtx/component_wise.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/matrix_operation.hpp"
#include "glm/gtx/matrix_query.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/scalar_relational.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/vector_query.hpp"
#include "glm/gtc/random.hpp"

// Stb
#include "stb/stb_image.h"

// RapidXml
#include "rapidxml_ext.h"
#include "rapidxml_utils.hpp"

// ToolKit
#include "ToolKit.h"
#include "Events.h"
#include "Logger.h"
#include "Serialize.h"

#ifdef TK_EDITOR

#include "ImGui/imgui.h"
#include "ImGui/misc/cpp/imgui_stdlib.h"
#include "Imgui/backends/imgui_impl_opengl3.h"
#include "Imgui/backends/imgui_impl_sdl2.h"

#endif