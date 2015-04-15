#ifndef CICLISTA_H
#define CICLISTA_H

#include <semaphore.h>

extern int **track;
extern bool *cycler_ready;
extern sem_t status_sem;
extern sem_t track_sem;

void *cycler(void *);

#endif
