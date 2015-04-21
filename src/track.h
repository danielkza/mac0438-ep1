#ifndef TRACK_H
#define TRACK_H

#include "cycler.h"

#define MAX_CYCLERS_PER_POS 4

typedef struct {
    int occupied;
    int occupied_ready;
    int cyclers[MAX_cyclers_PER_POS];
} track_pos_t;

typedef struct {
    int length;
    int laps;
    bool use_random_velocity;
    int orig_num_cyclers;
    int num_cyclers;
    cycler_info *cycler_infos;
    track_pos_t *positions;
} track_t;

track_t* track_new(int num_cyclers, int length, bool use_random_velocity);
void track_free(track_t *old_track);

#endif // TRACK_H
