#ifndef GLOBALS_H
#define GLOBALS_H

#include <semaphore.h>

#include "track.h"

/* Variáveis globais */
extern pthread_barrier_t interaction, join, printing;
extern bool debug;
extern bool use_random_velocity;

extern sem_t status_sem;
extern sem_t track_sem;

extern track_t *track;

#endif
