
#include <iostream>
#include <unistd.h>
#include "Shared.h"

int main()
{
  try
  {
    Shared &sh = Shared::getInstance();

    sh.registerCallback([](std::string msg)
                        { std::cout << "cb1: " << msg << std::endl; });

    sh.registerCallback([](std::string msg)
                        { std::cout << "cb2: " << msg << std::endl; });
    while(true);
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