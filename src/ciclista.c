#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

/* ARRUMAR ESSE GATO */
extern pthread_barrier_t interaction;

/* Vetor compartilhado */
int *pista;

void *ciclista(void *threadid)
{
  long tid = (long) threadid;
  int i = 0;

  while(i < 3)
  {
    printf("Hello %ld\n", tid);
    pthread_barrier_wait(&interaction);
    i++;
  }

  pthread_exit(NULL); 
}

