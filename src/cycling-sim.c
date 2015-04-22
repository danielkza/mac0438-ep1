#include <stdbool.h>        
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <pthread.h>

#include "util.h"
#include "cycler.h"
#include "track.h"

int main(int argc, char **argv)
{
    /* Variáveis de leitura de argumentos */
    bool debug = false;
    int dist = -1, num_cyclers = -1, use_random_velocity_i = -1;
    int debug_count = 0;

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

    g_track = track_new(num_cyclers, dist, use_random_velocity_i == 1);

    cycler_instant_start_counter = 0;

    for(int i = 0; i < g_track->num_cyclers; i++) {
        cycler_info *info = &(g_track->cycler_infos[i]);
        if(pthread_create(&info->thread, NULL, cycler, info) != 0) {
            perror("pthread_create: ");
            return 1;
        }
    }

    /* Iterações */
    int prev_lap = 0;

    while(1) {
        track_update_lap(g_track);

        /* Imprime os três últimos a cada volta */
        if(prev_lap != g_track->lap)
            track_print_three_last(g_track);

        /* Elimina o último colocado a cada duas voltas */
        if(g_track->lap != prev_lap && g_track->lap % 2 == 0) {
            if(g_track->lap % 4 == 0 && drand48() <= 0.1)
                g_track->will_crash++;

            g_track->waiting_for_elimination++;
        }

        prev_lap = g_track->lap;

        int eliminated = track_update_eliminations(g_track);

        /* Seta a condição de início da iteração e notifica todas as threads */
        PTHREAD_USING_MUTEX(&cycler_instant_mutex) {
            /* Como alguns ciclitas acabaram de ser eliminados, ainda é preciso
               incluí-los na variável de condição de contagem de entrada.
               A própria thread verá que não deve mais rodar e irá parar
               antes da iteração.
               */
            cycler_instant_start_counter = eliminated + g_track->num_cyclers;
            pthread_cond_broadcast(&cycler_instant_cond);
        }

        /* if(eliminated)
            printf("eliminated: %d, num: %d\n", eliminated, g_track->num_cyclers);
         */

        /* Espera que todas as threads tenham terminado a iteração */
        // printf("enter barrier main\n");
        pthread_barrier_wait(&cycler_instant_barrier);

        /* Sai do loop caso não haja mais ciclistas */
        if(g_track->num_cyclers == 0) {
            //printf("bailing main loop\n");
            break;
        }

        /* Print de debug */
        if(debug) {
            if(debug_count == 0)
                track_print_cyclers(g_track);
            else
                debug_count = (debug_count + 1) % 200;
        }
    }

    /* Imprime a colocação final */
    track_print_final(g_track);

    /* Sincroniza todos os processos antes de limpar as variáveis */
    pthread_barrier_destroy(&cycler_instant_barrier);
    pthread_cond_destroy(&cycler_instant_cond);
    pthread_mutex_destroy(&cycler_instant_mutex);

    track_free(g_track);
    g_track = NULL;

    pthread_exit(NULL);
    return 0;
}
