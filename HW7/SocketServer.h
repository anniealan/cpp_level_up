#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <system_error>
#include <unistd.h>

class SocketServer
{
public:
  SocketServer(std::string ip_address, int port, int max_pending_connections)
  {
    // create an endpoint for communication
    sockid = socket(AF_INET, SOCK_STREAM, 0);
    if (sockid < 0)
      throw std::system_error(errno, std::generic_category());
    sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    // convert port from host to network byte order
    saddr.sin_port = htons(port);
    // convert IP address from numbers-and-dots notation into binary form(in network byte order)
    saddr.sin_addr.s_addr = inet_addr(ip_address.c_str());
    if (saddr.sin_addr.s_addr == 0)
      throw std::runtime_error("IP address is not valid");
    int conn = bind(sockid, (sockaddr *)&saddr, sizeof(saddr));
    if (conn < 0)
      throw std::system_error(errno, std::generic_category());
    // listen for connections on a socket
    if (listen(sockid, max_pending_connections) < 0)
      throw std::system_error(errno, std::generic_category());
    std::cout << "Server is listening on port " << port << std::endl;
  }
  ~SocketServer()
  {
    disconnect();
  }
  int accept()
  {
    sockaddr_in saddr;
    int len = sizeof(saddr);
    // extracts the first connection request from the queue of pending connections
    // creates a new connected socket and returns its file desciptor number
    int newsockid = ::accept(sockid, (sockaddr *)&saddr, (socklen_t *)&len);
    if (newsockid < 0)
      throw std::system_error(errno, std::generic_category());
    return newsockid;
  }
  void disconnect()
  {
    // close the file descriptor
    if (close(sockid) < 0)
      throw std::system_error(errno, std::generic_category());
  }

private:
  int sockid;
};