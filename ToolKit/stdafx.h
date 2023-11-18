/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

// STL
#include <assert.h>

#include <algorithm>
#include <random>
#include <unordered_map>

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

// RapidXml
#include "RapidXml/rapidxml_ext.h"
#include "RapidXml/rapidxml_utils.hpp"

// ToolKit
#include "Events.h"
#include "Logger.h"
#include "Serialize.h"
#include "ToolKit.h"