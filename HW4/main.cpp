#include <string>
#include <iostream>
#include <unistd.h>
#include "MessageQueue.h"

int main()
{

  try
  {
    MessageQueue<std::string> mq;

    mq.registerHandler([](std::string msg)
                       {
                         sleep(1);
                         std::cout << "1 handler: " + msg + "\n";
                       });
    mq.registerHandler([](std::string msg)
                       {
                         sleep(1);
                         std::cout << "2 handler: " + msg + "\n";
                       });
    mq.registerHandler([](std::string msg)
                       {
                         sleep(1);
                         std::cout << "3 handler: " + msg + "\n";
                       });

    mq.addMessage("Message One");
    mq.addMessage("Message Two");
    sleep(5);
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