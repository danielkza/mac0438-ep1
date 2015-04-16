#include <stdbool.h>    
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <pthread.h>
#include "cycler.h"
#include "globals.h"

#define CYCLERS_PER_METER 4

/* Variáveis globais */
bool debug;
bool use_random_velocity;
pthread_barrier_t interaction, join;

int* init_track(int num_cyclers)
{
  int i = 0, j = 0, k = 0;
  int pos = 0;
  int *rev_vec = malloc(sizeof(int) * num_cyclers);

  for(i = 0; i < num_cyclers; i++)
  {
    pos = rand() % (num_cyclers - i);
    for(j = 0, k = 0; j < num_cyclers; j++)
    {
      if(track[j][0] == -1)
      {
        if(k == pos)
        {
          track[j][0] = i;
          rev_vec[i] = j;
          break;
        }
        else
          k++;
      }
    }
  }

  if(debug)
  {
    for(i = 0; i < num_cyclers; i++)
      printf("%d ", track[i][0]);
    printf("\n");
  }

  return rev_vec;
}

int main(int argc, char **argv)
{
    /* Contadores */
    int i = 0, j = 0;
    int resp = 0;
    /* Variáveis de leitura de argumentos */
    int dist = -1, num_cyclers = -1;
    int *rev_pos = NULL;
    /* Threads */
    pthread_t *cyclers = NULL;
    cycler_info *info = NULL;

    /* Inicialização de variáveis */
    use_random_velocity = false;
    debug = false;
    srand(0);

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
    pthread_barrier_init(&join, NULL, num_cyclers + 1);
    sem_init(&status_sem, 0, 1);
    sem_init(&track_sem, 0, 1);
   
    track = malloc(sizeof(int*) * dist);
    for(i = 0; i < dist; i++)
    {
      track[i] = malloc(sizeof(int) * CYCLERS_PER_METER);
      for(j = 0; j < CYCLERS_PER_METER; j++)
        track[i][j] = -1;
    }
    rev_pos = init_track(num_cyclers);

    /* Criando vetor de threads */
    cyclers = malloc(sizeof(pthread_t) * num_cyclers);
    cycler_ready = malloc(sizeof(bool) * num_cyclers);
    for(i = 0; i < num_cyclers; i++)
    {
      /* Cria as informações do ciclista */
      info = malloc(sizeof(cycler_info));
      info->id = i;
      info->pos = rev_pos[i];
      info->vel = 1;
      
      resp = pthread_create(&cyclers[i], NULL, cycler, (void*) info);
      if(resp)
      {
        printf("ERROR; return code from pthread_create() is %d\n", resp);
        exit(-1);
      }
    }
    /* Sincroniza todos os processos antes de limpar as variáveis */
    pthread_barrier_wait(&join);

    /* Limpeza */
    sem_destroy(&status_sem);
    sem_destroy(&track_sem);
    pthread_barrier_destroy(&interaction);
    pthread_barrier_destroy(&join);
    for(i = 0; i < dist; i++)
      free(track[i]);
    free(track);
    free(rev_pos);
    free(cyclers);
    free(cycler_ready);

    pthread_exit(NULL);
    return 0;
}
