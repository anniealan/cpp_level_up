#include <iostream>
#include "Test.h"

int main()
{
  try
  {
    Test test;
    test.run("Testing success function", []()
             {
               sleep(2);
               std::cout << "running..." << std::endl;
               sleep(2);
               return true;
             });
    test.run("Testing failure function", []()
             {
               sleep(2);
               std::cout << "running..." << std::endl;
               sleep(2);
               return false;
             });
    test.run("Testing abort function", []()
             {
               sleep(2);
               std::cout << "running..." << std::endl;
               sleep(2);
               abort();
               return false;
             });
    test.run("Testing another success function", []()
             {
               sleep(2);
               std::cout << "running..." << std::endl;
               sleep(2);
               return true;
             });
    test.report_summary();
  }
  catch (const std::system_error &err)
  {
    std::cout << "Error: " << err.code() << " - "
              << err.code().message() << std::endl;
    return 1;
  }
  return 0;
}