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

namespace ToolKit
{

  typedef task_thread_pool::task_thread_pool ThreadPool;

  /**
   * This is the class that keeps the thread pools and manages async tasks.
   */
  class WorkerManager
  {
   public:
    enum Executor
    {
      FramePool
    };

   public:
    WorkerManager();
    ThreadPool& GetExecutor(Executor executor);

   public:
    ThreadPool m_frameWorkers; //!< Task that suppose to complete in a frame should be using this pool.
  };

#define TKExecByConditional(Condition, Target)                                                                         \
  poolstl::par_if((Condition) && Main::GetInstance()->m_threaded, GetWorkerManager()->GetExecutor(Target))

#define TKExecBy(Target) poolstl::par_if(Main::GetInstance()->m_threaded, GetWorkerManager()->GetExecutor(Target))

} // namespace ToolKit