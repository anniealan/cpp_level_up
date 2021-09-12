#include <fcntl.h>
#include <unistd.h>
#include <iostream>
int main()
{
  int fd = open("file.txt", O_WRONLY | O_CREAT, 0666);
  if (fd < 0)
  {
    perror("[open]");
    return 1;
  }
  if (dup2(fd, STDOUT_FILENO) < 0)
  {
    perror("[dup2]");
    return 1;
  }
  std::cout << "hello world" << std::endl;
  if (close(fd) < 0)
  {
    perror("[HW1]");
    return 1;
  }
  return 0;
}