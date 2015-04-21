#include <stdbool.h>        
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <pthread.h>

#include "util.h"
#include "globals.h"
#include "cycler.h"
#include "track.h"

/* Variáveis globais */
bool debug;
bool use_random_velocity;
pthread_barrier_t cycler_instant_barrier, join, printing;

void print_cyclers(cycler_info *infos, int num_cyclers)
{
    for(int i = 0; i < num_cyclers; i++) {
        printf("%d ", infos[i].pos);
    }

    printf("\n");
}


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
    pthread_mutex_init(&cycler_instant_mutex, NULL);
    pthread_cond_init(&cycler_instant_cond, NULL);
    pthread_barrier_init(&cycler_instant_barrier, NULL, num_cyclers + 1);
    sem_init(&status_sem, 0, 1);
    sem_init(&track_sem, 0, 1);

    cycler_info *cycler_infos;
    track = track_new(num_cyclers, dist, &cycler_infos);

    print_cyclers(cycler_infos, num_cyclers);

    cycler_instant_start_counter = 0;
    
    for(int i = 0; i < num_cyclers; i++) {
        if(pthread_create(&(cycler_infos[i].thread), NULL, cycler, &cycler_infos[i]) != 0) {
            perror("pthread_create: ");
            return 1;
        }
    }

    while(1) {
        /* Seta a condição de início da iteração e notifica todas as threads */
        PTHREAD_USING_MUTEX(&cycler_instant_mutex) {
            cycler_instant_start_counter = 1;
            pthread_cond_broadcast(&cycler_instant_cond);
        }

        /* Espera que todas as threads tenham iniciado */

        pthread_barrier_wait(&cycler_instant_barrier);

        PTHREAD_USING_MUTEX(&cycler_instant_mutex) {
            cycler_instant_start_counter = 1;
        }

        pthread_mutex_lock(&cycler_instant_mutex);

        pthread_mutex_lock(&cycler_instant_mutex);
        cycler_instant_start_counter = 0;
        pthread_mutex_unlock(&cycler_instant_mutex);

        print_cyclers(cycler_infos, num_cyclers);
    }

    /* Sincroniza todos os processos antes de limpar as variáveis */
    sem_destroy(&status_sem);
    sem_destroy(&track_sem);
    pthread_barrier_destroy(&cycler_instant_barrier);
    pthread_cond_destroy(&cycler_instant_cond);
    pthread_mutex_destroy(&cycler_instant_mutex);

    track_free(track, cycler_infos);
    track = NULL;

    pthread_exit(NULL);
    return 0;
}
