#include <vector>
#include <pthread.h>
#include <iostream>
#include <queue>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>

#define MAX_THREADS 16
#define MAX_TASKS 100

class ThreadPool
{
public:
  ThreadPool(int minThreads) : threads(minThreads), running(0), exit(false),
                               threadsMutex(PTHREAD_MUTEX_INITIALIZER),
                               tasksMutex(PTHREAD_MUTEX_INITIALIZER),
                               tasksCond(PTHREAD_COND_INITIALIZER),
                               runningMutex(PTHREAD_MUTEX_INITIALIZER)
  {
    if (minThreads < 0 || minThreads > 16)
      throw std::runtime_error("Thread count should be between 0 and 16");

    for (int i = 0; i < minThreads; ++i)
    {
      pthread_t t;
      if (pthread_create(&t, nullptr, workerThread, this) != 0)
        throw std::system_error(errno, std::generic_category());
      threads[i] = t;
    }
  }
  ~ThreadPool()
  {
    exitPool();
  }
  void exitPool()
  {
    if (pthread_mutex_lock(&tasksMutex) != 0)
      throw std::system_error(errno, std::generic_category());
    exit = true;
    if (pthread_cond_broadcast(&tasksCond) != 0)
      throw std::system_error(errno, std::generic_category());
    if (pthread_mutex_unlock(&tasksMutex) != 0)
      throw std::system_error(errno, std::generic_category());

    for (auto t : threads)
    {
      if (pthread_join(t, nullptr) != 0)
        throw std::system_error(errno, std::generic_category());
    }
  }
  static void *workerThread(void *args)
  {
    ThreadPool *pool = (ThreadPool *)args;
    while (true)
    {
      try
      {
        if (pthread_mutex_lock(&pool->tasksMutex) != 0)
          throw std::system_error(errno, std::generic_category());

        while (pool->tasks.empty() && !pool->exit)
        {
          if (pthread_cond_wait(&pool->tasksCond, &pool->tasksMutex) != 0)
            throw std::system_error(errno, std::generic_category());
        }
        if (pool->exit && pool->tasks.empty())
        {
          if (pthread_mutex_unlock(&pool->tasksMutex) != 0)
            throw std::system_error(errno, std::generic_category());
          return nullptr;
        }
        auto task = pool->tasks.front();
        pool->tasks.pop();
        if (pthread_mutex_unlock(&pool->tasksMutex) != 0)
          throw std::system_error(errno, std::generic_category());

        if (pthread_mutex_lock(&pool->runningMutex) != 0)
          throw std::system_error(errno, std::generic_category());
        pool->running++;
        if (pthread_mutex_unlock(&pool->runningMutex) != 0)
          throw std::system_error(errno, std::generic_category());

        task();

        if (pthread_mutex_lock(&pool->runningMutex) != 0)
          throw std::system_error(errno, std::generic_category());
        pool->running--;
        if (pthread_mutex_unlock(&pool->runningMutex) != 0)
          throw std::system_error(errno, std::generic_category());
      }
      catch (const std::exception &err)
      {
        std::cout << "Error: "
                  << " - "
                  << err.what() << std::endl;
      }
    }
  }
  void addTask(void (*func)())
  {
    if (pthread_mutex_lock(&runningMutex) != 0)
      throw std::system_error(errno, std::generic_category());
    if (pthread_mutex_lock(&threadsMutex) != 0)
      throw std::system_error(errno, std::generic_category());

    bool add_new_thread = (running == threads.size() && threads.size() < MAX_THREADS);
    if (pthread_mutex_unlock(&runningMutex) != 0)
      throw std::system_error(errno, std::generic_category());

    if (add_new_thread)
    {
      pthread_t t;
      if (pthread_create(&t, nullptr, workerThread, this) != 0)
        throw std::system_error(errno, std::generic_category());
      threads.push_back(t);
    }
    if (pthread_mutex_unlock(&threadsMutex) != 0)
      throw std::system_error(errno, std::generic_category());

    if (pthread_mutex_lock(&tasksMutex) != 0)
      throw std::system_error(errno, std::generic_category());

    if (tasks.size() == MAX_TASKS)
    {
      if (pthread_mutex_unlock(&tasksMutex) != 0)
        throw std::system_error(errno, std::generic_category());
      throw std::runtime_error("Cannot add more tasks");
    }
    tasks.push(func);
    if (pthread_cond_signal(&tasksCond) != 0)
      throw std::system_error(errno, std::generic_category());

    if (pthread_mutex_unlock(&tasksMutex) != 0)
      throw std::system_error(errno, std::generic_category());
  }
  std::vector<pthread_t> threads;
  std::queue<void (*)()> tasks;
  int running;
  bool exit;
  pthread_mutex_t threadsMutex;
  pthread_mutex_t tasksMutex;
  pthread_cond_t tasksCond;
  pthread_mutex_t runningMutex;
};