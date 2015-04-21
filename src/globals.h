#ifndef GLOBALS_H
#define GLOBALS_H

#include <semaphore.h>

#include "track.h"

/* Variáveis globais */
extern pthread_barrier_t cycler_instant_barrier, join, printing;
extern bool debug;
extern bool use_random_velocity;

extern pthread_mutex_t cycler_instant_mutex;
extern pthread_cond_t cycler_instant_cond;
extern int cycler_instant_start_counter;

extern sem_t status_sem;
extern sem_t track_sem;

extern track_t *track;

#endif
