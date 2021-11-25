
#include <iostream>
#include <unistd.h>
#include "Shared.h"

int main()
{
  try
  {
    Shared &sh = Shared::getInstance();
    sh.setMessage("msg1");
    sleep(1);
    sh.setMessage("msg2");
    sleep(1);
    sh.setMessage("msg3");
    sleep(1);
    sh.setMessage("msg4");
    sleep(1);
    std::cout << sh.getMessage() << std::endl;
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