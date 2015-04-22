#include <stdbool.h>        
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#include "track.h"
#include "util.h"
#include "cycler.h"

/* Variáveis globais */

pthread_mutex_t cycler_instant_mutex;
pthread_cond_t cycler_instant_cond;
int cycler_instant_start_counter;
pthread_barrier_t cycler_instant_barrier;

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
        // Início de iteração é condicionado a liberação pela thread principal.
        // O contador é inicializado com o número de threads, a condição é
        // sinalizada pra todas elas, e o decremento faz com que o contador
        // volte a 0 após o início de todas, impedindo que uma segunda
        // iteração siga.
        PTHREAD_USING_MUTEX(&cycler_instant_mutex) {
            while(!cycler_instant_start_counter)
                pthread_cond_wait(&cycler_instant_cond, &cycler_instant_mutex);

            cycler_instant_start_counter--;
        }

        // Nosso ciclista saiu da corrida, podemos parar a thread atual
        if(info->status != CYCLER_RUNNING) {
            if(info->status == CYCLER_CRASHED)
                printf("Ciclista %d quebrou no metro %d da volta %d\n", info->id, info->lap, info->pos);
            break;
        }

        // Calcula posição nova. Ela será uma posição a frente da atual somente
        // se:
        // a) O ciclista roda a 50km/h, ou roda a 25km/h mas já andou 'meia'
        //    posição anteriormente
        // b) A posição seguinte não está totalmente ocupada por ciclistas que
        //    já se moveram: pode acontecer caso 4 threads rodem antes desta
        int old_pos = info->pos;
        int new_pos = old_pos;

        if(info->full_velocity || info->semi_meter) 
            new_pos = (info->pos + 1) % g_track->length;

        track_pos_t *old_track_pos = &g_track->positions[old_pos];
        track_pos_t *new_track_pos = &g_track->positions[new_pos];

        while(new_pos != old_pos) {
            // Posição já está cheia, e todos os ciclistas ali já fizeram seu movimento,
            // não iremos mais nos mover
            if(new_track_pos->occupied_ready == MAX_CYCLERS_PER_POS) {
                new_pos = old_pos;
                continue;
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

            // Não conseguimos nenhum slot, alguém chegou antes: repetir
            if(slot_pos == MAX_CYCLERS_PER_POS) {
                continue;
            }

            // Conseguimos a posição, incrementemos o contador de ocupados.
            // Isso pode ser feito sem uma seção crítica pois o compare-and-swap
            // na colocação do ID no vetor garante que ninguém vai 'pular na nossa frente'.
            // No máximo outra thread tentará repetir a inserção até que tenhamos
            // atualizado os contadores
            __sync_fetch_and_add(&new_track_pos->occupied_ready, 1);
            __sync_fetch_and_add(&new_track_pos->occupied, 1);

            // Agora vamos nos remover do contador da posição antiga.
            // Isso também pode ser feito somente comm um fetch-and-sub
            // pois outras threads só vão parar de tentar adentrar se todos os
            // ocupantes do espaço *já tiverem se movido*. Como nós não o fizemos,
            // faremos outros esperar até nossa remoção, mas só temporariamente.
            for(slot_pos = 0; slot_pos < MAX_CYCLERS_PER_POS; slot_pos++) {
                slot = &(old_track_pos->cyclers[slot_pos]);
                if(__sync_bool_compare_and_swap(slot, id, -1)) {
                    __sync_fetch_and_sub(&old_track_pos->occupied, 1);
                    break;
                }
            }
            // Nunca deve ser possível que falhemos em nos remover da posição
            // antiga
            assert(slot_pos != MAX_CYCLERS_PER_POS);

            break;
        };

        // Se não pudemos nos mover, precisamos marcar que não iremos mais nos
        // mover, e que outras threads devem parar de tentar adentrar caso
        // a posição esteja cheia
        if(new_pos == old_pos) {
            info->semi_meter = true;
            __sync_fetch_and_add(&old_track_pos->occupied_ready, 1);
        } else {
            // Se nos movemos, é necessario atualizar o número da volta, se
            // for o caso, a velocidade da próxima volta e remover o 'meio metro'
            if(!info->full_velocity)
                info->semi_meter = false;
            
            info->pos = new_pos;
            if(info->pos == 0) {
                info->lap++;
                if(use_random_velocity) 
                    info->full_velocity = (rand() % 2 == 0);
            }
        }

        // Está barreira existe para que a *main thread* possa esperar por 
        // todas as outras para fazer seu trabalho de limpeza. Quem realmente
        // garante que novas iterações começem no momento certo é a varíável
        // de condição (vide começo da função)
        pthread_barrier_wait(&cycler_instant_barrier);
    }

    return NULL; 
}

