#pragma once

#include <string>
#include <cstring>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <system_error>
#include <iomanip>
#include "Colors.h"

#define READ_END 0
#define WRITE_END 1

template <typename F, typename... Args>
inline int call_and_handle_errors(F fun, Args &&...args)
{

  int result = fun(std::forward<Args>(args)...);
  if (result < 0)
  {
    throw std::system_error(errno, std::generic_category());
  }
  return result;
}

class Test
{
  int pass_count;
  int fail_count;
  int unresolved_count;

public:
  Test() : pass_count(0), fail_count(0), unresolved_count(0) {}
  void report_summary()
  {
    std::cout << std::setfill('-') << std::setw(80) << "-" << std::endl;
    std::cout << "Test Summary Report " << std::endl;
    std::cout << std::setfill('-') << std::setw(80) << "-" << std::endl;
    std::cout << "Total: " << pass_count + fail_count + unresolved_count << std::endl;
    std::cout << GREENTEXT << "Passed: " << pass_count << std::endl;
    std::cout << REDTEXT << "Failed: " << fail_count << std::endl;
    std::cout << YELLOWTEXT << "Unresolved: " << unresolved_count << std::endl;
    std::cout << NORMALTEXT << std::setfill('-') << std::setw(80) << "-" << std::endl;
  }
  void run(std::string name, bool (*fun)())
  {
    int pipefd[2];
    call_and_handle_errors(pipe, pipefd);
    pid_t childpid = call_and_handle_errors(fork);
    if (childpid)
    {
      //parent
      char buff;
      int wstatus;

      call_and_handle_errors(close, pipefd[WRITE_END]);

      while (read(pipefd[READ_END], &buff, 1) > 0)
        std::cout << buff;
      std::cout << std::endl;

      call_and_handle_errors(close, pipefd[READ_END]);

      call_and_handle_errors(wait, &wstatus);
      if (WIFEXITED(wstatus))
      {
        if (0 == WEXITSTATUS(wstatus))
          pass_count++;
        else
          fail_count++;
      }
      else
      {
        unresolved_count++;
      }
    }
    else
    {
      //child
      call_and_handle_errors(close, pipefd[READ_END]);
      call_and_handle_errors(dup2, pipefd[WRITE_END], STDOUT_FILENO);
      std::cout << name << std::endl;
      if (fun())
        exit(EXIT_SUCCESS);
      else
        exit(EXIT_FAILURE);
    }
  }
};