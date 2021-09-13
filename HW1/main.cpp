#include <fcntl.h>
#include <unistd.h>
#include <iostream>
int main()
{
  if (close(STDOUT_FILENO) < 0)
  {
    perror("[close]");
    return 1;
  }
  int fd = open("file.txt", O_WRONLY | O_CREAT, 0666);
  if (fd < 0)
  {
    perror("[open]");
    return 1;
  }
  std::cout << "hello world" << std::endl;
  if (close(fd) < 0)
  {
    perror("[close]");
    return 1;
  }
  return 0;
}