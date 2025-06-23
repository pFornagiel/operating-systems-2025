
#define _POSIX_C_SOURCE 199309L
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#define PATIENT_WALK_SECONDS 10
#define QUEUE_LEN 3
#define MEDKIT_CAPACITY 3

// Ansi colors for printing
#define COLOR_DOCTOR   "\x1b[31m"
#define COLOR_PHARMACIST "\x1b[32m"
#define COLOR_PATIENT "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"

int num_of_patients = 0;
int num_of_pharms = 0;
int pharms_done = 0;
int patients_done = 0;

int waiting_room = 0;          // how many patients are waiting at the moment
int medkit = MEDKIT_CAPACITY;  // how much medicine is in the medkit
int patients_left;             // how many patients remain in the visit
int pharmacists_waiting = 0;   // bonus things to synrchronise all below
int is_refilling = 0;
int is_taking_patients = 0;
int are_patients_waiting = 0;
int can_patients_come = 1;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  doctor_cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t  seat_cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t  pharm_wait = PTHREAD_COND_INITIALIZER;
pthread_cond_t  refill_cv = PTHREAD_COND_INITIALIZER;

int patient_queue[QUEUE_LEN];


// Variadic function
void cprintf(char * color, char * text, ...){
  va_list args;
  printf("%s", color); 
  time_t t;
  time(&t);
  printf("[%ld] - ", t);

  va_start(args, text);
  vprintf(text, args);  // handling variable arguments
  va_end(args);

  printf("%s", ANSI_COLOR_RESET); 
}

void* patient(void* arg){
  int patient_id = *(int*)(arg);
  free(arg);

  int try_wait = 1;
  while(try_wait){
    pthread_mutex_lock(&mutex);

    if(can_patients_come == 0){
      cprintf(COLOR_PATIENT, "Pacjent(%d): za dużo pacjentów, wracam później za %d s\n", patient_id, PATIENT_WALK_SECONDS);
      pthread_mutex_unlock(&mutex);
      sleep(PATIENT_WALK_SECONDS);
    } else {
      are_patients_waiting = 1;
      patient_queue[waiting_room] = patient_id;
      waiting_room++;
      
      cprintf(COLOR_PATIENT, "Pacjent(%d): czeka %d pacjentów na lekarza\n", patient_id, waiting_room);

      if(waiting_room == QUEUE_LEN ){
        can_patients_come = 0;
        pthread_cond_signal(&doctor_cv);
        cprintf(COLOR_PATIENT, "Pacjent(%d): budzę lekarza\n", patient_id);
      } 
      
      while(are_patients_waiting == 1){
        pthread_cond_wait(&seat_cv, &mutex);
      }
      
      if(--waiting_room == 0){
        can_patients_come = 1;
      }

      try_wait=0;

      pthread_mutex_unlock(&mutex);
    }

  }
  
  cprintf(COLOR_PATIENT, "Pacjent(%d): kończę wizyte\n", patient_id);
  return 0;
}

void* pharmacist(void *arg)
{
  int pharmacist_id = *(int *)(arg);
  free(arg);
  pthread_mutex_lock(&mutex);

  if(medkit >= 3 || is_refilling || is_taking_patients){
    cprintf(COLOR_PHARMACIST, "Farmaceuta(%d): czekam na opróżnienie apteczki\n", pharmacist_id);
  }
  
  while(medkit >= 3 || pharmacists_waiting > 0){
    pthread_cond_wait(&pharm_wait, &mutex);
  }
  
  pharmacists_waiting++;
  cprintf(COLOR_PHARMACIST, "Farmaceuta(%d): budzę lekarza\n", pharmacist_id);

  while(is_refilling == 0) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts); // current time
    ts.tv_sec += 1; // 1 second delay
    pthread_cond_signal(&doctor_cv);
    pthread_cond_timedwait(&refill_cv, &mutex, &ts);
  }
  
  cprintf(COLOR_PHARMACIST, "Farmaceuta(%d): Zaczynam dostarczać leki\n", pharmacist_id);
  sleep(1 + rand() % 3);
  medkit = MEDKIT_CAPACITY;
  pharmacists_waiting--;
  is_refilling = 0;
  pharms_done += 1;

  cprintf(COLOR_PHARMACIST, "Farmaceuta(%d): dostarczono leki\n", pharmacist_id);
  // is_refilling = 0;
  pthread_cond_signal(&doctor_cv);
  pthread_mutex_unlock(&mutex);

  
  return 0;
}

void doctor(){
  while(1){
    pthread_mutex_lock(&mutex);

    if(num_of_patients - patients_done < 3 || (medkit < 3 && pharms_done == num_of_pharms)){
      cprintf(COLOR_DOCTOR, "Nic więcej nie da się zrobić. Lekarz kończy pracę.\n");
      return;
    }

    while (! ((waiting_room == 3 && medkit >= 3)
           || (medkit < 3 && pharmacists_waiting > 0)) ) {
      pthread_cond_wait(&doctor_cv, &mutex);
    }

    if (waiting_room == 3 && medkit >= 3) {
      // konsultacja
      is_taking_patients = 1;
      cprintf(COLOR_DOCTOR, "Lekarz: konsultuję pacjentów %d, %d, %d\n",
             patient_queue[0], patient_queue[1], patient_queue[2]);

      medkit -= 3;
      // patients_left -= 3;
      sleep(2 + rand() % 4);
      is_taking_patients = 0;
      are_patients_waiting = 0;
      patients_done += 3;
      
      // wake up patients and pharmacists
      pthread_cond_broadcast(&seat_cv);
      pthread_mutex_unlock(&mutex);
      pthread_cond_signal(&pharm_wait);
    }
    else if(medkit < 3 && pharmacists_waiting > 0) {
      is_refilling = 1;
      cprintf(COLOR_DOCTOR, "Lekarz: przyjmuję dostawę leków\n");
      pthread_cond_broadcast(&refill_cv);
      while(is_refilling == 1){
        pthread_cond_wait(&doctor_cv, &mutex);
      }
      pthread_mutex_unlock(&mutex);
    }

    cprintf(COLOR_DOCTOR, "Lekarz: zasypiam\n");
  }
}


void* patient_spawner()
{
  int counter = 0;
  while (counter < num_of_patients) {
    int sleep_amount = 2 + rand() % 5;
    cprintf(COLOR_PATIENT, "Pacjent(%d): Idę do szpitala, będę za %d s\n", counter, sleep_amount);
    sleep(sleep_amount);
    
    // Allocate memory for the "id"/counter passed to the patient
    int *id = (int*)malloc(sizeof(int));
    *id = counter;

    pthread_t tid;
    if(pthread_create(&tid, NULL, patient, (void *) id) != 0){
      perror("Could not create patient thread");
      free(id);
    } else {
      // make thread detached, so it frees its resources automatically without join 
      pthread_detach(tid);
    }
    counter++; 
  }
  return (void*) 0;
}

void* pharmacist_spawner()
{
  int counter = 0;
  while (counter < num_of_pharms) {
    int sleep_amount =  5 + rand() % 15;
    cprintf(COLOR_PHARMACIST, "Farmaceuta(%d): Idę do szpitala, będę za %d s\n", counter, sleep_amount);
    sleep(sleep_amount);
    
    // Allocate memory for the "id"/counter passed to the pharmacist
    int *id = (int*)malloc(sizeof(int));
    *id = counter;

    pthread_t tid;
    if(pthread_create(&tid, NULL, pharmacist, (void *) id) != 0){
      perror("Could not create patient thread");
      free(id);
    } else {
      // make thread detached, so it frees its resources automatically without join 
      pthread_detach(tid);
    }
    counter++; 
  }
  return (void*) 0;
}


int main(int argc, char ** argv){
  if(argc != 3) {
    printf("Usage: number_of_patients num_of_pharmacists");
    return EXIT_FAILURE;
  }

  num_of_patients = atoi(argv[1]);
  num_of_pharms = atoi(argv[2]);

  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&doctor_cv, NULL);
  pthread_cond_init(&seat_cv, NULL);
  pthread_cond_init(&pharm_wait, NULL);
  pthread_cond_init(&refill_cv, NULL);

  pthread_t _tid;
  if(pthread_create(&_tid, NULL, patient_spawner, NULL) != 0){
    perror("Could not create patient spawner! Exiting...");
    return EXIT_FAILURE;
  }
  
  if(pthread_create(&_tid, NULL, pharmacist_spawner, NULL) != 0){
    perror("Could not create pharmacist spawner! Exiting...");
    return EXIT_FAILURE;
  }

  doctor();
}