#pragma once

#include <string>
#include <queue>
#include <mutex>
#include <pthread.h>
#include <unistd.h>
#include <functional>
#include <errno.h>

#define QUEUE_MAX_LEN 100

struct Task
{
  void (*handler)(std::string);
  std::string msg;
};

class MessageQueue
{
public:
  MessageQueue() : messagesMutex(PTHREAD_MUTEX_INITIALIZER),
                   handlersMutex(PTHREAD_MUTEX_INITIALIZER),
                   messagesCond(PTHREAD_COND_INITIALIZER)
  {
    pthread_t t;
    pthread_attr_t attr;
    if (pthread_attr_init(&attr) != 0)
      throw std::system_error(errno, std::generic_category());
    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0)
      throw std::system_error(errno, std::generic_category());
    if (pthread_create(&t, &attr, workerThread, this) != 0)
      throw std::system_error(errno, std::generic_category());
    if (pthread_attr_destroy(&attr) != 0)
      throw std::system_error(errno, std::generic_category());
  }
  void addMessage(std::string msg)
  {
    if (pthread_mutex_lock(&messagesMutex) != 0)
      throw std::system_error(errno, std::generic_category());
    if (messages.size() >= QUEUE_MAX_LEN)
    {
      if (pthread_mutex_unlock(&messagesMutex) != 0)
        throw std::system_error(errno, std::generic_category());
      throw std::runtime_error("Message queue is full");
    }
    messages.push(msg);
    if (pthread_cond_signal(&messagesCond) != 0)
      throw std::system_error(errno, std::generic_category());
    if (pthread_mutex_unlock(&messagesMutex) != 0)
      throw std::system_error(errno, std::generic_category());
  }
  void registerHandler(void (*handler)(std::string))
  {
    if (pthread_mutex_lock(&handlersMutex) != 0)
      throw std::system_error(errno, std::generic_category());
    handlers.push_back(handler);
    if (pthread_mutex_unlock(&handlersMutex) != 0)
      throw std::system_error(errno, std::generic_category());
  }

  static void *workerThread(void *args)
  {
    MessageQueue *mq = (MessageQueue *)args;
    while (true)
    {
      if (pthread_mutex_lock(&mq->messagesMutex) != 0)
        throw std::system_error(errno, std::generic_category());
      while (mq->messages.empty())
      {
        if (pthread_cond_wait(&mq->messagesCond, &mq->messagesMutex) != 0)
          throw std::system_error(errno, std::generic_category());
      }
      std::string msg = mq->messages.front();
      mq->messages.pop();
      if (pthread_mutex_unlock(&mq->messagesMutex) != 0)
        throw std::system_error(errno, std::generic_category());

      if (pthread_mutex_lock(&mq->handlersMutex) != 0)
        throw std::system_error(errno, std::generic_category());
      pthread_t th[mq->handlers.size()];
      for (size_t i = 0; i < mq->handlers.size(); ++i)
      {
        Task *task = new Task{mq->handlers[i], msg};
        if (pthread_create(&th[i], nullptr, executeTask, (void *)task) != 0)
          throw std::system_error(errno, std::generic_category());
      }
      if (pthread_mutex_unlock(&mq->handlersMutex) != 0)
        throw std::system_error(errno, std::generic_category());
    }
  }

  static void *executeTask(void *args)
  {
    if (pthread_detach(pthread_self()) != 0)
      throw std::system_error(errno, std::generic_category());

    Task *task = (Task *)args;
    task->handler(task->msg);
    delete task;
  }

private:
  std::queue<std::string> messages;
  std::vector<void (*)(std::string)> handlers;
  pthread_mutex_t messagesMutex;
  pthread_cond_t messagesCond;
  pthread_mutex_t handlersMutex;
};