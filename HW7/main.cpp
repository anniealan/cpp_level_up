#include <iostream>
#include <string.h>
#include <algorithm>
#include "../HW5/ThreadPool.h"
#include "../HW4/MessageQueue.h"
#include "SocketServer.h"

struct Msg
{
  std::string content;
  int owner;
};

MessageQueue<Msg> mq;
std::vector<int> clients;

#define MAX_PENDING_SOCK_CONN 5
#define MIN_THREADS 5
#define BUFFER_SIZE 20

void threadfunc(void *data)
{
  int *newsockid = (int *)data;
  char resp[BUFFER_SIZE];
  while (true)
  {
    // receive a message from client socket, returns number of bytes
    int read_size = recv(*newsockid, (void *)resp, BUFFER_SIZE - 1, 0);

    if (read_size < 0)
      throw std::system_error(errno, std::generic_category());
    // in case of shutdown 0 is returned
    if (0 == read_size || !strncmp(resp, "quit", 4))
    {
      auto it = std::find(clients.begin(), clients.end(), *newsockid);
      if (it != clients.end())
      {
        clients.erase(it);
        if (close(*newsockid) < 0)
          throw std::system_error(errno, std::generic_category());
      }
      break;
    }
    else
    {
      resp[read_size] = '\0';
      Msg msg{resp, *newsockid};
      mq.addMessage(msg);
    }
  }
  delete newsockid;
}
class Application
{
public:
  void run()
  {
    mq.registerHandler([](Msg msg)
                       {
                         for (auto client : clients)
                         {
                           if (client != msg.owner)
                           {
                             if (write(client, (void *)msg.content.c_str(), msg.content.size()) < 0)
                               throw std::system_error(errno, std::generic_category());
                           }
                         }
                       });
    SocketServer sock(ip_address, port, MAX_PENDING_SOCK_CONN);

    ThreadPool pool(MIN_THREADS);
    std::string wlcMsg = "Hello!, use `quit` to exit\n";
    while (true)
    {
      int newsockid = sock.accept();
      if (write(newsockid, (void *)wlcMsg.c_str(), wlcMsg.size()) < 0)
        throw std::system_error(errno, std::generic_category());

      clients.push_back(newsockid);
      pool.addTask(threadfunc, new int(newsockid));
    }
  }

public:
  std::string ip_address;
  int port;
};

int main(int argc, char **argv)
{
  try
  {

    Application app;

    bool cmdlerr = false;
    if (argc != 3)
    {
      cmdlerr = true;
    }
    else
    {
      app.ip_address = argv[1];
      app.port = atoi(argv[2]);
    }

    if (cmdlerr || app.ip_address.empty() || !app.port)
    {
      std::cerr << "Usage: " << argv[0] << " <ip_address> <port>" << std::endl;
      return 1;
    }
    app.run();
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