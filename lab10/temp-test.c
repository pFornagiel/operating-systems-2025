#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>

#define PATIENT_WALK_SECONDS 10
#define QUEUE_LEN 3
#define MEDKIT_CAPACITY 3

// Ansi colors for printing
#define COLOR_DOCTOR   "\x1b[31m"
#define COLOR_PHARMACIST "\x1b[32m"
#define COLOR_PATIENT "\x1b[97m"
#define ANSI_COLOR_RESET   "\x1b[0m"

pthread_cond_t waiting_room_cond = PTHREAD_COND_INITIALIZER,
               medkit_cond = PTHREAD_COND_INITIALIZER,
               is_doctor_busy_cond = PTHREAD_COND_INITIALIZER,
               is_doctor_busy_medkit_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t waiting_room_mutex = PTHREAD_MUTEX_INITIALIZER,
                medkit_mutex = PTHREAD_MUTEX_INITIALIZER,
                is_doctor_busy_mutex = PTHREAD_MUTEX_INITIALIZER,
                is_doctor_busy_medkit_mutex = PTHREAD_MUTEX_INITIALIZER;
int waiting_room = 0, medkit = 0, is_doctor_busy = 0, is_doctor_busy_medkit = 0;

int patient_queue[3];


// Variadic function
void cprintf(char * color, char * text, ...){
  va_list args;
  printf("%s", color); 

  va_start(args, text);
  vprintf(text, args);  // handling variable arguments
  va_end(args);

  printf("%s", ANSI_COLOR_RESET); 
}

void patient(void* arg){
  int patient_id = *(int*)(arg);
  free(arg);

  int try_wait = 1;
  while(try_wait){
    pthread_mutex_lock(&waiting_room_mutex);

    if(waiting_room >= QUEUE_LEN){
      cprintf(COLOR_PATIENT, "Pacjent(%d): za dużo pacjentów, wracam później za %d s\n", patient_id, PATIENT_WALK_SECONDS);
      pthread_mutex_unlock(&waiting_room_mutex);
      sleep(PATIENT_WALK_SECONDS);
    } else {
      patient_queue[waiting_room] = patient_id;
      waiting_room++;
      
      cprintf(COLOR_PATIENT, "Pacjent(%d): czeka %d pacjentów na lekarza\n", patient_id, waiting_room);

      if(waiting_room == QUEUE_LEN){
        pthread_mutex_lock(&is_doctor_busy_mutex);
        is_doctor_busy = 1;
        pthread_cond_broadcast(&is_doctor_busy);
        cprintf("Pacjent(%d): budzę lekarza\n", patient_id);
        pthread_mutex_unlock(&is_doctor_busy_mutex);
      } else {
        while(waiting_room < 3){
          pthread_cond_wait(&waiting_room_cond, &waiting_room_mutex);
        }
      }
      pthread_mutex_unlock(&waiting_room_mutex);

      pthread_mutex_lock(&is_doctor_busy_mutex);
      while(is_doctor_busy == 1){
        pthread_cond_wait(&is_doctor_busy_cond, &is_doctor_busy_mutex);
      }
      
      cprintf(COLOR_PATIENT, "Pacjent(%d): kończę wizytę\n", patient_id);
      pthread_mutex_unlock(&is_doctor_busy_mutex);
      try_wait=0;
      
    }
  }
}

void pharmacist(void *arg)
{
  int pharmacist_id = *(int *)(arg);
  free(arg);
  pthread_mutex_lock(&medkit_mutex);
  while(medkit >= 3){
    cprintf(COLOR_PHARMACIST, "Farmaceuta(%d): czekam na oproznienie apteczki\n", pharmacist_id);
    pthread_cond_wait(&medkit_cond, &medkit_mutex);
  }
  if(medkit < 3){
    pthread_mutex_lock(&is_doctor_busy_medkit_mutex);
    is_doctor_busy_medkit = 1;
    
    pthread_cond_signal(&is_doctor_busy_medkit_mutex);
    pthread_mutex_unlock(&medkit_mutex);
    cprintf(COLOR_PHARMACIST, "Farmaceuta(%d): budzę lekarza\n", pharmacist_id);

    cprintf(COLOR_PHARMACIST, "Farmaceuta(%d): Zaczynam dostarczać leki\n", pharmacist_id);
    while(is_doctor_busy_medkit == 1){
      pthread_cond_wait(&is_doctor_busy_medkit_cond, &is_doctor_busy_medkit_mutex);
    }

    cprintf(COLOR_PHARMACIST, "Farmaceuta(%d): dostarczono leki\n", pharmacist_id);
    pthread_mutex_unlock(&is_doctor_busy_medkit_mutex);

  } 
  
}

void doctor(){
  pthread_mutex_lock(&medkit_mutex);
  pthread_mutex_lock(&is_doctor_busy_medkit_mutex);
  if(medkit < 3 && is_doctor_busy_medkit == 1){

  }
  pthread_mutex_unlock(&is_doctor_busy_medkit_mutex);
  
  pthread_mutex_lock(&)
  if(medkit >= 3 && is_doctor_busy == 1){

  }


}

void patient_spawner()
{
  int counter = 0;
  while (1) {
    int sleep_amount = (rand() + 2) % 5;
    cprintf(COLOR_PATIENT, "Pacjent(%d): Idę do szpitala, będę za %d s\n", counter, sleep_amount);
    sleep(sleep_amount);
    
    // Allocate memory for the "id"/counter passed to the patient
    int *id = (int)malloc(sizeof(int));
    *id = counter;

    pthread_t tid;
    if(pthread_create(&tid, NULL, patient, (void *)counter) != 0){
      perror("Could not create patient thread");
      free(id);
    } else {
      // make thread detached, so it frees its resources automatically without join 
      pthread_detach(tid);
    }
    counter++; 
  }
}

void pharmacist_spawner()
{
  int counter = 0;
  while (1) {
    int sleep_amount = (rand() + 5) % 15;
    cprintf(COLOR_PHARMACIST, "Farmaceuta(%d): Idę do szpitala, będę za %d s\n", counter, sleep_amount);
    sleep(sleep_amount);
    
    // Allocate memory for the "id"/counter passed to the pharmacist
    int *id = (int)malloc(sizeof(int));
    *id = counter;

    pthread_t tid;
    if(pthread_create(&tid, NULL, pharmacist, (void *)counter) != 0){
      perror("Could not create patient thread");
      free(id);
    } else {
      // make thread detached, so it frees its resources automatically without join 
      pthread_detach(tid);
    }
    counter++; 
  }
}


int main(int argc, char *argv)
{
  // srand(time(NULL));
  // pthread_cond_init(&waiting_room, NULL);
  // pthread_cond_init(&medkit, NULL);
  // pthread_cond_init(&is_doctor_busy, NULL);
}