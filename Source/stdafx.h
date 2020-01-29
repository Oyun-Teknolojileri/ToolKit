#pragma once
#pragma message("Compiling precompiled headers.\n")

#define GLM_FORCE_CTOR_INIT
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_MESSAGES 
#define GLM_ENABLE_EXPERIMENTAL

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_access.hpp"
#include "glm/gtc/epsilon.hpp"
#include "lodepng.h"
#include "GL/glew.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include <algorithm>
#include <vector>
