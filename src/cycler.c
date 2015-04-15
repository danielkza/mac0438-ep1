#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

/* ARRUMAR ESSE GATO */
extern pthread_barrier_t interaction;

/* Variáveis globais */
int **track;
bool *cycler_ready;
sem_t status_sem;
sem_t track_sem;

void *cycler(void *thread_id)
{
  long tid = (long) thread_id;
  int i = 0;

  while(i < 3)
  {
    /* Inicialização da iteração*/
    sem_wait(&status_sem);
    cycler_ready[tid] = false;
    sem_post(&status_sem);
    pthread_barrier_wait(&interaction);

    printf("Hello %ld\n", tid);
    pthread_barrier_wait(&interaction);

    i++;
  }

  pthread_exit(NULL); 
}

