#ifndef CYCLER_H
#define CYCLER_H

#include "pthread.h"

typedef enum {
    CYCLER_RUNNING = 0,
    CYCLER_FINISHED = 1,
    CYCLER_CRASHED = 2
} cycler_status;

/* Estrutura de informação */
typedef struct {
  int id;
  int pos;
  int lap;
  cycler_status status;
  pthread_t thread;
  bool full_velocity; // true se andando a 50km/h, false se 25km/h
  // true se na última iteração, uma tentativa de andar falhou devido a posição
  // cheia a frente. Já andamos 'meio' metro mesmo a 25km/h, e portanto na
  // próxima iteração poderemos adentrar imediatamente se possível. 
  bool semi_meter; 
} cycler_info;

extern pthread_barrier_t cycler_instant_barrier;
extern pthread_mutex_t cycler_instant_mutex;
extern pthread_cond_t cycler_instant_cond;
extern int cycler_instant_start_counter;

void cycler_info_init(cycler_info *info, int id);
void *cycler(void *);

#endif
