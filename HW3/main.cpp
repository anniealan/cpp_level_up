#include <iostream>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define CMD_FIFO "/tmp/cmdFifo"

void report_and_exit(const char *str)
{
  perror(str);
  exit(EXIT_FAILURE);
}
void execute_command_parent(const char *cmd)
{
  int fd = open(CMD_FIFO, O_WRONLY);
  if (fd < 0)
    report_and_exit("open failed");
  if (write(fd, cmd, strlen(cmd)) < 0)
    report_and_exit("write failed");
  if (close(fd) < 0)
    report_and_exit("close failed");
  if (wait(0) < 0)
    report_and_exit("wait failed");
}

void execute_command_child(const char *cmd)
{
  int fd = open(CMD_FIFO, O_RDONLY);
  if (fd < 0)
    report_and_exit("open failed");
  if (dup2(fd, STDIN_FILENO) < 0)
    report_and_exit("dup2 failed");
  std::string command, output_fname;
  std::cin >> command >> output_fname;
  if (close(fd) < 0)
    report_and_exit("close failed");
  if (mkfifo(output_fname.c_str(), S_IRUSR | S_IWUSR | S_IWGRP | S_IROTH) < 0 && errno != EEXIST)
    report_and_exit("mkfifo failed");
  if ((fd = open(output_fname.c_str(), O_WRONLY)) < 0)
    report_and_exit("open failed");
  if (dup2(fd, STDOUT_FILENO) < 0)
    report_and_exit("dup2 failed");
  execlp(command.c_str(), command.c_str(), NULL);
  if (close(fd) < 0)
    report_and_exit("close failed");
  std::cout << command << ": command not found..." << std::endl;
  exit(EXIT_FAILURE);
}

void execute_command(const char *cmd)
{
  pid_t childpid = fork();
  if (childpid < 0)
    report_and_exit("fork failed");
  if (childpid)
    execute_command_parent(cmd);
  else
    execute_command_child(cmd);
}

int main()
{
  if (mkfifo(CMD_FIFO, S_IRUSR | S_IWUSR | S_IWGRP | S_IROTH) < 0 && errno != EEXIST)
  {
    perror(CMD_FIFO);
    return EXIT_FAILURE;
  }
  execute_command("ls fifo1");
  execute_command("non_existing_command fifo2");
  execute_command("env fifo3");
  return EXIT_SUCCESS;
}