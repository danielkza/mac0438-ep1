#include <stdlib.h>

#include "track.h"
#include "cycler.h"

track_t* track_new(int num_cyclers, int length, cycler_info **cycler_infos_ret)
{
    track_t *track = malloc(sizeof(*track));
    track->length = length;
    track->laps = 0;
    track->positions = calloc(length, sizeof(*track->positions));
    
    cycler_info *cycler_infos = malloc(num_cyclers * sizeof(cycler_info));

    for(int i = 0; i < num_cyclers; i++) {
        cycler_infos[i].id = i;
        cycler_infos[i].lap = 0;
        track->positions[i].cyclists[0] = i;
    }
    
    for(int i = num_cyclers - 1; i > 0; i--) {
        int tmp = track->positions[i].cyclists[0], j = rand() % (num_cyclers - 1);
        track->positions[i].cyclists[0] = track->positions[j].cyclists[0];
        track->positions[j].cyclists[0] = tmp;
    }

    for(int i = 0; i < num_cyclers; i++)
        cycler_infos[track->positions[i].cyclists[0]].pos = i;

    *cycler_infos_ret = cycler_infos;
    return track;
}

void track_free(track_t *track, cycler_info *cycler_infos)
{
    free(cycler_infos);
    free(track->positions);
    free(track);
}
