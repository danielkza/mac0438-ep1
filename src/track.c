#include <stdlib.h>
#include <stdio.h>

#include "track.h"
#include "cycler.h"
#include "globals.h"

track_t *g_track = NULL;

track_t* track_new(int num_cyclers, int length, bool use_random_velocity)
{
    track_t *new_track = malloc(sizeof(*new_track));
    new_track->length = length;
    new_track->lap = 0;
    new_track->use_random_velocity = use_random_velocity;
    
    new_track->positions = calloc(length, sizeof(*new_track->positions));
    new_track->orig_num_cyclers = num_cyclers;
    new_track->num_cyclers = num_cyclers;
    new_track->cycler_infos = malloc(num_cyclers * sizeof(cycler_info));

    new_track->waiting_for_elimination = 0;
    new_track->will_crash = 0;

    for(int i = 0; i < length; i++) {
        for(int j = 0; j < MAX_CYCLERS_PER_POS; j++)
            new_track->positions[i].cyclers[j] = -1;
    }

    for(int i = 0; i < num_cyclers; i++) {
        cycler_info_init(&(new_track->cycler_infos[i]), i);
        new_track->positions[i].cyclers[0] = i;
    }
    
    // Fisher-Yates
    for(int i = num_cyclers - 1; i > 0; i--) {
        int tmp = new_track->positions[i].cyclers[0];
        int j = rand() % (num_cyclers - 1);
        new_track->positions[i].cyclers[0] = new_track->positions[j].cyclers[0];
        new_track->positions[j].cyclers[0] = tmp;
    }

    for(int i = 0; i < num_cyclers; i++) {
        new_track->cycler_infos[new_track->positions[i].cyclers[0]].pos = i;
        new_track->positions[i].occupied = 1;
    }

    return new_track;
}

void track_update_lap(track_t *track)
{
    for(int i = 0; i < track->orig_num_cyclers; i++) {
        cycler_info *info = &track->cycler_infos[i];
        if(info->status == CYCLER_RUNNING) {
            if(info->lap > track->lap)
                track->lap = info->lap;
        }
    }

    for(int i = 0; i < track->length; i++) {
        track->positions[i].occupied_ready = 0;
    }
}

static void track_eliminate_cycler(track_t *track, cycler_info *info,
                                   cycler_status status)
{
    info->status = status;
    track->num_cyclers--;

    track_pos_t* track_pos = &(track->positions[info->pos]);
    for(int i = 0; i < MAX_CYCLERS_PER_POS; i++) {
        if(track_pos->cyclers[i] == info->id) {
            track_pos->cyclers[i] = -1;
            break;
        }
    }
}

static int track_cyclers_compare_by_pos(const void *a, const void *b)
{
    cycler_info *info_a = *((cycler_info **)a),
                *info_b = *((cycler_info **)b);
    if(info_a->lap < info_b->lap || (info_a->lap == info_b->lap && info_a->pos < info_b->pos))
        return -1;
    else if(info_a->lap > info_b->lap || info_a->pos > info_b->pos)
        return 1;

    return 0;
}

cycler_info **track_get_cyclers_in_order(track_t *track)
{
    cycler_info **infos = malloc(track->num_cyclers * sizeof(*infos));
    int infos_last = 0;

    // Cria uma array com ponteiros para todas as estruturas cycler_info.
    // Essa cópia será ordenada e retornada para o caller.
    for(int i = 0; i < track->orig_num_cyclers; i++) {
        cycler_info *info = &(track->cycler_infos[i]);
        if(info->status != CYCLER_RUNNING)
            continue;

        infos[infos_last++] = info;
    }

    qsort(infos, track->num_cyclers, sizeof(*infos), &track_cyclers_compare_by_pos);
    return infos;
}

static void track_update_barrier(track_t *track)
{
    // Reseta barreira com o novo número de ciclistas para evitar deadlock
    pthread_barrier_destroy(&cycler_instant_barrier);
    pthread_barrier_init(&cycler_instant_barrier, NULL, track->num_cyclers + 1);
}

int track_update_eliminations(track_t *track)
{
    // Nenhuma eliminação ou crash a fazer
    if(!track->waiting_for_elimination && !track->will_crash)
        return 0;

    // Contabiliza número de eliminados para determinar se é necessário refazer
    // a barreira
    int eliminated = 0;
    // Pega todos os ciclistas já ordenados do último para o primeiro
    cycler_info **cyclers_in_order = track_get_cyclers_in_order(track);
    // Guarda o tamanho original do cyclers_in_order pois ele vai diferir do
    // tamanho guardado na track após eliminações
    int orig_num = track->num_cyclers;

    if(track->waiting_for_elimination != 0) {
        int max_to_check = track->waiting_for_elimination;
        for(int i = 0; i < max_to_check && i < track->num_cyclers; i++) {
            cycler_info *info = cyclers_in_order[i];
            if(info->pos == 0 && info->lap % 2 == 0) {
                track_eliminate_cycler(track, info, CYCLER_FINISHED);
                printf("Eliminou %d\n", info->id);

                for(int j = 0; j < orig_num; j++) {
                    cycler_info *info2 = cyclers_in_order[j];
                    if(info2 != NULL)
                        printf("%d ", info2->id);
                    else
                        printf("- ");
                }
                printf("\n");

                track->waiting_for_elimination--;
                eliminated++;
                // Impede que um ciclista eliminado seja considerado para crash
                cyclers_in_order[i] = NULL;
            }
        }
    }

    // Não podemos mais crashar ciclistas depois que um número mínimo sobrou
    if(track->num_cyclers <= MIN_CYCLERS_FOR_CRASHES) {
        if(track->num_cyclers == 1)
        {
            for(int i = orig_num - 1; i >= 0; i--) {
                cycler_info *info = cyclers_in_order[i];
                if(info != NULL){
                    track_eliminate_cycler(track, info, CYCLER_FINISHED);
                    eliminated++;
                    break;
                }
            }
        }
        track->will_crash = 0;
    } else {
        // Repete as tentativas caso encontremos slots que correspondiam a
        // ciclistas já eliminados
        while(track->will_crash != 0) {
            cycler_info *info = cyclers_in_order[rand() % orig_num];
            if(info == NULL)
                continue;

            track_eliminate_cycler(track, info, CYCLER_CRASHED);
            printf("Crashou %d\n", info->id);

            track->will_crash--;
            eliminated++;
        }
    }

    // Atualiza barreira se preciso
    if(eliminated)
        track_update_barrier(track);

    free(cyclers_in_order);
    return eliminated;
}

void track_print_cyclers(track_t *track)
{
    printf("%-5s|%-10s|%-10s|%-15s|%-15s\n", "ID", "Volta", "Posicao", "Status", "Velocidade");
    
    for(int i = 0; i < track->orig_num_cyclers; i++) {
        cycler_info *info = &(track->cycler_infos[i]);
        const char *status_s = "UNKNOWN";
        switch(info->status) {
            case CYCLER_RUNNING:
                status_s = "RUNNING";
                break;
            case CYCLER_FINISHED:
                status_s = "FINISHED";
                break;
            case CYCLER_CRASHED:
                status_s = "CRASHED";
                break;
        }

        int vel = info->full_velocity ? 50 : 25;
        printf("%-5d|%-10d|%-10d|%-15s|%-15d\n", info->id, info->lap, info->pos, status_s, vel);
    }

    printf("\n");
}

void track_print_tree_last(track_t *track)
{
    track = track;

    return;
}

void track_print_final(track_t *track)
{
    track = track;

    return;
}

int track_find_last(track_t *track)
{
    int last = -1, last_pos = -1, last_lap = -1;
    
    for(int i = 0; i < track->orig_num_cyclers; ++i) {
        cycler_info *info = &track->cycler_infos[i];
        if(info->status != CYCLER_RUNNING)
            continue;

        if(last != -1) {
            if(info->lap > last_lap || info->pos > last_pos
               || (info->pos == last_pos && rand() % 2 == 0))
            {
                continue;
            }
        }

        last = i;
        last_pos = info->pos;
        last_lap = info->lap;
    }

    return last;
}


void track_free(track_t *old_track)
{
    for(int i = 0; i < old_track->orig_num_cyclers; i++) {

    }

    free(old_track->cycler_infos);
    free(old_track->positions);
    free(old_track);
}
