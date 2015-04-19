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

  while(i < 3)
  {
    /* Inicialização da iteração */
    sem_wait(&status_sem);
    //cycler_ready[info->id] = false;
    if(debug)
      printf("STATUS\n");
    sem_post(&status_sem);
    if(debug)
      printf("OOPS\n");
    pthread_barrier_wait(&interaction);

    if(debug)
      printf("Hello %d\n", info->id);
    pthread_barrier_wait(&interaction);

    i++;
  }

  pthread_barrier_wait(&join);
  pthread_exit(NULL); 
}

