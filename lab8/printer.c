#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <time.h>
#include "queue.h"

int main(){
  sem_t *sem_fill;
  if((sem_fill = sem_open("semaphore_filled", O_RDWR)) == SEM_FAILED){
    perror("Could not create semaphore");
    return EXIT_FAILURE;
  }

  sem_t *sem_empty;
  if((sem_empty = sem_open("semaphore_empty", O_RDWR)) == SEM_FAILED){
    perror("Could not create semaphore");
    return EXIT_FAILURE;
  }

  int fd = shm_open("shared_mem", O_RDWR, 0444);
  if(fd == -1){
    perror("shm_open");
    return EXIT_FAILURE;
  }

  struct queue * Q;
  if((Q = mmap(NULL, sizeof(struct queue), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED){
    perror("Could not map the shared memory");
    close(fd);
    return EXIT_FAILURE;
  }
  
  char buff[MSG_LEN];
  while(sem_wait(sem_fill) != -1){
    // Concurency safety (hopefully)
    printf("Printing from the queue...\n");
    if(is_empty(Q)){
      printf("Queue empty! Concurency!\n");
      continue;
    }

    pop(Q, buff);
    sem_post(sem_empty);
    for(int i = 0; i<MSG_LEN - 1;i++){
      printf("%c", buff[i]);
      fflush(stdout);
      sleep(1);
    }
    printf("\n");
  }

}