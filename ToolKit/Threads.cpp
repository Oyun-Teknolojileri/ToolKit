/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Threads.h"

#include "TKPlatform.h"
#include "ToolKit.h"

namespace ToolKit
{

  WorkerManager::WorkerManager() {}

  WorkerManager::~WorkerManager() { UnInit(); }

  void WorkerManager::Init()
  {
    uint coreCount = std::thread::hardware_concurrency();
    if constexpr (TK_PLATFORM == PLATFORM::TKWeb)
    {
      m_frameWorkers      = new ThreadPool(glm::min(coreCount, 2u));
      m_backgroundWorkers = new ThreadPool(glm::min(coreCount, 2u));
    }
    else
    {
      m_frameWorkers      = new ThreadPool(glm::min(coreCount, 4u));
      m_backgroundWorkers = new ThreadPool(glm::min(coreCount, 2u));
    }

    Main::GetInstance()->RegisterPostUpdateFunction([this](float deltaTime) -> void
                                                    { ExecuteTasks(m_mainThreadTasks, m_mainTaskMutex); });
  }

  void WorkerManager::UnInit()
  {
    SafeDel(m_frameWorkers);
    SafeDel(m_backgroundWorkers);
  }

  ThreadPool& WorkerManager::GetPool(Executor executor)
  {
    switch (executor)
    {
    case WorkerManager::Executor::BackgroundPool:
      return *m_backgroundWorkers;
    case WorkerManager::Executor::FramePool:
    default:
      return *m_frameWorkers;
      break;
    }
  }

  int WorkerManager::GetThreadCount(Executor executor)
  {
    return Main::GetInstance()->m_threaded ? GetPool(executor).get_num_threads() : 0;
  }

  void WorkerManager::Flush()
  {
    auto flushPoolFn = [](ThreadPool* pool) -> void
    {
      pool->pause();
      pool->wait_for_tasks();
      pool->unpause();
    };

    flushPoolFn(m_frameWorkers);
    flushPoolFn(m_backgroundWorkers);

    ExecuteTasks(m_mainThreadTasks, m_mainTaskMutex);
  }

  void WorkerManager::ExecuteTasks(TaskQueue& queue, std::mutex& mex)
  {
    for (int i = 0; i < (int) queue.size(); i++)
    {
      mex.lock();
      std::packaged_task<void()> task {std::move(queue.front())};
      m_mainThreadTasks.pop();
      mex.unlock();

      task();
    }
  }

} // namespace ToolKit
