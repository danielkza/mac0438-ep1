#ifndef CYCLER_H
#define CYCLER_H

#include "pthread.h"

typedef enum {
    RUNNING = 0,
    FINISHED = 1,
    CRASHED = 2
} cycler_status;

/* Estrutura de informação */
typedef struct {
  int id;
  int pos;
  int lap;
  cycler_status status;
  pthread_t thread;
} cycler_info;

/* Funções */
void *cycler(void *);

#endif
