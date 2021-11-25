#include <iostream>
#include "ThreadPool.h"

void func1(void *data)
{
  int *n = (int *)data;
  sleep(5);
  std::cout << "task " << *n << std::endl;
  delete n;
}

void func2(void *data)
{
  char *msg = (char *)data;
  sleep(5);
  std::cout << msg << std::endl;
}

int main()
{
  try
  {
    ThreadPool pool(5);
    pool.addTask(func1, new int(1));
    pool.addTask(func1, new int(2));
    pool.addTask(func1, new int(3));
    pool.addTask(func1, new int(4));
    std::string msg1 = "Hello";
    std::string msg2 = "World";
    pool.addTask(func2, (void*)msg1.c_str());
    pool.addTask(func2, (void*)msg2.c_str());
  }
  catch (const std::exception &err)
  {
    std::cout << "Error: "
              << " - "
              << err.what() << std::endl;
    return 1;
  }
  return 0;
}