/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Threads.h"

#include "TKPlatform.h"

#include "DebugNew.h"

namespace ToolKit
{

  WorkerManager::WorkerManager()
  {
    uint coreCount = std::thread::hardware_concurrency();
    if constexpr (TK_PLATFORM == PLATFORM::TKWeb)
    {
      m_frameWorkers = new ThreadPool(glm::min(coreCount, 4u));
    }
    else
    {
      m_frameWorkers = new ThreadPool(glm::min(coreCount, 8u));
    }
  }

  WorkerManager::~WorkerManager() { SafeDel(m_frameWorkers); }

  ThreadPool& WorkerManager::GetExecutor(Executor executor)
  {
    switch (executor)
    {
    case WorkerManager::Executor::FramePool:
    default:
      return *m_frameWorkers;
      break;
    }
  }

} // namespace ToolKit
