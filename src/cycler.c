#include <stdbool.h>        
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include "cycler.h"
#include "globals.h"

/* Variáveis globais */
sem_t status_sem;
sem_t track_sem;

pthread_mutex_t cycler_instant_mutex;
pthread_cond_t cycler_instant_cond;
int cycler_instant_start_counter;

void cycler_info_init(cycler_info *info, int id)
{
    info->id = id;
    info->pos = 0;
    info->lap = 0;
    info->status = RUNNING;
}

void *cycler(void *c_info)
{
    //int vel = 0; /* Número de iterações para mudar de posição */
    // bool semi_meter = false; /* True caso o ciclista precise de mais uma iteracao para se mover */
    cycler_info *info = (cycler_info*) c_info;

    for(int i = 0; i < 3; i++) {
        PTHREAD_USING_MUTEX(&cycler_instant_mutex) {
            while(!cycler_instant_start_counter)
                pthread_cond_wait(&cycler_instant_cond, &cycler_instant_mutex);

            cycler_instant_start_counter--;
        }

        /* Inicialização da iteração */
        sem_wait(&status_sem);

        track->positions[info->pos].cyclers[0] = -1;
        info->pos = (info->pos + 1) % track->length;
        track->positions[info->pos].cyclers[0] = info->id;

        sem_post(&status_sem);

        pthread_barrier_wait(&cycler_instant_barrier);
    }

    return NULL; 
}

