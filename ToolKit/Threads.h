/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Types.h"

#include <poolSTL/poolstl.hpp>

namespace ToolKit
{

  typedef task_thread_pool::task_thread_pool ThreadPool;

  /**
   * This is the class that keeps the thread pools and manages async tasks.
   */
  class TK_API WorkerManager
  {
   public:
    /**
     * Predefined thread pools for specific jobs.
     */
    enum Executor
    {
      FramePool //!< Tasks that need to be completed within the frame should use this pool.
    };

   public:
    WorkerManager();                            //!< Default constructor, initializes thread pools.
    virtual ~WorkerManager();                   //!< Default destructor, destroy thread pools.
    ThreadPool& GetExecutor(Executor executor); //!< Returns the thread pool corresponding to the executor.

   public:
    ThreadPool* m_frameWorkers = nullptr; //!< Task that suppose to complete in a frame should be using this pool.
  };

  /**
   * Parallel loop execution target which lets the programmer to choose the thread pool to execute for loop on. Allows
   * to decide to run for loop sequential or parallel based on the given condition.
   */
#define TKExecByConditional(Condition, Target)                                                                         \
  poolstl::par_if((Condition) && Main::GetInstance()->m_threaded, GetWorkerManager()->GetExecutor(Target))

  /**
   * Parallel loop execution target which lets the programmer to choose the thread pool to execute for loop on.
   */
#define TKExecBy(Target) poolstl::par_if(Main::GetInstance()->m_threaded, GetWorkerManager()->GetExecutor(Target))

} // namespace ToolKit