#include <stdlib.h>

#include "track.h"
#include "cycler.h"

track_t *track = NULL;

track_t* track_new(int num_cyclers, int length, bool use_random_velocity)
{
    track_t *new_track = malloc(sizeof(*new_track));
    new_track->length = length;
    new_track->laps = 0;
    new_track->use_random_velocity = use_random_velocity;
    
    new_track->positions = calloc(length, sizeof(*new_track->positions));
    new_track->orig_num_cyclers = num_cyclers;
    new_track->num_cyclers = num_cyclers;
    new_track->cycler_infos = malloc(num_cyclers * sizeof(cycler_info));

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

    for(int i = 0; i < num_cyclers; i++)
        cycler_infos[new_track->positions[i].cyclers[0]].pos = i;

    *cycler_infos_ret = cycler_infos;
    return new_track;
}

void track_free(track_t *old_track)
{
    for(int i = 0; i < old_track->orig_num_cyclers; i++) {

    }

    free(old_track->cycler_infos);
    free(old_track->positions);
    free(old_track);
}
