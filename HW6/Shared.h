#pragma once

#include <sys/shm.h>
#include <vector>
#include <string>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>

#define MAX_MSG_SIZE 1024
#define MAX_PROCESS_COUNT 20
#define SHM_KEY_PATH "/dev/null"
#define SHM_KEY_ID 1
#define SEM_NAME "/semaphore"

// Data kept in shared memory segment
struct SharedInfo
{
  char message[MAX_MSG_SIZE];
  pid_t process_ids[MAX_PROCESS_COUNT];
  int process_count;
};

// Singleton class which can be shared between processes
class Shared
{
public:
  static Shared &getInstance()
  {
    static Shared instance;
    return instance;
  }

  std::string getMessage()
  {
    std::string msg;
    if (sem_wait(semaddress) == -1)
      throw std::system_error(errno, std::generic_category());

    msg = shmaddress->message;

    if (sem_post(semaddress) == -1)
      throw std::system_error(errno, std::generic_category());

    return msg;
  }
  void setMessage(std::string msg)
  {
    if (sem_wait(semaddress) == -1)
      throw std::system_error(errno, std::generic_category());

    shmaddress->message[0] = '\0';
    strncat(shmaddress->message, msg.c_str(), MAX_MSG_SIZE - 1);
    for (int i = 0; i < shmaddress->process_count; ++i)
    {
      // send signal to registered processes
      if(kill(shmaddress->process_ids[i], SIGUSR1) == -1)
        perror("Kill");
    }
    if (sem_post(semaddress) == -1)
      throw std::system_error(errno, std::generic_category());
  }
  void registerCallback(void (*f)(std::string))
  {
    if (callbacks.empty())
    {
      if (sem_wait(semaddress) == -1)
        throw std::system_error(errno, std::generic_category());
      shmaddress->process_ids[shmaddress->process_count] = getpid();
      shmaddress->process_count++;
      if (sem_post(semaddress) == -1)
        throw std::system_error(errno, std::generic_category());
    }
    callbacks.push_back(f);
  }
  ~Shared()
  {
    exit();
  }
  void exit()
  {
    // Destroy  the semaphore once all other processes that have the semaphore open close it
    sem_unlink(SEM_NAME);
    // Remove the shared memory segment identifier shmid from the system 
    // and destroy the shared memory segment after the last process detaches it
    if (shmctl(shmid, IPC_RMID, (struct shmid_ds *)nullptr))
      throw std::system_error(errno, std::generic_category());
  }
  Shared(Shared const &) = delete;
  Shared &operator=(Shared const &) = delete;

private:
  Shared()
  {
    // Generate an IPC key for shared memory
    // Pathname and project identifier are used to generate the key
    key_t shmkey = ftok(SHM_KEY_PATH, SHM_KEY_ID);
    if (shmkey == (key_t)-1)
      throw std::system_error(errno, std::generic_category());

    // Find shared memory segment associated with the key, create if not found
    shmid = shmget(shmkey, sizeof(SharedInfo), IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (shmid == -1)
      throw std::system_error(errno, std::generic_category());

    // Attach shared memory with specified id to the calling process
    shmaddress = (SharedInfo *)shmat(shmid, nullptr, 0);
    if (shmaddress == (void *)-1)
      throw std::system_error(errno, std::generic_category());

    // Open an existing named semaphore or create a new one
    semaddress = sem_open(SEM_NAME, O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, 1);
    if (semaddress == SEM_FAILED)
      throw std::system_error(errno, std::generic_category());

    // Register user defined signal handler
    if (signal(SIGUSR1, Shared::sig_handler) == SIG_ERR)
      throw std::system_error(errno, std::generic_category());
  }
  static void sig_handler(int)
  {
    Shared &sh = Shared::getInstance();
    std::string msg = sh.getMessage();
    for (auto cb : sh.callbacks)
    {
      cb(msg);
    }
  }
  std::vector<void (*)(std::string)> callbacks;
  int shmid;
  sem_t *semaddress;
  SharedInfo *shmaddress;
};
