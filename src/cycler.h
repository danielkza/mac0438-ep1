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
  bool full_velocity;
  bool semi_meter;
} cycler_info;

void cycler_info_init(cycler_info *info, int id);
void *cycler(void *);

#endif
