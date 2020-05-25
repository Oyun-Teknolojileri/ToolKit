#pragma once
#pragma message("Compiling precompiled headers.\n")

// GLM
#define GLM_FORCE_CTOR_INIT
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_MESSAGES 
#define GLM_ENABLE_EXPERIMENTAL

#include "glm/glm.hpp"
#include "glm/gtc/epsilon.hpp"
#include "glm/gtc/matrix_access.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/matrix_operation.hpp"
#include "glm/gtx/matrix_query.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/closest_point.hpp"
#include "glm/gtx/scalar_relational.hpp"
#include "glm/gtx/vector_query.hpp"
#include "glm/gtx/string_cast.hpp"

// LoadPng
#include "stb/stb_image.h"

// Glew
#include "GL/glew.h"

// RapidXml
#include "rapidxml_ext.h"

#ifdef TK_EDITOR
	// ImGui
	#include "ImGui/imgui.h"
	#include "ImGui/imgui_internal.h"
#endif 

// STL
#include <algorithm>
#include <assert.h>
#include <unordered_map>
#include <vector>
