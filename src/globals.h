#ifndef GLOBALS_H
#define GLOBALS_H

#include <semaphore.h>

/* Variáveis globais */
extern pthread_barrier_t interaction, join;
extern bool debug;
extern bool use_random_velocity;

extern int *track;
extern bool *cycler_ready;
extern sem_t status_sem;
extern sem_t track_sem;

#endif
