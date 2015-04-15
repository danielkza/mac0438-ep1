#include <stdbool.h>    
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <pthread.h>
#include "ciclista.h"

/* Variáveis globais */
bool debug;
pthread_barrier_t interaction;

int main(int argc, char **argv)
{
    /* Contadores */
    int i = 0, j = 0;
    long thread_id = 0;
    int resp = 0;
    /* Variáveis de leitura de argumentos */
    int dist = -1, num_cyclers = -1;
    bool use_random_velocity_i = false;
    /* Threads */
    pthread_t *cyclers = NULL;

    /* Inicialização de variáveis */
    debug = false;

    /* Leitura de argumentos*/
    if(argc >= 4) {
        dist = atoi(argv[1]);
        num_cyclers = atoi(argv[2]);
  
        if(strcasecmp(argv[3], "v") == 0)
            use_random_velocity_i = true;
        else if(strcasecmp(argv[3], "u") == 0)
            use_random_velocity_i = false;

        /* Opção de debug */
        if(argc > 4)
        {
          if(strcasecmp(argv[4], "-d") == 0)
            debug = true;
        }
    }

    /* Verificação */
    if(dist <= 249 || num_cyclers <= 3 || use_random_velocity_i < 0) {
        fprintf(stderr, "Uso: %s distância n_ciclistas [v|u]\n", argv[0]);
        return 1;
    }

    /* Inicialização de variáveis globais */
    pthread_barrier_init(&interaction, NULL, num_cyclers);
    sem_init(&status_sem, 0, 1);
    sem_init(&track_sem, 0, 1);
    track = malloc(sizeof(int*) * dist);
    for(i = 0; i < dist; i++)
    {
      track[i] = malloc(sizeof(int) * 4);
      for(j = 0; j < 4; j++)
        track[i][j] = -1;
    }

    /* Criando vetor de threads */
    cyclers = malloc(sizeof(pthread_t) * num_cyclers);
    cycler_ready = malloc(sizeof(bool) * num_cyclers);
    for(thread_id = 1; thread_id < num_cyclers; thread_id++)
    {
      resp = pthread_create(&cyclers[i], NULL, cycler, (void*) thread_id);
      if(resp)
      {
        printf("ERROR; return code from pthread_create() is %d\n", resp);
        exit(-1);
      }
    }


    /* Limpeza */
    sem_destroy(&status_sem);
    sem_destroy(&track_sem);
    pthread_barrier_destroy(&interaction);
    pthread_exit(NULL);
    for(i = 0; i < dist; i++)
      free(track[i]);
    free(track);
    free(cyclers);
    free(cycler_ready);

    return 0;
}
