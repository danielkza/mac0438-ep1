#ifndef TRACK_H
#define TRACK_H

#include "cycler.h"

#define MAX_CYCLISTS_PER_POS 4

typedef struct {
    int occupied;
    int occupied_ready;
    int cyclists[MAX_CYCLISTS_PER_POS];
} track_pos_t;

typedef struct {
    int length;
    int laps;
    track_pos_t *positions;
} track_t;

track_t* track_new(int num_cyclers, int length, cycler_info **cycler_infos_ret);
void track_free(track_t *track, cycler_info *cycler_infos);

#endif // TRACK_H
