#include <stdlib.h>

#include "track.h"
#include "cycler.h"

track_t *track = NULL;

track_t* track_new(int num_cyclers, int length, cycler_info **cycler_infos_ret)
{
    track_t *new_track = malloc(sizeof(*new_track));
    new_track->length = length;
    new_track->laps = 0;
    new_track->positions = calloc(length, sizeof(*new_track->positions));
    
    cycler_info *cycler_infos = malloc(num_cyclers * sizeof(cycler_info));

    for(int i = 0; i < num_cyclers; i++) {
        cycler_infos[i].id = i;
        cycler_infos[i].lap = 0;
        new_track->positions[i].cyclists[0] = i;
    }
    
    for(int i = num_cyclers - 1; i > 0; i--) {
        int tmp = new_track->positions[i].cyclists[0], j = rand() % (num_cyclers - 1);
        new_track->positions[i].cyclists[0] = new_track->positions[j].cyclists[0];
        new_track->positions[j].cyclists[0] = tmp;
    }

    for(int i = 0; i < num_cyclers; i++)
        cycler_infos[new_track->positions[i].cyclists[0]].pos = i;

    *cycler_infos_ret = cycler_infos;
    return new_track;
}

void track_free(track_t *old_track, cycler_info *cycler_infos)
{
    free(cycler_infos);
    free(old_track->positions);
    free(old_track);
}
