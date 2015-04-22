#include <stdbool.h>        
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#include "cycler.h"
#include "globals.h"
#include "util.h"

/* Variáveis globais */
sem_t status_sem;
sem_t track_sem;

pthread_mutex_t cycler_instant_mutex;
pthread_cond_t cycler_instant_cond;
int cycler_instant_start_counter;

void cycler_info_init(cycler_info *info, int id)
{
    info->id = id;
    info->pos = 0;
    info->lap = 0;
    info->status = CYCLER_RUNNING;
    info->full_velocity = 0;
    info->semi_meter = 0;
}

void *cycler(void *c_info)
{
    bool use_random_velocity = g_track->use_random_velocity;
    cycler_info *info = (cycler_info*) c_info;
    int id = info->id;

    info->full_velocity = !use_random_velocity;
    info->semi_meter = 0;
    
    while(1) {
        PTHREAD_USING_MUTEX(&cycler_instant_mutex) {
            while(!cycler_instant_start_counter)
                pthread_cond_wait(&cycler_instant_cond, &cycler_instant_mutex);

            cycler_instant_start_counter--;
        }

        if(info->status != CYCLER_RUNNING) {
            printf("SAINDO\n");
            break;
        }

        /* Inicialização da iteração */
        int old_pos = info->pos;
        int new_pos = old_pos;

        if(info->full_velocity || info->semi_meter) 
            new_pos = (info->pos + 1) % g_track->length;

        track_pos_t *old_track_pos = &g_track->positions[old_pos];
        track_pos_t *new_track_pos = &g_track->positions[new_pos];

        while(new_pos != old_pos) {
            // Posição já está cheia, e todos os ciclistas ali já fizeram seu movimento
            if(new_track_pos->occupied_ready == MAX_CYCLERS_PER_POS) {
                new_pos = old_pos;
                break;
            }

            // Posição está cheia, mas alguem não fez o movimento, podemos
            // tentar novamente
            if(new_track_pos->occupied == MAX_CYCLERS_PER_POS) {
                // TODO: sleep
                continue;
            }

            // Há um slot vazio: vamos procurá-lo e imediatamente nos colocarmos
            // ali se possível
            int *slot, slot_pos;
            for(slot_pos = 0; slot_pos < MAX_CYCLERS_PER_POS; slot_pos++) {
                slot = &(new_track_pos->cyclers[slot_pos]);
                if(__sync_bool_compare_and_swap(slot, -1, id)) {
                    break;
                }
            }

            // Não conseguimos nenhum slot, alguém chegou antes: vamos tentar novamente
            if(slot_pos == MAX_CYCLERS_PER_POS) {
                continue;
            }

            __sync_fetch_and_add(&new_track_pos->occupied_ready, 1);
            __sync_fetch_and_add(&new_track_pos->occupied, 1);

            for(slot_pos = 0; slot_pos < MAX_CYCLERS_PER_POS; slot_pos++) {
                slot = &(old_track_pos->cyclers[slot_pos]);
                if(__sync_bool_compare_and_swap(slot, id, -1)) {
                    __sync_fetch_and_sub(&old_track_pos->occupied, 1);
                    break;
                }
            }
            assert(slot_pos != MAX_CYCLERS_PER_POS);

            break;
        };

        if(new_pos == old_pos) {
            info->semi_meter = true;
            __sync_fetch_and_add(&old_track_pos->occupied_ready, 1);
        } else {
            if(!info->full_velocity)
                info->semi_meter = false;
            info->pos = new_pos;
            if(info->pos == 0) {
                info->lap++;
                if(use_random_velocity) 
                    info->full_velocity = (rand() % 2 == 0);
            }
        }

        pthread_barrier_wait(&cycler_instant_barrier);
    }

    return NULL; 
}

