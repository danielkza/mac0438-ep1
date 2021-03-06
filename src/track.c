#include <stdlib.h>
#include <stdio.h>

#include "track.h"
#include "cycler.h"

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
    
    // Fisher-Yates shuffle
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
    // Atualiza volta mais alta da pista inteira baseada na volta dos ciclistas
    for(int i = 0; i < track->orig_num_cyclers; i++) {
        cycler_info *info = &track->cycler_infos[i];
        if(info->status == CYCLER_RUNNING) {
            if(info->lap > track->lap)
                track->lap = info->lap;
        }
    }

    // Marca que nenhum ciclista ainda fez seu movimento, e portanto nenhum
    // espaço já esta totamente ocupado para essa iteração
    for(int i = 0; i < track->length; i++) {
        track->positions[i].occupied_ready = 0;
    }
}

static void track_eliminate_cycler(track_t *track, cycler_info *info,
                                   cycler_status status)
{
    info->status = status;
    track->num_cyclers--;

    // remove ciclista de seu espaço atual
    track_pos_t* track_pos = &(track->positions[info->pos]);
    for(int i = 0; i < MAX_CYCLERS_PER_POS; i++) {
        if(track_pos->cyclers[i] == info->id) {
            track_pos->cyclers[i] = -1;
            track_pos->occupied--;
            break;
        }
    }
}

static int track_cyclers_compare_by_pos(const void *a, const void *b)
{
    // Compara ciclistas em ordem crescente de posição na pista (último para primeiro)
    cycler_info *info_a = *((cycler_info **)a),
                *info_b = *((cycler_info **)b);
    if(info_a->lap < info_b->lap || (info_a->lap == info_b->lap && info_a->pos < info_b->pos))
        return -1;
    else if(info_a->lap > info_b->lap || info_a->pos > info_b->pos)
        return 1;

    return 0;
}

// Aloca e retorna um vetor contento ponteiros para os ciclistas da pista,
// em ordem de posição, opcionalmente incluindo somente aqueles que ainda
// competem.
cycler_info **track_get_cyclers_in_order(track_t *track, bool only_running)
{
    int res_size = (only_running? track->num_cyclers : track->orig_num_cyclers);
    cycler_info **infos = calloc(res_size, sizeof(*infos));
    int infos_last = 0;

    // Cria uma array com ponteiros para todas as estruturas cycler_info.
    // Essa cópia será ordenada e retornada para o caller.
    for(int i = 0; i < track->orig_num_cyclers; i++) {
        cycler_info *info = &(track->cycler_infos[i]);
        if(only_running && info->status != CYCLER_RUNNING)
            continue;

        infos[infos_last++] = info;
    }

    qsort(infos, res_size, sizeof(*infos), &track_cyclers_compare_by_pos);
    return infos;
}

// Recria barreira com o número atual de ciclistas
static void track_update_barrier(track_t *track)
{
    pthread_barrier_destroy(&cycler_instant_barrier);
    pthread_barrier_init(&cycler_instant_barrier, NULL, track->num_cyclers + 1);
}

// Processa eliminações e crashes de ciclistas antes de dar inicio a uma nova
// volta
int track_update_eliminations(track_t *track)
{
    // Nenhuma eliminação ou crash a fazer
    if(!track->waiting_for_elimination && !track->will_crash)
        return 0;

    // Contabiliza número de eliminados para determinar se é necessário refazer
    // a barreira
    int eliminated = 0;
    // Pega todos os ciclistas já ordenados do último para o primeiro
    cycler_info **cyclers_in_order = track_get_cyclers_in_order(track, true);
    // Guarda o tamanho original do cyclers_in_order pois ele vai diferir do
    // tamanho guardado na track após eliminações
    int orig_num = track->num_cyclers;

    if(track->waiting_for_elimination != 0) {
        // Procura dentre os elimináveis aqueles que acabaram de terminar suas
        // voltas
        int max_to_check = track->waiting_for_elimination;
        
        for(int i = 0; i < max_to_check && i < track->num_cyclers; i++) {
            cycler_info *info = cyclers_in_order[i];
            if(info->pos == 0 && info->lap % 2 == 0) {
                track_eliminate_cycler(track, info, CYCLER_FINISHED);
                eliminated++;
                track->waiting_for_elimination--;
                // Impede que um ciclista eliminado seja considerado para crash
                cyclers_in_order[i] = NULL;
            }
        }
    }

    // Não podemos mais crashar ciclistas depois que um número mínimo sobrou
    if(track->num_cyclers <= MIN_CYCLERS_FOR_CRASHES) {
        // Só sobrou o vencedor, vamos 'eliminá-lo' para completar o programa
        if(eliminated && track->num_cyclers == 1) {
            cycler_info *info = cyclers_in_order[orig_num - 1];
            track_eliminate_cycler(track, info, CYCLER_FINISHED);
            eliminated++;
        }

        track->will_crash = 0;
    } else {
        // Escolhe um ciclista aleatório para crashar
        //
        // Repete as tentativas caso encontremos slots que correspondiam a
        // ciclistas já eliminados
        while(track->will_crash != 0) {
            cycler_info *info = cyclers_in_order[rand() % orig_num];
            if(info == NULL)
                continue;

            track_eliminate_cycler(track, info, CYCLER_CRASHED);
            eliminated++;
            track->will_crash--;
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

void track_print_three_last(track_t *track)
{
    int min = (track->num_cyclers < 3? track->num_cyclers : 3);
    cycler_info **infos = track_get_cyclers_in_order(track, true);

    printf("Últimos colocados (último ao terceiro último): ");
    for(int i = 0; i < min; i++) 
        printf("%d ", infos[i]->id);
    printf("\n");

    free(infos);
}

void track_print_final(track_t *track)
{
    int orig_num_cyclers = track->orig_num_cyclers;
    cycler_info **infos = track_get_cyclers_in_order(track, false);

    printf("Colocações:\n");
    for(int i = orig_num_cyclers - 1, j = 1, f = 0; i >=0; i--, j++) {
        printf("%d colocado: %d", j, infos[i]->id);

        if(infos[i]->status == CYCLER_CRASHED)
            printf(" - QUEBRADO");
        else {
            f++;

            switch(f) {
                case 1:
                    printf(" - OURO");
                    break;
                case 2:
                    printf(" - PRATA");
                    break;
                case 3:
                    printf(" - BRONZE");
                    break;
            }
        }

        printf("\n");
    }

    free(infos);
}

void track_free(track_t *old_track)
{
    free(old_track->cycler_infos);
    free(old_track->positions);
    free(old_track);
}
