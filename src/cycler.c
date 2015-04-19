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

void *cycler(void *c_info)
{
  int i = 0;
  //int vel = 0; /* Número de iterações para mudar de posição */
  // bool semi_meter = false; /* True caso o ciclista precise de mais uma iteracao para se mover */
  cycler_info *info = (cycler_info*) c_info;

  pthread_barrier_wait(&interaction);
  while(i < 3)
  {
    /* Inicialização da iteração */
    sem_wait(&status_sem);
    //cycler_ready[info->id] = false;
    track->positions[info->pos].cyclists[0] = -1;
    info->pos = (info->pos + 1) % track->length;
    track->positions[info->pos].cyclists[0] = info->id;

    sem_post(&status_sem);

    pthread_barrier_wait(&interaction);
    pthread_barrier_wait(&printing);

    i++;
  }

  pthread_barrier_wait(&join);
  pthread_exit(NULL); 
}

