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

int track_update_cyclers(track_t *track)
{
    int num = 0, eliminated = 0;
    for(int i = 0; i < track->orig_num_cyclers; i++) {
        cycler_info *info = &track->cycler_infos[i];
        if(info->status == CYCLER_RUNNING) {
            num++;
            if(info->lap > track->lap)
                track->lap = info->lap;
        }
    }

    // Reseta barreira com o novo número de ciclistas para evitar deadlock
    if(num != track->num_cyclers) {
        pthread_barrier_destroy(&cycler_instant_barrier);
        pthread_barrier_init(&cycler_instant_barrier, NULL, num + 1);

        eliminated = num - track->num_cyclers;
        track->num_cyclers = num;
    }

    for(int i = 0; i < track->length; i++) {
        track->positions[i].occupied_ready = 0;
    }

    return eliminated;
}

void track_print_cyclers(track_t *track)
{
    for(int pos = 0; pos < track->length; pos++) {
        track_pos_t *track_pos = &track->positions[pos];
        if(track_pos->occupied == 0)
            continue;

        printf("%d (%d):", pos, track_pos->occupied);

        for(int i = 0; i < MAX_CYCLERS_PER_POS; i++) {
            int id = track_pos->cyclers[i];
            if(id == -1)
                continue;

            cycler_info *info = &(track->cycler_infos[id]);
            if(info->status == CYCLER_RUNNING)
                printf(" %d ", info->id);
        }

        printf("\n");
    }
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
