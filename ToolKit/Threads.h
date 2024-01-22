/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#define POOLSTL_STD_SUPPLEMENT 1
#include <poolSTL/poolstl.hpp>

#ifdef _WIN32
  #include <execution>
#endif

#include <mutex>
