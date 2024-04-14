/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Threads.h"

#include "TKPlatform.h"



namespace ToolKit
{

  WorkerManager::WorkerManager() {}

  WorkerManager::~WorkerManager() { UnInit(); }

  void WorkerManager::Init()
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

    Main::GetInstance()->RegisterPostUpdateFunction([this](float deltaTime) -> void
                                                    { ExecuteTasks(m_mainThreadTasks, m_mainTaskMutex); });
  }

  void WorkerManager::UnInit() { SafeDel(m_frameWorkers); }

  ThreadPool& WorkerManager::GetPool(Executor executor)
  {
    switch (executor)
    {
    case WorkerManager::Executor::FramePool:
    default:
      return *m_frameWorkers;
      break;
    }
  }

  void WorkerManager::Flush()
  {
    m_frameWorkers->pause();
    m_frameWorkers->wait_for_tasks();
    m_frameWorkers->unpause();

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
