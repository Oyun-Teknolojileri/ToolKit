/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Types.h"

#include <poolstl/poolstl.hpp>

#include <future>

namespace ToolKit
{

  typedef task_thread_pool::task_thread_pool ThreadPool;
  typedef std::queue<std::packaged_task<void()>> TaskQueue;
  typedef std::function<void()> Task;

  /** This is the class that keeps the thread pools and manages async tasks. */
  class TK_API WorkerManager
  {
   public:
    /** Predefined thread pools for specific jobs. */
    enum Executor
    {
      MainThread, //!< Tasks in this executor runs in sync with main thread at the end of the current frame.
      FramePool   //!< Tasks that need to be completed within the frame should use this pool.
    };

   public:
    /** Default constructor, initializes thread pools. */
    WorkerManager();

    /** Default destructor, destroy thread pools. */
    ~WorkerManager();

    /** Initialize threads,  pools and task queues. */
    void Init();

    /** Flushes all the tasks in the pools, queues than terminates threads. */
    void UnInit();

    /** Returns the thread pool corresponding to the executor. */
    ThreadPool& GetPool(Executor executor);

    /** Stops waiting tasks and completes ongoing tasks on all pools and threads. */
    void Flush();

    template <typename F, typename... A, typename R = std::invoke_result_t<std::decay_t<F>, std::decay_t<A>...>>
    std::future<R> AsyncTask(Executor exec, F&& func, A&&... args)
    {
      if (exec == FramePool)
      {
        return m_frameWorkers->submit(func, std::forward<A>(args)...);
      }
      else if (exec == MainThread)
      {
        std::shared_ptr<std::packaged_task<R()>> ptask =
            std::make_shared<std::packaged_task<R()>>(std::bind(std::forward<F>(func), std::forward<A>(args)...));

        const std::lock_guard<std::mutex> tasks_lock(m_mainTaskMutex);
        m_mainThreadTasks.emplace(std::bind(std::forward<F>(func), std::forward<A>(args)...));

        return ptask->get_future();
      }

      return std::future<void>();
    };

   private:
    void ExecuteTasks(TaskQueue& queue, std::mutex& mex);

   public:
    /** Task that suppose to complete in a frame should be using this pool. */
    ThreadPool* m_frameWorkers = nullptr;

    /** Tasks that will be executed at the main thread frame end is stored here. */
    TaskQueue m_mainThreadTasks;

   private:
    /** Lock for main thread tasks. */
    std::mutex m_mainTaskMutex;
  };

  /**
   * Parallel loop execution target which lets the programmer to choose the thread pool to execute for loop on.
   * Allows to decide to run for loop sequential or parallel based on the given condition.
   */
#define TKExecByConditional(Condition, Target)                                                                         \
  poolstl::par_if((Condition) && Main::GetInstance()->m_threaded, GetWorkerManager()->GetPool(Target))

  /** Parallel loop execution target which lets the programmer to choose the thread pool to execute for loop on. */
#define TKExecBy(Target)         poolstl::par_if(Main::GetInstance()->m_threaded, GetWorkerManager()->GetPool(Target))

  /** Insert an async task to given target. */
#define TKAsyncTask(Target, ...) GetWorkerManager()->AsyncTask(Target, __VA_ARGS__);

} // namespace ToolKit