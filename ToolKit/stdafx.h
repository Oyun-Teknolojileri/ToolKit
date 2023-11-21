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
#include <filesystem>
#include <functional>
#include <limits>
#include <memory>
#include <random>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// GLM
#ifndef TK_GLM
  #define TK_GLM
  #define GLM_FORCE_QUAT_DATA_XYZW
  #define GLM_FORCE_XYZW_ONLY
  #define GLM_FORCE_CTOR_INIT
  #define GLM_ENABLE_EXPERIMENTAL
  #define GLM_FORCE_ALIGNED_GENTYPES
  #define GLM_FORCE_INTRINSICS
#endif

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/scalar_relational.hpp>

// RapidXml
#include <RapidXml/rapidxml_ext.h>
#include <RapidXml/rapidxml_utils.hpp>

// ToolKit
#include "Entity.h"
#include "Events.h"
#include "Logger.h"
#include "Pass.h"
#include "Serialize.h"

#ifdef TK_EDITOR
  #include "App.h"
  #include "EditorRenderer.h"
#endif
