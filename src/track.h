#ifndef TRACK_H
#define TRACK_H

#include <stdbool.h>
#include "cycler.h"

#define MAX_CYCLERS_PER_POS 4

typedef struct {
    int occupied;
    int occupied_ready;
    int cyclers[MAX_CYCLERS_PER_POS];
} track_pos_t;

typedef struct {
    int length;
    int lap;
    bool use_random_velocity;
    int orig_num_cyclers;
    int num_cyclers;
    cycler_info *cycler_infos;
    track_pos_t *positions;
} track_t;

track_t* track_new(int num_cyclers, int length, bool use_random_velocity);
int track_update_cyclers(track_t *track);
void track_print_cyclers(track_t *track);
int track_find_last(track_t *track);
void track_free(track_t *old_track);

#endif // TRACK_H
