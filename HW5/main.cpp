#include <iostream>
#include "ThreadPool.h"

int main()
{
  try
  {
    ThreadPool pool(5);
    pool.addTask([]()
                 {
                   sleep(5);
                   std::cout << "task 1" << std::endl;
                 });
    pool.addTask([]()
                 {
                   sleep(5);
                   std::cout << "task 2" << std::endl;
                 });
    pool.addTask([]()
                 {
                   sleep(5);
                   std::cout << "task 3" << std::endl;
                 });
    pool.addTask([]()
                 {
                   sleep(5);
                   std::cout << "task 4" << std::endl;
                 });
    pool.addTask([]()
                 {
                   sleep(5);
                   std::cout << "task 5" << std::endl;
                 });
    pool.addTask([]()
                 {
                   sleep(5);
                   std::cout << "task 6" << std::endl;
                 });
    pool.addTask([]()
                 {
                   sleep(5);
                   std::cout << "task 7" << std::endl;
                 });
    pool.addTask([]()
                 {
                   sleep(5);
                   std::cout << "task 8" << std::endl;
                 });
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