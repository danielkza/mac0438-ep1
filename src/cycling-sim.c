#include <stdbool.h>    
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <pthread.h>

#include "cycler.h"
#include "track.h"
#include "globals.h"

/* Variáveis globais */
bool debug;
bool use_random_velocity;
pthread_barrier_t interaction, join, printing;


int main(int argc, char **argv)
{
  /* Variáveis de leitura de argumentos */
  int dist = -1, num_cyclers = -1, use_random_velocity_i = -1;

  /* Inicialização de variáveis */
  debug = false;
  srand(0);

  /* Leitura de argumentos*/
  if(argc >= 4) {
    dist = atoi(argv[1]);
    num_cyclers = atoi(argv[2]);

    if(strcasecmp(argv[3], "v") == 0)
      use_random_velocity_i = 1;
    else if(strcasecmp(argv[3], "u") == 0)
      use_random_velocity_i = 0;

    /* Opção de debug */
    if(argc > 4 && strcasecmp(argv[4], "-d") == 0)
      debug = true;
  }

  /* Verificação */
  if(dist <= 249 || num_cyclers <= 3 || use_random_velocity_i < 0) {
    fprintf(stderr, "Uso: %s distância n_ciclistas [v|u]\n", argv[0]);
    return 1;
  }

  /* Inicialização de variáveis globais */
  pthread_barrier_init(&interaction, NULL, num_cyclers + 1);
  pthread_barrier_init(&printing, NULL, num_cyclers + 1);
  pthread_barrier_init(&join, NULL, num_cyclers + 1);
  sem_init(&status_sem, 0, 1);
  sem_init(&track_sem, 0, 1);

  cycler_info *cycler_infos;
  track = track_new(num_cyclers, dist, &cycler_infos);

  for(int i = 0; i < num_cyclers; i++) {
    printf("%d ", cycler_infos[i].pos);
  }
  printf("\n");

  for(int i = 0; i < num_cyclers; i++) {
    if(pthread_create(&(cycler_infos[i].thread), NULL, cycler, &cycler_infos[i]) != 0) {
      perror("pthread_create: ");
      return 1;
    }
  }

  pthread_barrier_wait(&interaction);
  for(int i = 0; i < 3; i++) {
    pthread_barrier_wait(&interaction);
    for(int j = 0; j < num_cyclers; j++) {
      printf("%d ", cycler_infos[j].pos);
    }
    printf("\n");
    pthread_barrier_wait(&printing);
  }

  /* Sincroniza todos os processos antes de limpar as variáveis */
  pthread_barrier_wait(&join);

  sem_destroy(&status_sem);
  sem_destroy(&track_sem);
  pthread_barrier_destroy(&interaction);
  pthread_barrier_destroy(&join);

  track_free(track, cycler_infos);
  track = NULL;

  pthread_exit(NULL);
  return 0;
}
