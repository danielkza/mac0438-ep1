#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

/* Vetor compartilhado */
int *pista;

void *ciclista(void *threadid)
{
  long tid = (long) threadid;

  printf("Hello %ld\n", tid);
  pthread_exit(NULL); 
}

