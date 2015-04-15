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
    int i = 0;
    long j = 0;
    int resp = 0;
    /* Variáveis de leitura de argumentos */
    int dist = -1, num_cyclers = -1;
    bool use_random_velocity_i = false;
    /* Threads */
    pthread_t *ciclistas = NULL;

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

    pthread_barrier_init(&interaction, NULL, num_cyclers);

    /* Criando vetor de threads */
    ciclistas = malloc(sizeof(pthread_t) * num_cyclers);
    for(j = 0; j < num_cyclers; j++)
    {
      printf("In main: creating thread %d\n", j);
      resp = pthread_create(&ciclistas[j], NULL, ciclista, (void *)j);
      if(resp)
      {
        printf("ERROR; return code from pthread_create() is %d\n", resp);
        exit(-1);
      }
    }


    /* Limpeza */
    pthread_barrier_destroy(&interaction);
    pthread_exit(NULL);
    free(ciclistas);

    return 0;
}
