#ifndef CYCLER_H
#define CYCLER_H

#include "pthread.h"

/* Estrutura de informação */
typedef struct {
  int id;
  int pos;
  int lap;
  pthread_t thread;
} cycler_info;

/* Funções */
void *cycler(void *);

#endif
